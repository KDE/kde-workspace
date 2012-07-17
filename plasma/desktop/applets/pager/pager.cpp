/***************************************************************************
 *   Copyright (C) 2007 by Daniel Laidig <d.laidig@gmx.de>                 *
 *   Copyright (C) 2012 by Luís Gabriel Lima <lampih@gmail.com>            *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "pager.h"

#include <math.h>

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFont>
#include <QGraphicsSceneHoverEvent>
#include <QTimer>
#include <QX11Info>
#include <QDBusInterface>
#include <QTextDocument>
#include <QPropertyAnimation>
#include <QApplication>
#include <QDesktopWidget>
#include <QtGui/QGraphicsLinearLayout>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>

#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KColorScheme>
#include <KConfigDialog>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KSharedConfig>
#include <KWindowSystem>
#include <NETRootInfo>

#include <Plasma/Svg>
#include <Plasma/FrameSvg>
#include <Plasma/PaintUtils>
#include <Plasma/Theme>
#include <Plasma/ToolTipManager>
#include <Plasma/Animator>
#include <Plasma/DeclarativeWidget>
#include <Plasma/Package>

#include <KActivities/Consumer>

#include <taskmanager/task.h>

const int FAST_UPDATE_DELAY = 100;
const int UPDATE_DELAY = 500;
const int DRAG_SWITCH_DELAY = 1000;
const int MAXDESKTOPS = 20;
// random(), find a less magic one if you can. -sreich
const qreal MAX_TEXT_WIDTH = 800;

DesktopRectangle::DesktopRectangle(QObject *parent)
    : QObject(parent),
      m_alpha(0)
{
}

QPropertyAnimation *DesktopRectangle::animation() const
{
    return m_animation.data();
}

void DesktopRectangle::setAnimation(QPropertyAnimation *animation)
{
    m_animation = animation;
}

qreal DesktopRectangle::alphaValue() const
{
    return m_alpha;
}

void DesktopRectangle::setAlphaValue(qreal value)
{
    m_alpha = value;

    Pager *parentItem = qobject_cast<Pager*>(parent());
    parentItem->update();
}

Pager::Pager(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_displayedText(None),
      m_currentDesktopSelected(DoNothing),
      m_showWindowIcons(false),
      m_showOwnBackground(false),
      m_rows(2),
      m_columns(0),
      m_desktopDown(false),
      m_hoverIndex(-1),
      m_addDesktopAction(0),
      m_removeDesktopAction(0),
      m_colorScheme(0),
      m_verticalFormFactor(false),
      m_dragId(0),
      m_dragStartDesktop(-1),
      m_dragHighlightedDesktop(-1),
      m_dragSwitchDesktop(-1),
      m_ignoreNextSizeConstraint(false),
      m_configureDesktopsWidget(0),
      m_desktopWidget(qApp->desktop())
{
    setAcceptsHoverEvents(true);
    setAcceptDrops(true);
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    // initialize with a decent default
    m_desktopCount = KWindowSystem::numberOfDesktops();
    m_size = QSizeF(176, 88);
    resize(m_size);
}

Pager::~Pager()
{
    delete m_colorScheme;
}

void Pager::init()
{
    initDeclarativeUI();
    createMenu();

    m_verticalFormFactor = (formFactor() == Plasma::Vertical);

    configChanged();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(recalculateWindowRects()));

    m_dragSwitchTimer = new QTimer(this);
    m_dragSwitchTimer->setSingleShot(true);
    connect(m_dragSwitchTimer, SIGNAL(timeout()), this, SLOT(dragSwitch()));

    connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(currentDesktopChanged(int)));
    connect(KWindowSystem::self(), SIGNAL(windowAdded(WId)), this, SLOT(startTimerFast()));
    connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(startTimerFast()));
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(startTimerFast()));
    connect(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)), this, SLOT(numberOfDesktopsChanged(int)));
    connect(KWindowSystem::self(), SIGNAL(desktopNamesChanged()), this, SLOT(desktopNamesChanged()));
    connect(KWindowSystem::self(), SIGNAL(stackingOrderChanged()), this, SLOT(startTimerFast()));
    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId,const ulong*)), this, SLOT(windowChanged(WId,const ulong*)));
    connect(KWindowSystem::self(), SIGNAL(showingDesktopChanged(bool)), this, SLOT(startTimer()));
    connect(m_desktopWidget, SIGNAL(screenCountChanged(int)), SLOT(desktopsSizeChanged()));
    connect(m_desktopWidget, SIGNAL(resized(int)), SLOT(desktopsSizeChanged()));

    // connect to KWin's reloadConfig signal to get updates on the desktop layout
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect(QString(), "/KWin", "org.kde.KWin", "reloadConfig",
                 this, SLOT(configChanged()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeRefresh()));

    recalculateGridSizes(m_rows);

    m_currentDesktop = KWindowSystem::currentDesktop();

    KActivities::Consumer *act = new KActivities::Consumer(this);
    connect(act, SIGNAL(currentActivityChanged(QString)), this, SLOT(currentActivityChanged(QString)));
    m_currentActivity = act->currentActivity();
}

void Pager::initDeclarativeUI()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    m_declarativeWidget = new Plasma::DeclarativeWidget(this);
    layout->addItem(m_declarativeWidget);

    m_pagerModel = new VirtualDesktopModel(this);
    m_declarativeWidget->engine()->rootContext()->setContextProperty("pagerModel", m_pagerModel);

    Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
    m_package = new Plasma::Package(QString(), "org.kde.pager", structure);
    m_declarativeWidget->setQmlPath(m_package->filePath("mainscript"));
}

void Pager::configChanged()
{
    KConfigGroup cg = config();
    bool changed = false;

    DisplayedText displayedText = (DisplayedText) cg.readEntry("displayedText", (int) m_displayedText);
    if (displayedText != m_displayedText) {
        m_displayedText = displayedText;
        changed = true;
    }

    bool showWindowIcons = cg.readEntry("showWindowIcons", m_showWindowIcons);
    if (showWindowIcons != m_showWindowIcons) {
        m_showWindowIcons = showWindowIcons;
        changed = true;
    }

    CurrentDesktopSelected currentDesktopSelected =
        (CurrentDesktopSelected) cg.readEntry("currentDesktopSelected",
                                              (int) m_currentDesktopSelected);
    if (currentDesktopSelected != m_currentDesktopSelected) {
        m_currentDesktopSelected = currentDesktopSelected;
        changed = true;
    }

    int rows = m_rows;
#ifdef Q_WS_X11
    unsigned long properties[] = {0, NET::WM2DesktopLayout };
    NETRootInfo info(QX11Info::display(), properties, 2);
    rows = info.desktopLayoutColumnsRows().height();
#endif

    if (changed || rows != m_rows) {
        recalculateGridSizes(rows);
        recalculateWindowRects();
        update();
    }
}

void Pager::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::SizeConstraint) {
        // no need to update everything twice (if we are going to flip rows and columns later)
        if (!(constraints & Plasma::FormFactorConstraint) ||
             m_verticalFormFactor == (formFactor() == Plasma::Vertical) ||
             m_columns == m_rows) {
            // use m_ignoreNextSizeConstraint to decide whether to try to resize the plasmoid again
            updateSizes(!m_ignoreNextSizeConstraint);
            m_ignoreNextSizeConstraint = !m_ignoreNextSizeConstraint;

            recalculateWindowRects();
        }

        update();
    }

    if (constraints & Plasma::FormFactorConstraint) {

        if (m_verticalFormFactor != (formFactor() == Plasma::Vertical)) {
            m_verticalFormFactor = (formFactor() == Plasma::Vertical);
            // whenever we switch to/from vertical form factor, swap the rows and columns around
            if (m_columns != m_rows) {
                // pass in columns as the new rows
                recalculateGridSizes(m_columns);
                recalculateWindowRects();
                update();
            }
        }

        if (formFactor() == Plasma::Horizontal) {
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
            setMinimumSize(preferredSize().width(), 0);
        } else if (formFactor() == Plasma::Vertical) {
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            setMinimumSize(0, preferredSize().height());
        } else {
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            setMinimumSize(preferredSize());
        }
    }
}

KColorScheme *Pager::colorScheme()
{
    if (!m_colorScheme) {
        m_colorScheme = new KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme());
    }

    return m_colorScheme;
}

void Pager::createMenu()
{
#ifdef Q_WS_X11
    m_addDesktopAction = new QAction(SmallIcon("list-add"),i18n("&Add Virtual Desktop"), this);
    m_actions.append(m_addDesktopAction);
    connect(m_addDesktopAction, SIGNAL(triggered(bool)), this , SLOT(slotAddDesktop()));
    m_removeDesktopAction = new QAction(SmallIcon("list-remove"),i18n("&Remove Last Virtual Desktop"), this);
    m_actions.append(m_removeDesktopAction);
    connect(m_removeDesktopAction, SIGNAL(triggered(bool)), this , SLOT(slotRemoveDesktop()));

    if (m_desktopCount <= 1) {
        m_removeDesktopAction->setEnabled(false);
    } else if (m_desktopCount >= MAXDESKTOPS) {
        m_addDesktopAction->setEnabled(false);
    }
#endif
}

QList<QAction*> Pager::contextualActions()
{
  return m_actions;
}

#ifdef Q_WS_X11
void Pager::slotAddDesktop()
{
    NETRootInfo info(QX11Info::display(), NET::NumberOfDesktops);
    info.setNumberOfDesktops(info.numberOfDesktops() + 1);
}

void Pager::slotRemoveDesktop()
{
    NETRootInfo info(QX11Info::display(), NET::NumberOfDesktops);
    int desktops = info.numberOfDesktops();
    if (desktops > 1) {
        info.setNumberOfDesktops(info.numberOfDesktops() - 1);
    }
}
#endif

void Pager::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    m_configureDesktopsWidget = new KCModuleProxy("desktop");

    parent->addPage(widget, i18n("General"), icon());
    parent->addPage(m_configureDesktopsWidget, m_configureDesktopsWidget->moduleInfo().moduleName(),
                    m_configureDesktopsWidget->moduleInfo().icon());

    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));

    switch (m_displayedText){
        case Number:
            ui. desktopNumberRadioButton->setChecked(true);
            break;

        case Name:
            ui.desktopNameRadioButton->setChecked(true);
            break;

        case None:
            ui.displayNoneRadioButton->setChecked(true);
            break;
    }

    ui.showWindowIconsCheckBox->setChecked(m_showWindowIcons);

    switch (m_currentDesktopSelected){
        case DoNothing:
            ui.doNothingRadioButton->setChecked(true);
            break;

        case ShowDesktop:
            ui.showDesktopRadioButton->setChecked(true);
            break;

        case ShowDashboard:
            ui.showDashboardRadioButton->setChecked(true);
            break;
    }

    connect(ui.desktopNumberRadioButton, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.desktopNameRadioButton, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.displayNoneRadioButton, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.showWindowIconsCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.doNothingRadioButton, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.showDesktopRadioButton, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(ui.showDashboardRadioButton, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));

    connect(m_configureDesktopsWidget, SIGNAL(changed(bool)), parent, SLOT(settingsModified()));
}

void Pager::recalculateGridSizes(int rows)
{
    // recalculate the number of rows and columns in the grid
    rows = qBound(1, rows, m_desktopCount);
    // avoid weird cases like having 3 rows for 4 desktops, where the last row is unused
    int columns = m_desktopCount / rows;
    if (m_desktopCount % rows > 0) {
        columns++;
    }
    rows = m_desktopCount / columns;
    if (m_desktopCount % columns > 0) {
        rows++;
    }

    // update the grid size
    if (m_rows != rows || m_columns != columns) {
        m_rows = rows;
        m_columns = columns;
    }

    updateSizes(true);
}

QRectF Pager::mapToDeclarativeUI(const QRectF &rect) const
{
    QPointF p = mapToItem(m_declarativeWidget, rect.x(), rect.y());
    return QRectF(p, rect.size());
}

void Pager::updateSizes(bool allowResize)
{
    int padding = 2; // Space between miniatures of desktops
    int textMargin = 3; // Space between name of desktop and border

    qreal leftMargin = 0;
    qreal topMargin = 0;
    qreal rightMargin = 0;
    qreal bottomMargin = 0;

    QRect totalRect;
    for (int x = 0; x < m_desktopWidget->screenCount(); x++) {
        totalRect |= m_desktopWidget->screenGeometry(x);
    }

    qreal ratio = (qreal) totalRect.width() /
                  (qreal) totalRect.height();

    // calculate the margins
    if (formFactor() == Plasma::Vertical || formFactor() == Plasma::Horizontal) {
        if (formFactor() == Plasma::Vertical) {
            qreal optimalSize = (geometry().width() -
                                 KIconLoader::SizeSmall * ratio * m_columns -
                                 padding * (m_columns - 1)) / 2;

            if (optimalSize < leftMargin || optimalSize < rightMargin) {
                leftMargin = rightMargin = qMax(qreal(0), optimalSize);
                m_showOwnBackground = false;
            }
        } else if (formFactor() == Plasma::Horizontal) {
            qreal optimalSize = (geometry().height() -
                                 KIconLoader::SizeSmall * m_rows -
                                 padding * (m_rows - 1)) / 2;

            if (optimalSize < topMargin || optimalSize < bottomMargin) {
                topMargin = bottomMargin = qMax(qreal(0), optimalSize);
                m_showOwnBackground = false;
            }
        }
    } else {
        getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
    }


    qreal itemHeight;
    qreal itemWidth;
    qreal preferredItemHeight;
    qreal preferredItemWidth;

    if (formFactor() == Plasma::Vertical) {
        // work out the preferred size based on the width of the contentsRect
        preferredItemWidth = (contentsRect().width() - leftMargin - rightMargin -
                              padding * (m_columns - 1)) / m_columns;
        preferredItemHeight = preferredItemWidth / ratio;
        // make sure items of the new size actually fit in the current contentsRect
        itemHeight = (contentsRect().height() - topMargin - bottomMargin -
                      padding * (m_rows - 1)) / m_rows;
        if (itemHeight > preferredItemHeight) {
            itemHeight = preferredItemHeight;
        }
        itemWidth = itemHeight * ratio;

        m_widthScaleFactor = itemWidth / m_desktopWidget->geometry().width();
        m_heightScaleFactor = itemHeight / m_desktopWidget->geometry().height();
    } else {
        // work out the preferred size based on the height of the contentsRect
        if (formFactor() == Plasma::Horizontal) {
            preferredItemHeight = (contentsRect().height() - topMargin - bottomMargin -
                                   padding * (m_rows - 1)) / m_rows;
        } else {
            preferredItemHeight = (contentsRect().height() - padding * (m_rows - 1)) / m_rows;
        }
        preferredItemWidth = preferredItemHeight * ratio;

        if (m_displayedText == Name) {
            // When containment is in this position we are not limited by low width and we can
            // afford increasing width of applet to be able to display every name of desktops
            for (int i = 0; i < m_desktopCount; i++) {
                QFontMetricsF metrics(KGlobalSettings::taskbarFont());
                QSizeF textSize = metrics.size(Qt::TextSingleLine, KWindowSystem::desktopName(i+1));
                if (textSize.width() + textMargin * 2 > preferredItemWidth) {
                     preferredItemWidth = textSize.width() + textMargin * 2;
                }
            }
        }

        // make sure items of the new size actually fit in the current contentsRect
        if (formFactor() == Plasma::Horizontal) {
            itemWidth = (contentsRect().width() - leftMargin - rightMargin -
                         padding * (m_columns - 1)) / m_columns;
        } else {
            itemWidth = (contentsRect().width() - padding * (m_columns - 1)) / m_columns;
        }
        if (itemWidth > preferredItemWidth) {
            itemWidth = preferredItemWidth;
        }
        itemHeight = preferredItemHeight;
        if (itemWidth < itemHeight * ratio) {
            itemHeight = itemWidth / ratio;
        }

        m_widthScaleFactor = itemWidth / m_desktopWidget->geometry().width();
        m_heightScaleFactor = itemHeight / m_desktopWidget->geometry().height();
    }

    m_hoverRect = QRectF();
    m_pagerModel->clearDesktopRects();
    qDeleteAll(m_animations);
    m_animations.clear();

    QRectF itemRect(QPoint(leftMargin, topMargin) , QSize(floor(itemWidth), floor(itemHeight)));
    for (int i = 0; i < m_desktopCount; i++) {
        itemRect.moveLeft(leftMargin + floor((i % m_columns)  * (itemWidth + padding)));
        itemRect.moveTop(topMargin + floor((i / m_columns) * (itemHeight + padding)));
        m_pagerModel->appendDesktopRect(mapToDeclarativeUI(itemRect));
        m_animations.append(new DesktopRectangle(this));
    }

    if (m_hoverIndex >= m_animations.count()) {
        m_hoverIndex = -1;
    }

    // do not try to resize unless the caller has allowed it,
    // or the height has changed (or the width has changed in a vertical panel)
    if (allowResize ||
        (formFactor() != Plasma::Vertical && contentsRect().height() != m_size.height()) ||
        (formFactor() == Plasma::Vertical && contentsRect().width()  != m_size.width())) {

        // this new size will have the same height/width as the horizontal/vertical panel has given it
        QSizeF preferred = QSizeF(ceil(m_columns * preferredItemWidth + padding * (m_columns - 1) +
                                       leftMargin + rightMargin),
                                  ceil(m_rows * preferredItemHeight + padding * (m_rows - 1) +
                                       topMargin + bottomMargin));

        //kDebug() << "current size:" << contentsRect() << " new preferred size: " << preferred << " form factor:" << formFactor() << " grid:" << m_rows << "x" << m_columns <<
        //            " actual grid:" << rows << "x" << columns << " item size:" << itemWidth << "x" << itemHeight << " preferred item size:" << preferredItemWidth << "x" << preferredItemHeight;

        // make sure the minimum size is smaller than preferred
        setMinimumSize(qMin(preferred.width(),  minimumSize().width()),
                       qMin(preferred.height(), minimumSize().height()));
        setPreferredSize(preferred);
        emit sizeHintChanged(Qt::PreferredSize);
    }

    m_size = contentsRect().size();
}

void Pager::recalculateWindowRects()
{
    QList<WId> windows = KWindowSystem::stackingOrder();
    m_pagerModel->clearWindowRects();
    m_windowInfo.clear();

    foreach (WId window, windows) {
        KWindowInfo info = KWindowSystem::windowInfo(window, NET::WMGeometry | NET::WMFrameExtents |
                                                             NET::WMWindowType | NET::WMDesktop |
                                                             NET::WMState | NET::XAWMState | NET::WMVisibleName);
        NET::WindowType type = info.windowType(NET::NormalMask | NET::DialogMask | NET::OverrideMask |
                                               NET::UtilityMask | NET::DesktopMask | NET::DockMask |
                                               NET::TopMenuMask | NET::SplashMask | NET::ToolbarMask |
                                               NET::MenuMask);

        // the reason we don't check for -1 or Net::Unknown here is that legitimate windows, such
        // as some java application windows, may not have a type set for them.
        // apparently sane defaults on properties is beyond the wisdom of x11.
        if (type == NET::Desktop || type == NET::Dock || type == NET::TopMenu ||
            type == NET::Splash || type == NET::Menu || type == NET::Toolbar ||
            info.hasState(NET::SkipPager) || info.isMinimized()) {
            continue;
        }

        //check activity
        unsigned long properties[] = { 0, NET::WM2Activities };
        NETWinInfo netInfo(QX11Info::display(), window, QX11Info::appRootWindow(), properties, 2);
        QString result(netInfo.activities());
        if (!result.isEmpty() && result != "ALL") {
            QStringList activities = result.split(',');
            if (!activities.contains(m_currentActivity)) {
                continue;
            }
        }

        for (int i = 0; i < m_desktopCount; i++) {
            if (!info.isOnDesktop(i+1)) {
                continue;
            }

            QRectF windowRect = info.frameGeometry();

            if (KWindowSystem::mapViewport()) {
                windowRect = fixViewportPosition(windowRect.toRect());
            }

            windowRect = QRectF(windowRect.x() * m_widthScaleFactor,
                                windowRect.y() * m_heightScaleFactor,
                                windowRect.width() * m_widthScaleFactor,
                                windowRect.height() * m_heightScaleFactor).toRect();

            bool active = (window == KWindowSystem::activeWindow());
            m_pagerModel->appendWindowRect(i, window, windowRect, active);
            m_windowInfo.append(info);
        }
    }

    update();
}

void Pager::configAccepted()
{
    // only write the config here, it will be loaded in by configChanged(),
    // which is called after this when the config dialog is accepted
    KConfigGroup cg = config();

    DisplayedText displayedText;
    if (ui.desktopNumberRadioButton->isChecked()) {
        displayedText = Number;
    } else if (ui.desktopNameRadioButton->isChecked()) {
        displayedText = Name;
    } else {
        displayedText = None;
    }
    cg.writeEntry("displayedText", (int) displayedText);

    cg.writeEntry("showWindowIcons", ui.showWindowIconsCheckBox->isChecked());

    CurrentDesktopSelected currentDesktopSelected;
    if (ui.doNothingRadioButton->isChecked()) {
        currentDesktopSelected = DoNothing;
    } else if (ui.showDesktopRadioButton->isChecked()) {
        currentDesktopSelected = ShowDesktop;
    } else {
        currentDesktopSelected = ShowDashboard;
    }
    cg.writeEntry("currentDesktopSelected", (int) currentDesktopSelected);

    m_configureDesktopsWidget->save();

    emit configNeedsSaving();
}

void Pager::currentDesktopChanged(int desktop)
{
    if (desktop < 1) {
        return; // bogus value, don't accept it
    }

    m_currentDesktop = desktop;
    m_desktopDown = false;
    startTimerFast();
}

void Pager::currentActivityChanged(const QString &activity)
{
    m_currentActivity = activity;
    startTimerFast();
}

void Pager::numberOfDesktopsChanged(int num)
{
    if (num < 1) {
        return; // refuse to update to zero desktops
    }

#ifdef Q_WS_X11
    m_removeDesktopAction->setEnabled(num > 1);
    m_addDesktopAction->setEnabled(num < MAXDESKTOPS);
#endif

    m_desktopCount = num;

    m_pagerModel->clearDesktopRects();
    recalculateGridSizes(m_rows);
    recalculateWindowRects();
}

void Pager::desktopNamesChanged()
{
    m_pagerModel->clearDesktopRects();
    updateSizes(true);
    startTimer();
}

void Pager::windowChanged(WId id, const unsigned long* dirty)
{
    Q_UNUSED(id)

    if (dirty[NETWinInfo::PROTOCOLS] & (NET::WMGeometry | NET::WMDesktop) ||
        dirty[NETWinInfo::PROTOCOLS2] & NET::WM2Activities) {
        startTimer();
    }
}

void Pager::desktopsSizeChanged()
{
    m_pagerModel->clearDesktopRects();
    updateSizes(true);
    startTimer();
}

void Pager::startTimer()
{
    if (m_timer->isActive()) {
        return;
    }

    m_timer->start(UPDATE_DELAY);
}

void Pager::startTimerFast()
{
    if (m_timer->isActive()) {
        return;
    }

    m_timer->start(FAST_UPDATE_DELAY);
}

void Pager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() != Qt::RightButton)
    {
        for (int i = 0; i < m_pagerModel->rowCount(); ++i) {
            if (m_pagerModel->desktopRectAt(i).contains(event->pos())) {
                m_dragStartDesktop = m_dragHighlightedDesktop = i;
                m_dragOriginalPos = m_dragCurrentPos = event->pos();
                if (m_dragOriginal.isEmpty()) {
                    m_dragOriginal = m_pagerModel->desktopRectAt(i).toRect();
                }

                update();
                return;
            }
        }
    }
    Applet::mousePressEvent(event);
}

void Pager::wheelEvent(QGraphicsSceneWheelEvent *e)
{
    int newDesk;
    int desktops = KWindowSystem::numberOfDesktops();

    /*
       if (m_kwin->numberOfViewports(0).width() * m_kwin->numberOfViewports(0).height() > 1 )
       desktops = m_kwin->numberOfViewports(0).width() * m_kwin->numberOfViewports(0).height();
       */
    if (e->delta() < 0) {
        newDesk = m_currentDesktop % desktops + 1;
    } else {
        newDesk = (desktops + m_currentDesktop - 2) % desktops + 1;
    }

    KWindowSystem::setCurrentDesktop(newDesk);
    m_currentDesktop = newDesk;
    update();

    Applet::wheelEvent(e);
}

