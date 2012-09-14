/***************************************************************************
 *   applet.cpp                                                            *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Sebastian Sauer                                    *
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>                   *
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

#include "applet.h"
#include "plasmoid.h"

#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QGraphicsLayout>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QPainter>
#include <QtGui/QX11Info>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QtDeclarative/QDeclarativeError>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeComponent>

#include <KDE/KStandardDirs>
#include <KAction>
#include <KConfigDialog>
#include <KComboBox>
#include <KWindowSystem>
#include <KCategorizedView>
#include <KCategorizedSortFilterProxyModel>
#include <KCategoryDrawer>
#include <KKeySequenceWidget>

#include <Solid/Device>

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/extendergroup.h>
#include <plasma/framesvg.h>
#include <plasma/widgets/label.h>
#include <plasma/theme.h>
#include <plasma/dataenginemanager.h>
#include <plasma/dataengine.h>
#include <Plasma/TabBar>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/IconWidget>
#include <Plasma/Dialog>
#include <Plasma/WindowEffects>
#include <KDE/Plasma/DeclarativeWidget>

#include "config.h"

#include "../core/manager.h"
#include "taskarea.h"

static const bool DEFAULT_SHOW_APPS = true;
static const bool DEFAULT_SHOW_COMMUNICATION = true;
static const bool DEFAULT_SHOW_SERVICES = true;
static const bool DEFAULT_SHOW_HARDWARE = true;
static const bool DEFAULT_SHOW_UNKNOWN = true;

namespace SystemTray
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace
{

static void _RegisterEnums(QDeclarativeContext *context, const QMetaObject &meta)
{
    for (int i = 0, s = meta.enumeratorCount(); i < s; ++i) {
        QMetaEnum e = meta.enumerator(i);
        for (int i = 0, s = e.keyCount(); i < s; ++i) {
            context->setContextProperty(e.key(i), e.value(i));
        }
    }
}

} // namespace


K_EXPORT_PLASMA_APPLET(systemtray, Applet)


Manager *Applet::s_manager = 0;
int Applet::s_managerUsage = 0;

Applet::Applet(QObject *parent, const QVariantList &arguments)
    : Plasma::Applet(parent, arguments),
      m_plasmoid(new Plasmoid(this)),
      m_widget(0),
      m_firstRun(true)
{
    if (!s_manager) {
        s_manager = new SystemTray::Manager();
    }

    ++s_managerUsage;

    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(true);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(themeChanged()));
}

Applet::~Applet()
{
    // stop listening to the manager
    disconnect(s_manager, 0, this, 0);

    foreach (Task *task, s_manager->tasks()) {
        // we don't care about the task updates anymore
        disconnect(task, 0, this, 0);

        // delete our widget (if any); some widgets (such as the extender info one)
        // may rely on the applet being around, so we need to delete them here and now
        // while we're still kicking
        delete task->widget(this, false);
    }

    delete m_widget;
    delete m_plasmoid;

    --s_managerUsage;
    if (s_managerUsage < 1) {
        delete s_manager;
        s_manager = 0;
        s_managerUsage = 0;
    }
}

void Applet::init()
{
    // Find data directory
    KStandardDirs std_dirs;
    QStringList dirs = std_dirs.findDirs("data", "plasma/plasmoids/systemtray");
    QString data_path;
    if (!dirs.isEmpty()) {
        data_path = dirs.at(0);
    } else {
        setFailedToLaunch(true, "Data directory for applet isn't found");
        return;
    }

    // Create declarative engine, etc
    m_widget = new Plasma::DeclarativeWidget(this);
    m_widget->setInitializationDelayed(true);
    m_widget->setQmlPath(data_path + QString::fromLatin1("contents/ui/main.qml"));

    if (!m_widget->engine() || !m_widget->engine()->rootContext() || !m_widget->engine()->rootContext()->isValid()
            || m_widget->mainComponent()->isError()) {
        QString reason;
        foreach (QDeclarativeError error, m_widget->mainComponent()->errors()) {
            reason += error.toString();
        }
        setFailedToLaunch(true, reason);
        return;
    }

    // setup context add global object "plasmoid"
    QDeclarativeEngine *engine = m_widget->engine();
    engine->rootContext()->setContextProperty("plasmoid", m_plasmoid);

    // add enumerations manually to global context
    _RegisterEnums(engine->rootContext(), Plasmoid::staticMetaObject);

    // add declarative widget to our applet
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addItem(m_widget);
}

bool Applet::isFirstRun()
{
    return m_firstRun;
}


void Applet::configChanged()
{
    KConfigGroup gcg = globalConfig();
    KConfigGroup cg = config();

    const QStringList hiddenTypes = cg.readEntry("hidden", QStringList());
    const QStringList alwaysShownTypes = cg.readEntry("alwaysShown", QStringList());

    m_shownCategories.clear();

    if (cg.readEntry("ShowApplicationStatus", gcg.readEntry("ShowApplicationStatus", DEFAULT_SHOW_APPS))) {
        m_shownCategories.insert(Task::ApplicationStatus);
    }

    if (cg.readEntry("ShowCommunications", gcg.readEntry("ShowCommunications", DEFAULT_SHOW_COMMUNICATION))) {
        m_shownCategories.insert(Task::Communications);
    }

    if (cg.readEntry("ShowSystemServices", gcg.readEntry("ShowSystemServices", DEFAULT_SHOW_SERVICES))) {
        m_shownCategories.insert(Task::SystemServices);
    }

    if (cg.readEntry("ShowHardware", gcg.readEntry("ShowHardware", DEFAULT_SHOW_HARDWARE))) {
        m_shownCategories.insert(Task::Hardware);
    }

    if (cg.readEntry("ShowUnknown", gcg.readEntry("ShowUnknown", DEFAULT_SHOW_UNKNOWN))) {
        m_shownCategories.insert(Task::UnknownCategory);
    }

    s_manager->loadApplets(this);
}

void Applet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        m_plasmoid->setFormFactor(Plasmoid::ToFormFactor(formFactor()));
    }

    if (constraints & Plasma::LocationConstraint) {
        m_plasmoid->setLocation(Plasmoid::ToLocation(location()));
    }

    if (constraints & Plasma::SizeConstraint) {
    }

    if (constraints & Plasma::ImmutableConstraint) {
        if (m_visibleItemsInterface) {
            bool visible = (immutability() == Plasma::UserImmutable);
            m_visibleItemsUi.visibleItemsView->setEnabled(immutability() == Plasma::Mutable);
            m_visibleItemsUi.unlockLabel->setVisible(visible);
            m_visibleItemsUi.unlockButton->setVisible(visible);
        }
    }

    if (constraints & Plasma::StartupCompletedConstraint) {
        QTimer::singleShot(0, this, SLOT(checkDefaultApplets()));
        configChanged();
    }

    s_manager->forwardConstraintsEvent(constraints, this);
}

SystemTray::Manager *Applet::manager() const
{
    return s_manager;
}

QSet<Task::Category> Applet::shownCategories() const
{
    return m_shownCategories;
}

void Applet::themeChanged()
{
    update();
}


void Applet::propogateSizeHintChange(Qt::SizeHint which)
{
    emit sizeHintChanged(which);
}

void Applet::createConfigurationInterface(KConfigDialog *parent)
{
    if (!m_autoHideInterface) {
        m_autoHideInterface = new QWidget();
        m_visibleItemsInterface = new QWidget();

        m_autoHideUi.setupUi(m_autoHideInterface.data());
        m_autoHideUi.icons->header()->setResizeMode(QHeaderView::ResizeToContents);

        m_visibleItemsUi.setupUi(m_visibleItemsInterface.data());

        QAction *unlockAction = 0;
        if (containment() && containment()->corona()) {
            unlockAction = containment()->corona()->action("lock widgets");
        }

        if (unlockAction) {
            disconnect(m_visibleItemsUi.unlockButton, SIGNAL(clicked()), this, SLOT(unlockContainment()));
            connect(m_visibleItemsUi.unlockButton, SIGNAL(clicked()), unlockAction, SLOT(trigger()), Qt::UniqueConnection);
        } else {
            disconnect(m_visibleItemsUi.unlockButton, SIGNAL(clicked()), unlockAction, SLOT(trigger()));
            connect(m_visibleItemsUi.unlockButton, SIGNAL(clicked()), this, SLOT(unlockContainment()), Qt::UniqueConnection);
        }


        connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        parent->addPage(m_visibleItemsInterface.data(), i18n("Display"),
                        "preferences-desktop-notification",
                        i18n("Choose which information to show"));
        parent->addPage(m_autoHideInterface.data(), i18n("Entries"), "configure-toolbars");

        bool visible = (immutability() == Plasma::UserImmutable);
        //FIXME: always showing the scrollbar is due to a bug somewhere in QAbstractScrollArea,
        //QListView and/or KCategorizedView; without it, under certain circumstances it will
        //go into an infinite loop. too many people are running into this problem, so we are
        //working around the problem rather than waiting for an upstream fix, which is against
        //our usual policy.
        //to determine if this line is no longer needed in the future, comment it out, lock
        //widgets, then call up the configuration dialog for a system tray applet and click
        //on the "unlock widgets" button.
        m_visibleItemsUi.visibleItemsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        m_visibleItemsUi.visibleItemsView->setEnabled(immutability() == Plasma::Mutable);
        m_visibleItemsUi.unlockLabel->setVisible(visible);
        m_visibleItemsUi.unlockButton->setVisible(visible);

        m_visibleItemsUi.visibleItemsView->setCategoryDrawer(new KCategoryDrawerV3(m_visibleItemsUi.visibleItemsView));
        m_visibleItemsUi.visibleItemsView->setMouseTracking(true);
        m_visibleItemsUi.visibleItemsView->setVerticalScrollMode(QListView::ScrollPerPixel);

        KCategorizedSortFilterProxyModel *visibleItemsModel = new KCategorizedSortFilterProxyModel(m_visibleItemsUi.visibleItemsView);
        visibleItemsModel->setCategorizedModel(true);

        m_visibleItemsSourceModel = new QStandardItemModel(m_visibleItemsUi.visibleItemsView);
        visibleItemsModel->setSourceModel(m_visibleItemsSourceModel.data());

        m_visibleItemsUi.visibleItemsView->setModel(visibleItemsModel);
    }

    m_autoHideUi.icons->clear();
    if (m_visibleItemsSourceModel) {
        m_visibleItemsSourceModel.data()->clear();
    }

    QMultiMap<QString, Task *> sortedTasks;
    foreach (Task *task, s_manager->tasks()) {
        if (!m_shownCategories.contains(task->category())) {
            continue;
        }

        if (!task->widget(this, false)) {
            // it is not being used by this widget
            continue;
        }

        sortedTasks.insert(task->name(), task);
    }

    KConfigGroup gcg = globalConfig();
    KConfigGroup cg = config();
    KConfigGroup shortcutsConfig = KConfigGroup(&cg, "Shortcuts");

    foreach (Task *task, sortedTasks) {
        QTreeWidgetItem *listItem = new QTreeWidgetItem(m_autoHideUi.icons);
        KComboBox *itemCombo = new KComboBox(m_autoHideUi.icons);
        listItem->setText(0, task->name());
        listItem->setIcon(0, task->icon());
        listItem->setFlags(Qt::ItemIsEnabled);
        listItem->setData(0, Qt::UserRole, task->typeId());

        itemCombo->addItem(i18nc("Item will be automatically shown or hidden from the systray", "Auto"));
        itemCombo->addItem(i18nc("Item is never visible in the systray", "Hidden"));
        itemCombo->addItem(i18nc("Item is always visible in the systray", "Always Visible"));

        if (task->hidden() & Task::UserHidden) {
            itemCombo->setCurrentIndex(1);
//        } else if (m_taskArea->alwaysShownTypes().contains(task->typeId())) {
//            itemCombo->setCurrentIndex(2);
        } else {
            itemCombo->setCurrentIndex(0);
        }
        m_autoHideUi.icons->setItemWidget(listItem, 1, itemCombo);

        KKeySequenceWidget *button = new KKeySequenceWidget(m_autoHideUi.icons);

        Plasma::IconWidget *icon = qobject_cast<Plasma::IconWidget *>(task->widget(this));
        Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(task->widget(this));

        if (task && icon) {
            QString shortcutText = shortcutsConfig.readEntryUntranslated(icon->action()->objectName(), QString());
            button->setKeySequence(shortcutText);
        } else if (task && applet) {
            button->setKeySequence(applet->globalShortcut().primary());
        //no way to have a shortcut for the fdo protocol
        } else {
            button->setEnabled(false);
        }
        m_autoHideUi.icons->setItemWidget(listItem, 2, button);
        m_autoHideUi.icons->addTopLevelItem(listItem);

        // try to make sure we have enough width!
        int totalWidth = 0;
        for (int i = 0; i < m_autoHideUi.icons->header()->count(); ++i) {
            totalWidth += m_autoHideUi.icons->columnWidth(i);
        }
        m_autoHideUi.icons->setMinimumWidth(totalWidth + style()->pixelMetric(QStyle::PM_ScrollBarExtent));

        connect(itemCombo, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
        connect(button, SIGNAL(keySequenceChanged(QKeySequence)), parent, SLOT(settingsModified()));
    }

    const QString itemCategories = i18nc("Categories of items in the systemtray that will be shown or hidden", "Shown Item Categories");

    QStandardItem *applicationStatusItem = new QStandardItem();
    applicationStatusItem->setText(i18nc("Systemtray items that describe the status of a generic application", "Application status"));
    applicationStatusItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    bool checked = cg.readEntry("ShowApplicationStatus",
                                gcg.readEntry("ShowApplicationStatus", DEFAULT_SHOW_APPS));
    applicationStatusItem->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    applicationStatusItem->setData(itemCategories, KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    applicationStatusItem->setData("ShowApplicationStatus", Qt::UserRole+1);
    m_visibleItemsSourceModel.data()->appendRow(applicationStatusItem);

    QStandardItem *communicationsItem = new QStandardItem();
    communicationsItem->setText(i18nc("Items communication related, such as chat or email clients", "Communications"));
    communicationsItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    checked = cg.readEntry("ShowCommunications",
                           gcg.readEntry("ShowCommunications", DEFAULT_SHOW_COMMUNICATION));
    communicationsItem->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    communicationsItem->setData(itemCategories, KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    communicationsItem->setData("ShowCommunications", Qt::UserRole+1);
    m_visibleItemsSourceModel.data()->appendRow(communicationsItem);

    QStandardItem *systemServicesItem = new QStandardItem();
    systemServicesItem->setText(i18nc("Items about the status of the system, such as a filesystem indexer", "System services"));
    systemServicesItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    checked = cg.readEntry("ShowSystemServices",
                           gcg.readEntry("ShowSystemServices", DEFAULT_SHOW_SERVICES));
    systemServicesItem->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    systemServicesItem->setData(itemCategories, KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    systemServicesItem->setData("ShowSystemServices", Qt::UserRole+1);
    m_visibleItemsSourceModel.data()->appendRow(systemServicesItem);

    QStandardItem *hardwareControlItem = new QStandardItem();
    hardwareControlItem->setText(i18nc("Items about hardware, such as battery or volume control", "Hardware control"));
    hardwareControlItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    checked = cg.readEntry("ShowHardware",
                           gcg.readEntry("ShowHardware", DEFAULT_SHOW_HARDWARE));
    hardwareControlItem->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    hardwareControlItem->setData(itemCategories, KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    hardwareControlItem->setData("ShowHardware", Qt::UserRole+1);
    m_visibleItemsSourceModel.data()->appendRow(hardwareControlItem);

    QStandardItem *unknownItem = new QStandardItem();
    unknownItem->setText(i18nc("Other uncategorized systemtray items", "Miscellaneous"));
    unknownItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    checked = cg.readEntry("ShowUnknown",
                           gcg.readEntry("ShowUnknown", DEFAULT_SHOW_UNKNOWN));
    unknownItem->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    unknownItem->setData(itemCategories, KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    unknownItem->setData("ShowUnknown", Qt::UserRole+1);
    m_visibleItemsSourceModel.data()->appendRow(unknownItem);

    QStringList ownApplets = s_manager->applets(this);

    foreach (const KPluginInfo &info, Plasma::Applet::listAppletInfo()) {
        KService::Ptr service = info.service();
        if (service->property("X-Plasma-NotificationArea", QVariant::Bool).toBool()) {
            QStandardItem *item = new QStandardItem();
            item->setText(service->name());
            item->setIcon(KIcon(service->icon()));
            item->setCheckable(true);
            item->setCheckState(ownApplets.contains(info.pluginName()) ? Qt::Checked : Qt::Unchecked);
            item->setData(i18nc("Extra items to be manually added in the systray, such as little Plasma widgets", "Extra Items"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
            item->setData(info.pluginName(), Qt::UserRole+2);
            m_visibleItemsSourceModel.data()->appendRow(item);
        }
    }

    connect(m_visibleItemsSourceModel.data(), SIGNAL(itemChanged(QStandardItem*)), parent, SLOT(settingsModified()));
}

//not always the corona lock action is available: netbook locks per-containment
void Applet::unlockContainment()
{
    if (containment() && containment()->immutability() == Plasma::UserImmutable) {
        containment()->setImmutability(Plasma::Mutable);
    }
}

void Applet::configAccepted()
{
    KConfigGroup cg = config();
    KConfigGroup shortcutsConfig = KConfigGroup(&cg, "Shortcuts");

    QStringList hiddenTypes;
    QStringList alwaysShownTypes;
    QTreeWidget *hiddenList = m_autoHideUi.icons;
    for (int i = 0; i < hiddenList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = hiddenList->topLevelItem(i);
        KComboBox *itemCombo = static_cast<KComboBox *>(hiddenList->itemWidget(item, 1));
        //kDebug() << (item->checkState() == Qt::Checked) << item->data(Qt::UserRole).toString();
        const QString taskTypeId = item->data(0, Qt::UserRole).toString();
        if (itemCombo->currentIndex() == 1) {
            //Always hidden
            hiddenTypes << taskTypeId;
        } else if (itemCombo->currentIndex() == 2) {
            //Always visible
            alwaysShownTypes << taskTypeId;
        }

        KKeySequenceWidget *keySeq = static_cast<KKeySequenceWidget *>(hiddenList->itemWidget(item, 2));
        QKeySequence seq = keySeq->keySequence();
        Task *task = 0;
        //FIXME: terribly inefficient
        foreach (Task *candidateTask, s_manager->tasks()) {
            if (candidateTask->typeId() == taskTypeId) {
                task = candidateTask;
                break;
            }
        }

        if (task) {
            QGraphicsWidget *widget = task->widget(this);

            if (widget) {
                Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);
                Plasma::IconWidget *icon = qobject_cast<Plasma::IconWidget *>(widget);
                if (applet) {
                    applet->setGlobalShortcut(KShortcut(seq));
                } else if (icon) {
                    KAction *action = qobject_cast<KAction *>(icon->action());
                    if (action && !seq.isEmpty()) {
                        action->setGlobalShortcut(KShortcut(seq),
                            KAction::ShortcutTypes(KAction::ActiveShortcut | KAction::DefaultShortcut),
                            KAction::NoAutoloading);
                        shortcutsConfig.writeEntry(action->objectName(), seq.toString());
                    } else if (seq.isEmpty()) {
                        action->forgetGlobalShortcut();
                        shortcutsConfig.deleteEntry(action->objectName());
                    }
                }
            }
        }
    }

    cg.writeEntry("hidden", hiddenTypes);
    cg.writeEntry("alwaysShown", alwaysShownTypes);

    QStringList applets = s_manager->applets(this);

    for (int i = 0; i <= m_visibleItemsSourceModel.data()->rowCount() - 1; i++) {
        QModelIndex index = m_visibleItemsSourceModel.data()->index(i, 0);
        QString itemCategory = index.data(Qt::UserRole+1).toString();
        QString appletName = index.data(Qt::UserRole+2).toString();
        if (!itemCategory.isEmpty()) {
            QStandardItem *item = m_visibleItemsSourceModel.data()->itemFromIndex(index);
            cg.writeEntry(itemCategory, (item->checkState() == Qt::Checked));
        } else if (!appletName.isEmpty()){
            QStandardItem *item = m_visibleItemsSourceModel.data()->itemFromIndex(index);

            if (item->checkState() != Qt::Unchecked && !applets.contains(appletName)) {
                s_manager->addApplet(appletName, this);
            }

            if (item->checkState() == Qt::Checked) {
                applets.removeAll(appletName);
            }
        }
    }

    foreach (const QString &appletName, applets) {
        s_manager->removeApplet(appletName, this);
    }

    emit configNeedsSaving();
}

void Applet::checkDefaultApplets()
{
    if (config().readEntry("DefaultAppletsAdded", false)) {
        m_firstRun = false;
        return;
    }


    QStringList applets = s_manager->applets(this);
    if (!applets.contains("org.kde.networkmanagement")) {
        s_manager->addApplet("org.kde.networkmanagement", this);
    }

    if (!applets.contains("notifier")) {
        s_manager->addApplet("notifier", this);
    }

    if (!applets.contains("notifications")) {
        s_manager->addApplet("notifications", this);
    }

    if (!applets.contains("battery")) {
        Plasma::DataEngineManager *engines = Plasma::DataEngineManager::self();
        Plasma::DataEngine *power = engines->loadEngine("powermanagement");
        if (power) {
            const QStringList &batteries = power->query("Battery")["Sources"].toStringList();
            if (!batteries.isEmpty()) {
                s_manager->addApplet("battery", this);
            }
        }
        engines->unloadEngine("powermanagement");
    }

    config().writeEntry("DefaultAppletsAdded", true);
}

}

#include "applet.moc"
