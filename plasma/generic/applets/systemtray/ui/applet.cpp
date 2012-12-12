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
#include "widgetitem.h"
#include "mouseredirectarea.h"

#include "../protocols/dbussystemtray/dbussystemtraytask.h"

#include <QtCore/QTimer>
#include <QtGui/QMenu>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QStandardItemModel>
#include <QtDeclarative/QDeclarativeError>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeComponent>

#include <KDE/KStandardDirs>
#include <KDE/KAction>
#include <KDE/KWindowSystem>
#include <KConfigDialog>
#include <KComboBox>
#include <KCategorizedSortFilterProxyModel>
#include <KCategoryDrawer>
#include <KKeySequenceWidget>

#include <plasma/dataenginemanager.h>
#include <Plasma/Corona>
#include <Plasma/IconWidget>
#include <KDE/Plasma/DeclarativeWidget>

#include <inttypes.h>

#include "config.h"

#include "../core/manager.h"

static const bool DEFAULT_SHOW_APPS = true;
static const bool DEFAULT_SHOW_COMMUNICATION = true;
static const bool DEFAULT_SHOW_SERVICES = true;
static const bool DEFAULT_SHOW_HARDWARE = true;
static const bool DEFAULT_SHOW_UNKNOWN = true;
static const char KlipperName[] = "Klipper";

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
      m_widget(0),
      m_firstRun(true)
{
    if (!s_manager) {
        s_manager = new SystemTray::Manager();
    }

    ++s_managerUsage;

    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(true);
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
        if (task->isWidget())
            delete task->widget(this, false);
    }

    delete m_widget;

    --s_managerUsage;
    if (s_managerUsage < 1) {
        delete s_manager;
        s_manager = 0;
        s_managerUsage = 0;
    }
}

void Applet::init()
{
    // First of all, we have to register new QML types because they won't be registered later
    qmlRegisterType<WidgetItem>("Private", 0, 1, "WidgetItem");
    qmlRegisterType<MouseRedirectArea>("Private", 0, 1, "MouseRedirectArea");

    // Find data directory
    KStandardDirs std_dirs;
    QStringList dirs = std_dirs.findDirs("data", SYSTEMTRAY_DATA_INSTALL_DIR);
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
    connect(m_widget, SIGNAL(finished()), this, SLOT(_onWidgetCreationFinished()));
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
    QDeclarativeContext *root_context = m_widget->engine()->rootContext();
    root_context->setContextProperty("plasmoid", this);

    // add enumerations manually to global context
    _RegisterEnums(root_context, Task::staticMetaObject);
    _RegisterEnums(root_context, SystemTray::Applet::staticMetaObject);

    // add declarative widget to our applet
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addItem(m_widget);
}


void Applet::_onAddedTask(Task *task)
{
    if (task->isWidget()) {
        // If task is presented as a widget then we should check that widget
        if (!task->isEmbeddable(this)) {
            //was a widget created previously? kill it
            QGraphicsWidget *widget = task->widget(this, false);
            if (widget) {
                task->abandon(this);
            }
            return;
        }

        QGraphicsWidget *widget = task->widget(this);
        if (!widget) {
            return;
        }

        //If the applet doesn't want to show FDO tasks, remove (not just hide) any of them
        //if the dbus icon has a category that the applet doesn't want to show remove it
        if (!m_shownCategories.contains(task->category()) && !qobject_cast<Plasma::Applet *>(widget)) {
            task->abandon(this);
            return;
        }
    } else if (!m_shownCategories.contains(task->category())) {
        return;
    }

    // provide task to qml code
    emit newTask(task);

    DBusSystemTrayTask *dbus_task = qobject_cast<DBusSystemTrayTask*>(task);
    if (dbus_task && !dbus_task->objectName().isEmpty() && dbus_task->shortcut().isEmpty()) {
        // try to set shortcut
        bool is_klipper = false;
        QString default_shortcut;
        if (dbus_task->name() == KlipperName) {
            // for klipper we have to read its default hotkey from its config
            is_klipper = true;
            QString file = KStandardDirs::locateLocal("config", "kglobalshortcutsrc");
            KConfig config(file);
            KConfigGroup cg(&config, "klipper");
            QStringList shortcutTextList = cg.readEntry("show_klipper_popup", QStringList());

            if (shortcutTextList.size() >= 2) {
                default_shortcut = shortcutTextList.first();
                if (default_shortcut.isEmpty()) {
                    default_shortcut = shortcutTextList[1];
                }
            }
            if (default_shortcut.isEmpty()) {
                default_shortcut = "Ctrl+Alt+V";
            }
        }

        QString action_name = _getActionName(task);
        KConfigGroup cg = config();
        KConfigGroup shortcutsConfig = KConfigGroup(&cg, "Shortcuts");
        QString shortcut = shortcutsConfig.readEntryUntranslated(action_name, default_shortcut);
        dbus_task->setShortcut(shortcut);

        if (is_klipper && shortcut == default_shortcut) {
            // we have to write klipper's hotkey to config
            if (shortcut.isEmpty())
                shortcutsConfig.deleteEntry(action_name);
            else
                shortcutsConfig.writeEntry(action_name, shortcut);
        }
    }
}


void Applet::_onRemovedTask(Task *task)
{
    //remove task from QML code
    emit deletedTask(task);
}

void Applet::_onStatusChangedTask()
{
    foreach (Task *task, s_manager->tasks()) { 
        if (task->status() == Task::NeedsAttention)  { 
            setStatus(Plasma::NeedsAttentionStatus);
            return; 
        }
    }
    
    setStatus(Plasma::PassiveStatus);
}