void Pager::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragId > 0) {
        m_dragCurrentPos = event->pos();
        m_dragHighlightedDesktop = -1;
        m_hoverRect = QRectF();
        int i = 0;
        for (int k = 0; k < m_pagerModel->rowCount(); k++) {
            const QRectF &rect = m_pagerModel->desktopRectAt(k);
            if (rect.contains(event->pos())) {
                m_dragHighlightedDesktop = i;
                m_hoverRect = rect;
                break;
            }

            ++i;
        }
        update();
        event->accept();
        return;
    } else if (m_dragStartDesktop != -1 &&
               (event->pos() - m_dragOriginalPos).toPoint().manhattanLength() > KGlobalSettings::dndEventDelay()) {
        m_dragId = 0; // prevent us from going through this more than once
        WindowModel *windows= m_pagerModel->windowsAt(m_dragStartDesktop);
        for (int k = windows->rowCount() - 1; k >= 0 ; k--) {
            if (windows->rectAt(k).contains(m_dragOriginalPos.toPoint())) {
                m_dragOriginal = windows->rectAt(k);
                m_dragId = windows->idAt(k);
                event->accept();
                break;
            }
        }
    }

    if (m_dragOriginal.isEmpty()) {
        Applet::mouseMoveEvent(event);
    }
}

