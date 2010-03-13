/***************************************************************************
 *   Copyright (C) 2008 - 2009 by Lukas Appelhans                                 *
 *   l.appelhans@gmx.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "quicklaunchApplet.h"

#include <KConfigDialog>
#include <KDesktopFile>
#include <KMessageBox>
#include <KOpenWithDialog>
#include <KPropertiesDialog>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsWidget>
#include <QDir>
#include <QDrag>
#include <QHash>
#include <QMimeData>

#include <KDialog>
#include <KMimeType>
#include <KWindowSystem>
#include <KIconLoader>
#include <KUrl>

#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/corona.h>

#include "math.h"

//TODO: Make Dialog resizeable (which also makes the number of rows in it configurable :)

static const int s_defaultIconSize = 16;
static const int s_defaultSpacing = 2;

QuicklaunchApplet::QuicklaunchApplet(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent, args),
    m_layout(0),
    m_innerLayout(0),
    m_timer(new QTimer(this)),
    m_visibleIcons(6),
    m_preferredIconSize(s_defaultIconSize),
    m_iconSize(s_defaultIconSize),
    m_dialogIconSize(s_defaultIconSize),
    m_dialog(0),
    m_dialogWidget(0),
    m_dialogLayout(0),
    m_addDialog(0),
    m_rightClickedIcon(0),
    m_isBusy(false),
    m_showIconNames(false),
    m_addAction(0),
    m_removeAction(0),
    m_sortappAscending(0),
    m_sortappDescending(0)
{
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(performUiRefactor()));
    m_timer->setSingleShot(true);

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    resize(s_defaultIconSize * 3 + s_defaultSpacing * 4 + left + right,
           s_defaultIconSize + s_defaultSpacing * 2 + top + bottom);
}

QuicklaunchApplet::~QuicklaunchApplet()
{
    if (m_dialog) {
        m_dialog->close();
        delete m_dialog;
    }

    delete m_dialogWidget;
}

void QuicklaunchApplet::saveConfig()
{
    KConfigGroup cg = config();
    saveState(cg);
    emit configNeedsSaving();
}

void QuicklaunchApplet::saveState(KConfigGroup &config) const
{
    QStringList iconUrls;
    foreach (QuicklaunchIcon * container, m_icons) {
        iconUrls.append(container->url().prettyUrl());
    }

    config.writeEntry("iconUrls", iconUrls);
}

void QuicklaunchApplet::init()
{
    m_isBusy = true;
    KConfigGroup cg = config();

    m_preferredIconSize =
        qMax(s_defaultIconSize, int(cg.readEntry("iconSize", contentsRect().height() / 2)));
    m_iconSize = m_preferredIconSize;
    m_dialogIconSize =
        qMax(s_defaultIconSize, (int)cg.readEntry("dialogIconSize", contentsRect().height() / 2));

    m_visibleIcons = qMax(-1, cg.readEntry("visibleIcons", m_visibleIcons));
    m_showIconNames = (bool)cg.readEntry("showIconNames", false);
    setShowIconNames(m_showIconNames);

    // Initialize outer layout
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    // Initialize inner layout
    m_innerLayout = new QuicklaunchLayout(1, this, 0);
    m_innerLayout->setContentsMargins(0, 0, 0, 0);
    m_innerLayout->setSpacing(0);
    m_layout->addItem(m_innerLayout);

    // Initial "more icons" arrow
    m_arrow = new Plasma::IconWidget(this);
    m_arrow->setIcon(KIcon("arrow-right"));
    connect(m_arrow, SIGNAL(clicked()), SLOT(showDialog()));

    QStringList desktopFiles = cg.readEntry("iconUrls", QStringList());

    bool firstStart = desktopFiles.isEmpty();
    if (firstStart) {
        QStringList defaultApps;
        defaultApps << "konqbrowser" << "dolphin" << "kopete";

        foreach (const QString &defaultApp, defaultApps) {
            KService::Ptr service = KService::serviceByStorageId(defaultApp);
            if (service && service->isValid()) {
                QString path = service->entryPath();
                //kDebug() << path;
                if (!path.isEmpty() && QDir::isAbsolutePath(path)) {
                    desktopFiles << path;
                }
            }
        }
        //kDebug() << desktopFiles;
    }

    foreach (const QString &desktopFile, desktopFiles) {
        addProgram(-1, desktopFile);
    }

    performUiRefactor();

    if (firstStart) {
        resize(sizeHint(Qt::PreferredSize));
    }

    m_isBusy = false;
}

void QuicklaunchApplet::setShowIconNames(bool set)
{
    foreach (QuicklaunchIcon *icon, m_icons) {
        icon->setText(set ? icon->appName() : QString());
    }
}

QSizeF QuicklaunchApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (which == Qt::PreferredSize) {
        QSizeF sizeHint = size();
        if (!m_innerLayout) {
            return sizeHint;
        }
        qreal newWidth =
            m_innerLayout->columnCount() * sizeHint.height() /
            qMax(1, m_innerLayout->preferredRowCount());

        if (m_icons.size() > m_visibleIcons) {
            sizeHint.setWidth(newWidth + sizeHint.height());
        } else {
            sizeHint.setWidth(newWidth);
        }
        return sizeHint;
    }
    return QGraphicsWidget::sizeHint(which, constraint);
}

void QuicklaunchApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::SizeConstraint) {
        refactorUi();
    }
}

void QuicklaunchApplet::refactorUi()
{
     //static int my_counter = 0;
     //kDebug() << "refactorUi count = " << my_counter++;
     m_timer->start(100);
}

void QuicklaunchApplet::performUiRefactor()
{
    //static int my_perforcount = 0;
    //kDebug() << "performUiRefactor count = " << my_perforcount++;
    clearLayout(m_innerLayout);

    //Don't accept values under 16 nor anything over applet's height
    m_iconSize =
        qMin(
            qMax(m_preferredIconSize, s_defaultIconSize),
            int(contentsRect().height() - 2 * s_defaultSpacing));

    m_dialogIconSize = qMax(m_dialogIconSize, s_defaultIconSize);

    const int numInnerLayoutIcons =
        m_visibleIcons != -1 ?
            qMin(m_icons.size(), m_visibleIcons) :
            m_icons.size();

    // Determine the number of rows to show.
    int rowCount;
    int colCount;

    if (formFactor() == Plasma::Vertical) {
        // Use as many columns as possible.
        colCount = qMax(1, int(contentsRect().width() / m_iconSize));
        rowCount = qMax(1, numInnerLayoutIcons / colCount + numInnerLayoutIcons % colCount);
    } else {
        // Use as many rows as possible.
        rowCount = qMax(1, int(contentsRect().height() / m_iconSize));
        colCount = qMax(1, numInnerLayoutIcons / rowCount + numInnerLayoutIcons % rowCount);
    }

    m_innerLayout->setPreferredRowCount(rowCount);

    // Add the items that are shown in the inner layout
    for (int i = 0; i < numInnerLayoutIcons; i++) {
        QuicklaunchIcon *icon = m_icons.at(i);
        icon->resize(QSize(m_iconSize, m_iconSize));
        icon->setIconSize(m_iconSize);
        icon->show();

        m_innerLayout->addItem(icon, i / colCount, i % colCount);
    }

    if (m_dialogLayout) {
        clearLayout(m_dialogLayout);

        if (numInnerLayoutIcons < m_icons.size()) {

            int numDialogIcons = m_icons.size() - numInnerLayoutIcons;
            int dialogRowCount =
                qMax(1, int(size().height() / qMin(m_dialogIconSize, m_dialog->size().height())));
            int dialogColCount =
                qMax(1, numDialogIcons / dialogRowCount + numDialogIcons % dialogRowCount);

            m_dialogLayout->setPreferredRowCount(dialogRowCount);

            for (int i = numInnerLayoutIcons; i < m_icons.size(); i++) {
                QuicklaunchIcon *icon = m_icons.at(i);
                icon->setMinimumSize(QSize(m_dialogIconSize, m_dialogIconSize)); //TODO: Remove maximum/minimum sizes
                icon->setMaximumSize(QSize(m_dialogIconSize, m_dialogIconSize));
                icon->setIconSize(m_dialogIconSize);
                icon->show();

                int dialogIconIndex = i - numInnerLayoutIcons;
                m_dialogLayout->addItem(icon, dialogIconIndex / dialogColCount, dialogIconIndex % dialogColCount);
            }
        }
    } else { // !m_dialogLayout
        for (int i = numInnerLayoutIcons; i < m_icons.size(); i++) {
            m_icons.at(i)->hide();
        }
    }

    m_layout->removeItem(m_arrow);
    if (m_icons.size() > m_visibleIcons && m_visibleIcons != -1) {
        m_arrow->resize(QSize(size().height(), size().height()));
        m_layout->addItem(m_arrow);
        m_arrow->show();
    } else {
        m_arrow->hide();
    }

    setPreferredSize(QSize((m_iconSize + 6) * colCount,
                           (m_iconSize + 6) * ceil(qMax(1, numInnerLayoutIcons) / double(colCount))));

    if (m_dialog) {
        m_dialog->close();
        m_dialogLayout->updateGeometry();
        m_dialog->adjustSize();
    }

    update();
}

void QuicklaunchApplet::showDialog()
{
    if (!m_dialog) {
        m_dialogWidget = new QGraphicsWidget(this);
        m_dialogWidget->setAcceptDrops(true);
        m_dialogWidget->installEventFilter(this);
        qobject_cast<Plasma::Corona*>(m_dialogWidget->scene())->addOffscreenWidget(m_dialogWidget);

        // Initialize "more icons" dialog
        m_dialog = new Plasma::Dialog(0, Qt::X11BypassWindowManagerHint);
        m_dialog->setAcceptDrops(true);
        //m_dialog->installEventFilter(this);
        m_dialog->setContextMenuPolicy(Qt::ActionsContextMenu);
        m_dialogLayout = new QuicklaunchLayout(1, m_dialogWidget, m_dialogWidget);
        m_dialogWidget->setLayout(m_dialogLayout);
        performUiRefactor();
        m_dialog->setGraphicsWidget(m_dialogWidget);
    }

    if (m_dialog->isVisible()) {
        m_dialog->hide();
    } else {
        m_dialog->resize(m_dialogLayout->preferredSize().toSize());
        //m_dialog->updateGeometry();
        if (containment() && containment()->corona()) {
            //kDebug() << "position:" << containment()->corona()->popupPosition(m_arrow, m_dialog->size()) << "dialog size:" << m_dialog->size() << "layout preferred-size:" << m_dialogLayout->preferredSize().toSize();
            m_dialog->move(containment()->corona()->popupPosition(m_arrow, m_dialog->size()));
        }
        KWindowSystem::setState(m_dialog->winId(), NET::SkipTaskbar);
        m_dialog->show();
    }
}

void QuicklaunchApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    uiConfig.setupUi(widget);
    connect(parent, SIGNAL(accepted()), SLOT(configAccepted()));

    int dialogHeight = (int)KIconLoader::SizeEnormous;
    int height = dialogHeight;

    if (formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical) {
        height = contentsRect().height() - 2 * s_defaultSpacing;
    }

    uiConfig.iconSizeSpin->setMaximum(height);
    uiConfig.iconSizeSlider->setMaximum(height);
    uiConfig.dialogIconSizeSpin->setMaximum(dialogHeight);
    uiConfig.dialogIconSizeSlider->setMaximum(dialogHeight);

    uiConfig.iconSizeSpin->setMinimum(s_defaultIconSize);
    uiConfig.iconSizeSlider->setMinimum(s_defaultIconSize);
    uiConfig.dialogIconSizeSpin->setMinimum(s_defaultIconSize);
    uiConfig.dialogIconSizeSlider->setMinimum(s_defaultIconSize);

    uiConfig.iconSizeSpin->setValue(m_preferredIconSize);
    uiConfig.iconSizeSlider->setValue(m_preferredIconSize);
    uiConfig.dialogIconSizeSpin->setValue(m_dialogIconSize);
    uiConfig.dialogIconSizeSlider->setValue(m_dialogIconSize);

    uiConfig.iconNamesCheckBox->setChecked(m_showIconNames);

    uiConfig.icons->setValue(m_visibleIcons);
    parent->addPage(widget, i18n("General"), icon());
}

void QuicklaunchApplet::configAccepted()
{
    KConfigGroup cg = config();

    bool changed = false;
    int temp;

    temp = uiConfig.icons->value();
    if (temp != m_visibleIcons) {
        m_visibleIcons = temp;
        cg.writeEntry("visibleIcons", m_visibleIcons);
        changed = true;
    }

    temp = uiConfig.iconSizeSpin->value();
    if (temp != m_preferredIconSize) {
        m_preferredIconSize = temp;
        cg.writeEntry("iconSize", m_preferredIconSize);
        changed = true;
    }

    temp = uiConfig.dialogIconSizeSpin->value();
    if (temp != m_dialogIconSize) {
        m_dialogIconSize = temp;
        cg.writeEntry("dialogIconSize", m_dialogIconSize);
        changed = true;
    }

    bool iconNames = uiConfig.iconNamesCheckBox->isChecked();
    if (m_showIconNames != iconNames) {
        m_showIconNames = iconNames;
	cg.writeEntry("showIconNames", m_showIconNames);
        setShowIconNames(m_showIconNames);
	changed = true;
    }

    if (changed) {
        emit configNeedsSaving();
        resize(sizeHint(Qt::PreferredSize));
        performUiRefactor();
    }
}

QList<QAction*> QuicklaunchApplet::contextActions(QuicklaunchIcon *icon)
{
    QList<QAction*> tempActions;
    if (!m_addAction) {
        m_addAction = new QAction(KIcon("list-add"), i18n("Add Launcher..."), this);
        connect(m_addAction, SIGNAL(triggered(bool)), this, SLOT(showAddInterface()));
    }

    tempActions << m_addAction;

    if (icon) {
        m_rightClickedIcon = icon;
        if (!m_removeAction) {
            m_removeAction = new QAction(KIcon("list-remove"), i18n("Remove Launcher"), this);
            connect(m_removeAction, SIGNAL(triggered(bool)), this, SLOT(removeCurrentIcon()));
        }
        tempActions << m_removeAction;
    }

    if (!m_sortappAscending) {
        m_sortappAscending = new QAction(KIcon("view-sort-ascending"), i18n("Sort Alphabetically (A to Z)"), this);
        connect(m_sortappAscending, SIGNAL(triggered(bool)), this, SLOT(ascendingSort()));
    }
    tempActions << m_sortappAscending;

    if (!m_sortappDescending) {
        m_sortappDescending = new QAction(KIcon("view-sort-descending"), i18n("Sort Alphabetically (Z to A)"), this);
        connect(m_sortappDescending, SIGNAL(triggered(bool)), this, SLOT(descendingSort()));
    }
    tempActions << m_sortappDescending;

    return tempActions;
}

void QuicklaunchApplet::ascendingSort()
{
    m_isBusy = true;
    sortQuicklaunch(AscendingSort);
    m_isBusy = false;
}

void QuicklaunchApplet::descendingSort()
{
    m_isBusy = true;
    sortQuicklaunch(DescendingSort);
    m_isBusy = false;
}

void QuicklaunchApplet::sortQuicklaunch(SortingOrder sortingorder)
{
    QHash <QString,QString> quicklaunchHash;
    KUrl::List urls;
    QList<QString> sortedList;
    QList<QString> saveSortedUrlList;

    foreach (QuicklaunchIcon *icon, m_icons) {
        quicklaunchHash.insert(icon->appName(),icon->url().prettyUrl());
    }
    sortedList = quicklaunchHash.keys();

    qSort(sortedList);

    for (int i = 0; i < quicklaunchHash.size(); i++) {
        saveSortedUrlList.append(quicklaunchHash.value(sortedList.value(i)));
    }

    if (sortingorder == DescendingSort) {
        QList<QString> tempUrl;

        for (int i = saveSortedUrlList.size(); i > 0; i--) {
            tempUrl.append(saveSortedUrlList.takeLast());
        }
        saveSortedUrlList = tempUrl;
    }

    foreach (QuicklaunchIcon *icon, m_icons) {
        m_icons.removeAll(icon);
        icon->hide();
        icon->deleteLater();
    }

    foreach (const QString &desktopFile, saveSortedUrlList) {
        addProgram(-1, desktopFile);
    }
    performUiRefactor();

    KConfigGroup cg = config();
    cg.writeEntry("iconUrls", saveSortedUrlList);
}

void QuicklaunchApplet::dropApp(QGraphicsSceneDragDropEvent *event, bool droppedOnDialog)
{
    int pos = 0;
    if (droppedOnDialog) {
        QPointF point = event->pos();
        for (int i = 0; i < m_dialogLayout->count(); i++) {
           QGraphicsLayoutItem *item = m_dialogLayout->itemAt(i);
           if (item->geometry().contains(point)) {
               //m_dialogLayout->insertItem(dropedItem, i + 1);
               pos = i + 1 + m_visibleIcons;
               kDebug() << "The position of Droped Item in dialog is = " << pos;
               break;
           }
        }
    }
    else if (!m_icons.isEmpty()) {
        QPointF point = mapFromScene(event->scenePos());
        const int rowCount = m_innerLayout->rowCount();
        const int colCount = m_innerLayout->columnCount();

        int row = 0;
        while ((row + 1) < rowCount && point.y() > m_innerLayout->itemAt(row + 1, 0)->geometry().center().y()) {
            row++;
        }

        int col = 0;
        while ((col + 1) < colCount && point.x() > m_innerLayout->itemAt(0, col + 1)->geometry().center().x()) {
            col++;
        }

        pos = row * colCount + col;
    }

    if (dropHandler(pos, event->mimeData())) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
        saveConfig();
        performUiRefactor();
    }
}

bool QuicklaunchApplet::eventFilter(QObject * object, QEvent * event)
{
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        m_mousePressPos = static_cast<QGraphicsSceneMouseEvent*>(event)->pos();
    }

    if (event->type() == QEvent::GraphicsSceneMouseMove) {
        QuicklaunchIcon * icon = qobject_cast<QuicklaunchIcon*>(object);
        if (m_icons.contains(icon)) {
            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
            int dragDistance = (mouseEvent->pos() - m_mousePressPos).toPoint().manhattanLength();
            if (dragDistance >= QApplication::startDragDistance()) {
                int numberOfPreviousIcons = m_icons.count();
                QMimeData *mimeData = new QMimeData();
                mimeData->setData("text/uri-list", icon->url().url().toAscii());
                mimeData->setText(mimeData->text());

                QDrag *drag = new QDrag(mouseEvent->widget());
                drag->setMimeData(mimeData);
                drag->setPixmap(icon->icon().pixmap(m_iconSize, m_iconSize));

                Qt::DropAction dropAction = drag->exec();

                // If the current number of icons is more than the previous amount then
                // it means that the icon was re-arranged and not dragged outside the applet
                // Therefore we remove the previous icon
                if (dropAction == Qt::MoveAction && m_icons.count() > numberOfPreviousIcons) {
                    m_icons.removeAll(icon);
                    icon->hide();
                    icon->deleteLater();
                    performUiRefactor();
                }
            }
        }
    }

    if (event->type() == QEvent::GraphicsSceneDrop) {
        dropApp(static_cast<QGraphicsSceneDragDropEvent*>(event), object == m_dialogWidget);
        return true;
    }

    return QObject::eventFilter(object, event);
}

void QuicklaunchApplet::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setDropAction(Qt::MoveAction);
    event->setAccepted(event->mimeData()->hasUrls());
}

void QuicklaunchApplet::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setDropAction(Qt::MoveAction);
    event->setAccepted(event->mimeData()->hasUrls());
}

void QuicklaunchApplet::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    dropApp(event, false);
}

void QuicklaunchApplet::addProgram(int index, const QString &url, bool isNewIcon)
{
    if (index < 0 || index > m_icons.size()) {
        index = m_icons.size();
    }

    if (url.isEmpty()) {
        return;
    }

    KUrl appUrl = KUrl(url);
    KIcon icon;
    QString text;
    QString genericName;

    if (appUrl.isLocalFile() && KDesktopFile::isDesktopFile(appUrl.toLocalFile())) {
        KDesktopFile *f = new KDesktopFile(appUrl.toLocalFile());

        text = f->readName();
        icon = KIcon(f->readIcon());
        genericName = f->readGenericName();
        delete f;
    } else {
        icon = KIcon(KMimeType::iconNameForUrl(appUrl));
    }

    if (text.isNull()) {
        text = appUrl.fileName();
    }

    if (icon.isNull()) {
        icon = KIcon("unknown");
    }

    QuicklaunchIcon *container = new QuicklaunchIcon(appUrl, text, icon, genericName, this);
    if (m_showIconNames) {
        container->setText(container->appName());
    }
    container->installEventFilter(this);

    m_icons.insert(index, container);

    if (isNewIcon) {
        saveConfig();
    }
}

void QuicklaunchApplet::clearLayout(QGraphicsLayout *layout)
{
    while (layout->count() > 0) {
        layout->removeAt(0);
    }
}

void QuicklaunchApplet::removeCurrentIcon()
{
    m_icons.removeAll(m_rightClickedIcon);
    m_rightClickedIcon->hide();
    m_rightClickedIcon->deleteLater();
    performUiRefactor();
    saveConfig();
}

bool QuicklaunchApplet::dropHandler(const int pos, const QMimeData *mimedata)
{
    Q_ASSERT(pos >= 0 && pos <= m_icons.size());

    if (!KUrl::List::canDecode(mimedata)) {
        return false;
    }

    KUrl::List urls = KUrl::List::fromMimeData(mimedata);

    if (urls.isEmpty()) {
        return false;
    }

    bool rtn = false;


    foreach (const KUrl &url, urls) {
        foreach (QuicklaunchIcon *icon, m_icons) {
            if (icon->url().url() == url.url()) {
                int icon_pos = m_icons.indexOf(icon);

                if (pos == m_icons.size()) {
                    m_icons.removeAt(icon_pos);
                    m_icons.append(icon);
                } else {
                    m_icons.move(icon_pos, pos);
                }

                urls.removeOne(url);
                rtn = true;
                break;
            }
        }
    }

    if (urls.count()) {
        foreach (const KUrl &url, urls) {
            if (KDesktopFile::isDesktopFile(url.toLocalFile())) {
                addProgram(pos, url.toLocalFile(), true);
            }
        }
        rtn = true;
    }
    return rtn;
}


void QuicklaunchApplet::showAddInterface()
{
    KOpenWithDialog appChooseDialog(NULL);
    appChooseDialog.hideRunInTerminal();
    appChooseDialog.setSaveNewApplications(true);

    if (appChooseDialog.exec() == QDialog::Accepted) {
        QString programPath = appChooseDialog.service()->entryPath();
        if (appChooseDialog.service()->icon() == NULL) {
            // If the program chosen doesn't have an icon, then we give
            // it a default icon and open up its properties in a dialog
            // so the user can change it's icon and name etc
            KConfig kc(programPath, KConfig::SimpleConfig);
            KConfigGroup kcg = kc.group("Desktop Entry");
            kcg.writeEntry("Icon","system-run");
            kc.sync();

            KPropertiesDialog propertiesDialog(KUrl(programPath), NULL);
            if (propertiesDialog.exec() != QDialog::Accepted) {
                return;
            }

            // In case the name changed
            programPath = propertiesDialog.kurl().path();
        }

        KUrl::List uniqueUrls = removeDuplicateUrls(KUrl::List(programPath));
        if (uniqueUrls.count() == 1) {
            int pos =
                m_rightClickedIcon
                    ? m_icons.indexOf(m_rightClickedIcon)
                    : m_icons.size();
            addProgram(pos, programPath, true);
            performUiRefactor();
        }
    }
}

KUrl::List QuicklaunchApplet::removeDuplicateUrls(const KUrl::List &urls)
{
    QStringList duplicateWarningMessage;
    duplicateWarningMessage.clear();

    KUrl::List uniqueUrls = urls;
    foreach (const KUrl &url, urls) {
        foreach (QuicklaunchIcon *icon, m_icons) {
            if (icon->url().url() == url.url()) {
                duplicateWarningMessage << url.pathOrUrl();
                uniqueUrls.removeOne(url);
                break;
            }
        }
    }

    if (!duplicateWarningMessage.isEmpty()) {
        KDialog *warningDialog = new KDialog;
        warningDialog->setButtons(KDialog::Ok);
        warningDialog->setModal(false);
        warningDialog->setDefaultButton(KDialog::Ok);

        QString warningMessage;
        int count = duplicateWarningMessage.count();

        foreach (const QString &str, duplicateWarningMessage) {
            warningMessage += str + '\n';
        }

        KMessageBox::createKMessageBox(warningDialog,
            QMessageBox::Warning,
	    // xgettext:no-c-format
            i18np("The following is already in quicklaunch, ignoring it:\n%2",
                  "The following are already in quicklaunch, ignoring them:\n%2",
                  count,
                  warningMessage),
            QStringList(),
            QString(""),
            NULL,
            KMessageBox::NoExec);

        warningDialog->resize(400, 300);
        warningDialog->show();
    }

    return uniqueUrls;
}

K_EXPORT_PLASMA_APPLET(quicklaunch, QuicklaunchApplet)

#include "quicklaunchApplet.moc"