void Applet::_onWidgetCreationFinished()
{
    // add already existing tasks
    QList<Task*> tasks = s_manager->tasks();
    foreach (Task *t, tasks) {
        _onAddedTask(t);
    }

    connect(s_manager, SIGNAL(taskAdded(SystemTray::Task*)),   this, SLOT(_onAddedTask(SystemTray::Task*)));
    connect(s_manager, SIGNAL(taskRemoved(SystemTray::Task*)), this, SLOT(_onRemovedTask(SystemTray::Task*)));
    connect(s_manager, SIGNAL(taskStatusChanged()), this, SLOT(_onStatusChangedTask()));
}


bool Applet::isFirstRun()
{
    return m_firstRun;
}


void Applet::configChanged()
{
    KConfigGroup gcg = globalConfig();
    KConfigGroup cg = config();

    m_hiddenTypes = QSet<QString>::fromList(cg.readEntry("hidden", QStringList()));
    m_alwaysShownTypes = QSet<QString>::fromList(cg.readEntry("alwaysShown", QStringList()));

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

    // notify QML code about new user's preferences
    emit visibilityPreferenceChanged();
}


QString Applet::_getActionName(Task *task) const {
    if (task->objectName().isEmpty())
        return QString("");
    return task->objectName() + QString("-") + QString::number(this->id());
}

void Applet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        emit formFactorChanged();
    }

    if (constraints & Plasma::LocationConstraint) {
        emit locationChanged();
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

        if (task->isWidget() && !task->widget(this, false)) {
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

        if (m_hiddenTypes.contains(task->typeId())) {
            itemCombo->setCurrentIndex(1);
        } else if (m_alwaysShownTypes.contains(task->typeId())) {
            itemCombo->setCurrentIndex(2);
        } else {
            itemCombo->setCurrentIndex(0);
        }
        m_autoHideUi.icons->setItemWidget(listItem, 1, itemCombo);

        KKeySequenceWidget *button = new KKeySequenceWidget(m_autoHideUi.icons);

        DBusSystemTrayTask *dbus_task = qobject_cast<DBusSystemTrayTask*>(task);
        Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(task->widget(this));

        if (task && dbus_task && !dbus_task->objectName().isEmpty()) {
            QString action_name = _getActionName(task);
            QString shortcutText = shortcutsConfig.readEntryUntranslated(action_name, QString());
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
            if (!task->isWidget()) {
                DBusSystemTrayTask *dbus_task = qobject_cast<DBusSystemTrayTask*>(task);
                if (dbus_task) {
                    QString shortcut = seq.toString();
                    dbus_task->setShortcut(shortcut);
                    QString action_name = _getActionName(task);
                    if (seq.isEmpty())
                        shortcutsConfig.deleteEntry(action_name);
                    else
                        shortcutsConfig.writeEntry(action_name, shortcut);
                    dbus_task->setShortcut(shortcut);
                }
            } else {
                Plasma::Applet *applet = qobject_cast<Plasma::Applet *>( task->widget(this) );
                if (applet) {
                    applet->setGlobalShortcut(KShortcut(seq));
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

    if (!applets.contains("org.kde.notifications")) {
        s_manager->addApplet("org.kde.notifications", this);
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


int Applet::getVisibilityPreference(QObject *task) const
{
    Task *t = qobject_cast<Task*>(task);
    if (!t)
        return AutoVisibility;
    if ( m_hiddenTypes.contains(t->typeId()) ) {
        return AlwaysHidden;
    } else if ( m_alwaysShownTypes.contains(t->typeId()) ) {
        return AlwaysShown;
    }
    return AutoVisibility;
}

QAction *Applet::createShortcutAction(QString action_id) const
{
    KAction *action = new KAction(parent());
    action->setObjectName(action_id);
    return action;
}

void Applet::updateShortcutAction(QAction *action, QString shortcut) const
{
    KAction *act = qobject_cast<KAction*>(action);
    if (!act) {
        return;
    }

    act->forgetGlobalShortcut();
    if (!shortcut.isEmpty()) {
        act->setGlobalShortcut(KShortcut(QKeySequence(shortcut)),
                               KAction::ShortcutTypes(KAction::ActiveShortcut | KAction::DefaultShortcut),
                               KAction::NoAutoloading);
    }
}

void Applet::destroyShortcutAction(QAction *action) const
{
    KAction *act = qobject_cast<KAction*>(action);
    if (act) {
        delete act;
    }
}

void Applet::showMenu(QObject *menu_var, int x, int y, QObject *item_var) const
{
    QGraphicsItem *item = qobject_cast<QGraphicsItem *>(item_var);
    QMenu *menu = qobject_cast<QMenu *>(menu_var);
    if (menu) {
        QPoint pos(x, y);
        menu->adjustSize();
        if (item && containment() && containment()->corona()) {
            pos = containment()->corona()->popupPosition(item, menu->size());
        } else {
            pos = Plasma::Applet::popupPosition(menu->size());
        }
        menu->popup(pos);
    }
}

void Applet::hideFromTaskbar(qulonglong win_id) const
{
    if (win_id > 0) {
        KWindowSystem::setState(win_id, NET::SkipTaskbar | NET::SkipPager);
    }
}

QString Applet::getUniqueId(QObject *obj) const
{
    return QString::number(reinterpret_cast<uintmax_t>(obj));
}

QPoint Applet::popupPosition(QObject *item_var, QSize size, int align) const
{
    QGraphicsItem *item = qobject_cast<QGraphicsItem*>(item_var);
    if ( item && containment() && containment()->corona() ) {
        return containment()->corona()->popupPosition(item, size, (Qt::AlignmentFlag)align);
    }
    return Plasma::Applet::popupPosition(size, (Qt::AlignmentFlag)align);
}


}

#include "applet.moc"