void Pager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragId) {
        if (m_dragHighlightedDesktop != -1) {
            QPointF dest = m_dragCurrentPos
                           - m_pagerModel->desktopRectAt(m_dragHighlightedDesktop).topLeft()
                           - m_dragOriginalPos + m_dragOriginal.topLeft();
            dest = QPointF(dest.x()/m_widthScaleFactor, dest.y()/m_heightScaleFactor);
            // don't move windows to negative positions
            dest = QPointF(qMax(dest.x(), qreal(0.0)), qMax(dest.y(), qreal(0.0)));
            if (!KWindowSystem::mapViewport()) {
                KWindowInfo info = KWindowSystem::windowInfo(m_dragId, NET::WMDesktop | NET::WMState);

                if (!info.onAllDesktops()) {
                    KWindowSystem::setOnDesktop(m_dragId, m_dragHighlightedDesktop+1);
                }

                // only move the window if it is not full screen and if it is kept within the same desktop
                // moving when dropping between desktop is too annoying due to the small drop area.
                if (!(info.state() & NET::FullScreen) &&
                    (m_dragHighlightedDesktop == m_dragStartDesktop || info.onAllDesktops())) {
                    // use _NET_MOVERESIZE_WINDOW rather than plain move, so that the WM knows this is a pager request
                    NETRootInfo i( QX11Info::display(), 0 );
                    int flags = ( 0x20 << 12 ) | ( 0x03 << 8 ) | 1; // from tool, x/y, northwest gravity
                    i.moveResizeWindowRequest( m_dragId, flags, dest.toPoint().x(), dest.toPoint().y(), 0, 0 );
                }
            } else {
                // setOnDesktop() with viewports is also moving a window, and since it takes a moment
                // for the WM to do the move, there's a race condition with figuring out how much to move,
                // so do it only as one move
                dest += KWindowSystem::desktopToViewport( m_dragHighlightedDesktop+1, false );
                QPoint d = KWindowSystem::constrainViewportRelativePosition( dest.toPoint());
                NETRootInfo i( QX11Info::display(), 0 );
                int flags = ( 0x20 << 12 ) | ( 0x03 << 8 ) | 1; // from tool, x/y, northwest gravity
                i.moveResizeWindowRequest( m_dragId, flags, d.x(), d.y(), 0, 0 );
            }
        }
        m_timer->start();
    } else if (m_dragStartDesktop != -1 && m_dragStartDesktop < m_pagerModel->rowCount() &&
               m_pagerModel->desktopRectAt(m_dragStartDesktop).contains(event->pos()) &&
               m_currentDesktop != m_dragStartDesktop + 1) {
        // only change the desktop if the user presses and releases the mouse on the same desktop
        KWindowSystem::setCurrentDesktop(m_dragStartDesktop + 1);
        m_currentDesktop = m_dragStartDesktop + 1;
    } else if (m_dragStartDesktop != -1 && m_dragStartDesktop < m_pagerModel->rowCount() &&
               m_pagerModel->desktopRectAt(m_dragStartDesktop).contains(event->pos()) &&
               m_currentDesktop == m_dragStartDesktop + 1) {
        // toogle the desktop or the dashboard
        // if the user presses and releases the mouse on the current desktop, default option is do nothing
        if (m_currentDesktopSelected == ShowDesktop) {

            NETRootInfo info(QX11Info::display(), 0);
            m_desktopDown = !m_desktopDown;
            info.setShowingDesktop(m_desktopDown);
        } else if (m_currentDesktopSelected == ShowDashboard) {
            QDBusInterface plasmaApp("org.kde.plasma-desktop", "/App");
            plasmaApp.call("toggleDashboard");
        }
    }

    m_dragId = 0;
    m_dragOriginal = QRect();
    m_dragHighlightedDesktop = -1;
    m_dragStartDesktop = -1;
    m_dragOriginalPos = m_dragCurrentPos = QPointF();

    update();
    Applet::mouseReleaseEvent(event);
}

// If the pager is hovered in drag and drop mode, no hover events are geneated.
// This method provides the common implementation for hoverMoveEvent and dragMoveEvent.
void Pager::handleHoverMove(const QPointF& pos)
{
    if (m_hoverRect.contains(pos)) {
        return;
    } else if (m_hoverIndex > -1) {
        QPropertyAnimation *animation = m_animations[m_hoverIndex]->animation();
        if (animation && animation->state() == QAbstractAnimation::Running) {
            animation->pause();
        } else {
            animation = new QPropertyAnimation(m_animations[m_hoverIndex], "alphaValue");
            m_animations[m_hoverIndex]->setAnimation(animation);
        }

        animation->setDuration(s_FadeOutDuration);
        animation->setEasingCurve(QEasingCurve::OutQuad);
        animation->setStartValue(1);
        animation->setEndValue(0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    int i = 0;
    for (int j = 0; j < m_pagerModel->rowCount(); j++) {
        const QRectF &rect = m_pagerModel->desktopRectAt(i);
        if (rect.contains(pos)) {
            if (m_hoverRect != rect) {
                m_hoverRect = rect;
                m_hoverIndex = i;

                QPropertyAnimation *animation = m_animations[m_hoverIndex]->animation();
                if (animation && animation->state() == QAbstractAnimation::Running) {
                    animation->pause();
                } else {
                    animation = new QPropertyAnimation(m_animations[m_hoverIndex], "alphaValue");
                    m_animations[m_hoverIndex]->setAnimation(animation);
                }

                animation->setDuration(s_FadeInDuration);
                animation->setEasingCurve(QEasingCurve::InQuad);
                animation->setStartValue(0);
                animation->setEndValue(1);
                animation->start(QAbstractAnimation::DeleteWhenStopped);

                update();
                updateToolTip();
            }
            return;
        }
        ++i;
    }

    m_hoverIndex = -1;
    m_hoverRect = QRectF();
    update();
}

// If the pager is hovered in drag and drop mode, no hover events are geneated.
// This method provides the common implementation for hoverLeaveEvent and dragLeaveEvent.
void Pager::handleHoverLeave()
{
    if (m_hoverRect != QRectF()) {
        m_hoverRect = QRectF();
        update();
    }

    if (m_hoverIndex != -1) {
        QPropertyAnimation *animation = m_animations[m_hoverIndex]->animation();
        if (animation && animation->state() == QAbstractAnimation::Running) {
            animation->pause();
        } else {
            animation = new QPropertyAnimation(m_animations[m_hoverIndex], "alphaValue");
            m_animations[m_hoverIndex]->setAnimation(animation);
        }

        animation->setDuration(s_FadeOutDuration);
        animation->setEasingCurve(QEasingCurve::OutQuad);
        animation->setStartValue(1);
        animation->setEndValue(0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);

        m_hoverIndex = -1;
    }

    // The applet doesn't always get mouseReleaseEvents, for example when starting a drag
    // on the containment and releasing the mouse on the desktop or another window. This can cause
    // weird bugs because the pager still thinks a drag is going on.
    // The only reliable event I found is the hoverLeaveEvent, so we just stop the drag
    // on this event.
    if (m_dragId || m_dragStartDesktop != -1) {
        m_dragId = 0;
        m_dragOriginal = QRect();
        m_dragHighlightedDesktop = -1;
        m_dragStartDesktop = -1;
        m_dragOriginalPos = m_dragCurrentPos = QPointF();
        update();
    }
}

void Pager::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    handleHoverMove(event->pos());
    Applet::hoverEnterEvent(event);
}

void Pager::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    handleHoverMove(event->pos());
}

void Pager::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    handleHoverLeave();
    Applet::hoverLeaveEvent(event);
}

void Pager::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(true);
    if (event->mimeData()->hasFormat(TaskManager::Task::mimetype())) {
        return;
    }

    handleHoverMove(event->pos());

    if (m_hoverIndex != -1) {
        m_dragSwitchDesktop = m_hoverIndex;
        m_dragSwitchTimer->start(DRAG_SWITCH_DELAY);
    }
    Applet::dragEnterEvent(event);
}

void Pager::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    handleHoverMove(event->pos());

    if (m_dragSwitchDesktop != m_hoverIndex && m_hoverIndex != -1) {
        m_dragSwitchDesktop = m_hoverIndex;
        m_dragSwitchTimer->start(DRAG_SWITCH_DELAY);
    } else if (m_hoverIndex == -1) {
        m_dragSwitchDesktop = m_hoverIndex;
        m_dragSwitchTimer->stop();
    }
    Applet::dragMoveEvent(event);
}

void Pager::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    handleHoverLeave();

    m_dragSwitchDesktop = -1;
    m_dragSwitchTimer->stop();
    Applet::dragLeaveEvent(event);
}

void Pager::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    bool ok;
    QList<WId> ids = TaskManager::Task::idsFromMimeData(event->mimeData(), &ok);
    if (ok) {
        for (int i = 0; i < m_pagerModel->rowCount(); ++i) {
            if (m_pagerModel->desktopRectAt(i).contains(event->pos().toPoint())) {
                foreach (const WId &id, ids) {
                    KWindowSystem::setOnDesktop(id, i + 1);
                }
                m_dragSwitchDesktop = -1;
                break;
            }
        }
    }
}

void Pager::dragSwitch()
{
    if (m_dragSwitchDesktop == -1) {
        return;
    }

    KWindowSystem::setCurrentDesktop(m_dragSwitchDesktop + 1);
    m_currentDesktop = m_dragSwitchDesktop + 1;
}

// KWindowSystem does not translate position when mapping viewports
// to virtual desktops (it'd probably break more things than fix),
// so the offscreen coordinates need to be fixed
QRect Pager::fixViewportPosition( const QRect& r )
{
    QRect desktopGeom = m_desktopWidget->geometry();
    int x = r.center().x() % desktopGeom.width();
    int y = r.center().y() % desktopGeom.height();
    if( x < 0 ) {
        x = x + desktopGeom.width();
    }
    if( y < 0 ) {
        y = y + desktopGeom.height();
    }
    return QRect( x - r.width() / 2, y - r.height() / 2, r.width(), r.height());
}

void Pager::themeRefresh()
{
    delete m_colorScheme;
    m_colorScheme = 0;
    update();
}

void Pager::updateToolTip()
{
    int hoverDesktopNumber = 0;

    for (int i = 0; i < m_desktopCount; ++i) {
        if (m_pagerModel->desktopRectAt(i) == m_hoverRect) {
            hoverDesktopNumber = i + 1;
        }
    }

    Plasma::ToolTipContent data;
    QString subtext;
    int taskCounter = 0;
    int displayedTaskCounter = 0;

    QList<WId> windows;

    foreach(const KWindowInfo &winInfo, m_windowInfo){
        if (winInfo.isOnDesktop(hoverDesktopNumber) && !windows.contains(winInfo.win())) {

            bool active = (winInfo.win() == KWindowSystem::activeWindow());
            if ((taskCounter < 4) || active){
                // prefer to use the System Settings specified Small icon (usually 16x16)
                // TODO: should we actually be using Small for this? or Panel, Toolbar, etc?
                int windowIconSize = KIconLoader::global()->currentSize(KIconLoader::Small);
                QPixmap icon = KWindowSystem::icon(winInfo.win(), windowIconSize, windowIconSize, true);
                if (icon.isNull()) {
                     subtext += "<br />&bull;" + Qt::escape(winInfo.visibleName());
                } else {
                    data.addResource(Plasma::ToolTipContent::ImageResource, QUrl("wicon://" + QString::number(taskCounter)), QVariant(icon));
                    subtext += "<br /><img src=\"wicon://" + QString::number(taskCounter) + "\"/>&nbsp;";
                }

                QFontMetricsF metrics(KGlobalSettings::taskbarFont());
                const QString combinedString = (active ? "<u>" : "") + Qt::escape(winInfo.visibleName()).replace(' ', "&nbsp;") + (active ? "</u>" : "");
                // elide text that is too long
                subtext += metrics.elidedText(combinedString, Qt::ElideMiddle, MAX_TEXT_WIDTH, Qt::TextShowMnemonic);

                displayedTaskCounter++;
                windows.append(winInfo.win());
            }
            taskCounter++;
        }
    }

    if (taskCounter) {
        subtext.prepend(i18np("One window:", "%1 windows:", taskCounter));
    }

    if (taskCounter - displayedTaskCounter > 0) {
        subtext.append("<br>&bull; <i>" + i18np("and 1 other", "and %1 others", taskCounter - displayedTaskCounter) + "</i>");
    }

    data.setMainText(KWindowSystem::desktopName(hoverDesktopNumber));
    data.setSubText(subtext);

    Plasma::ToolTipManager::self()->setContent(this, data);
}

#include "pager.moc"
