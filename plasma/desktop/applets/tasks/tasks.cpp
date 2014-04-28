/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright (C) 2012-2013 by Eike Hein <hein@kde.org>                   *
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

#include "tasks.h"
#include "support/draghelper.h"
#include "support/textlabel.h"
#include "support/tooltip.h"

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/DeclarativeWidget>
#include <Plasma/Package>
#include <Plasma/WindowEffects>

#include <KAuthorized>
#include <KConfigDialog>

#include <taskmanager/groupmanager.h>
#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/task.h>
#include <taskmanager/taskactions.h>
#include <taskmanager/taskgroup.h>
#include <taskmanager/taskitem.h>
#include <taskmanager/tasksmodel.h>

#include <QtDeclarative>

class GroupManager : public TaskManager::GroupManager
{
    public:
        GroupManager(Plasma::Applet *applet)
            : TaskManager::GroupManager(applet),
            m_applet(applet)
        {
        }

    protected:
        KConfigGroup config() const
        {
            return m_applet->config();
        }

    private:
        Plasma::Applet *m_applet;
};

Tasks::Tasks(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
    m_groupManager(0),
    m_declarativeWidget(0),
    m_highlightWindows(false),
    m_lastViewId(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(true);
}

Tasks::~Tasks()
{
}

void Tasks::init()
{
    m_groupManager = new GroupManager(this);
    connect(m_groupManager, SIGNAL(configChanged()), this, SIGNAL(configNeedsSaving()));

    Plasma::Containment *c = containment();

    if (c) {
        m_groupManager->setScreen(c->screen());
    }

    m_tasksModel = new TaskManager::TasksModel(m_groupManager, this);

    m_declarativeWidget = new Plasma::DeclarativeWidget(this);
    QDeclarativeContext* rootContext = m_declarativeWidget->engine()->rootContext();

    qmlRegisterType<TextLabel>( "Tasks", 0, 1, "TextLabel" );
    qmlRegisterType<ToolTipProxy>( "Tasks", 0, 1, "ToolTip" );
    rootContext->setContextProperty("tasksModel", QVariant::fromValue(static_cast<QObject *>(m_tasksModel)));
    rootContext->setContextProperty("dragHelper", QVariant::fromValue(static_cast<QObject *>(new DragHelper(this))));

    // NOTE: This can go away once Plasma::Location becomes available (i.e. once this is
    // a pure-QML applet.)
    rootContext->setContextProperty("LeftEdge", Plasma::LeftEdge);
    rootContext->setContextProperty("TopEdge", Plasma::TopEdge);
    rootContext->setContextProperty("RightEdge", Plasma::RightEdge);
    rootContext->setContextProperty("BottomEdge", Plasma::BottomEdge);

    Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
    Plasma::Package *package = new Plasma::Package(QString(), "org.kde.plasma.tasks", structure);
    m_declarativeWidget->setQmlPath(package->filePath("mainscript"));
    delete package;

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addItem(m_declarativeWidget);

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMaximumSize(INT_MAX, INT_MAX);

    QDeclarativeProperty preferredWidth(m_declarativeWidget->rootObject(), "preferredWidth");
    preferredWidth.connectNotifySignal(this, SLOT(changeSizeHint()));

    QDeclarativeProperty preferredHeight(m_declarativeWidget->rootObject(), "preferredHeight");
    preferredHeight.connectNotifySignal(this, SLOT(changeSizeHint()));

    QDeclarativeProperty optimumCapacity(m_declarativeWidget->rootObject(), "optimumCapacity");
    optimumCapacity.connectNotifySignal(this, SLOT(optimumCapacityChanged()));

    connect(m_declarativeWidget->rootObject(), SIGNAL(activateItem(int,bool)), this, SLOT(activateItem(int,bool)));
    connect(m_declarativeWidget->rootObject(), SIGNAL(itemContextMenu(int)), this, SLOT(itemContextMenu(int)), Qt::QueuedConnection);
    connect(m_declarativeWidget->rootObject(), SIGNAL(itemMove(int,int)), this, SLOT(itemMove(int,int)));
    connect(m_declarativeWidget->rootObject(), SIGNAL(itemGeometryChanged(int,int,int,int,int)),
        this, SLOT(itemGeometryChanged(int,int,int,int,int)));
    connect(m_declarativeWidget->rootObject(), SIGNAL(itemNeedsAttention(bool)), this, SLOT(itemNeedsAttention(bool)));
    connect(m_declarativeWidget->rootObject(), SIGNAL(presentWindows(int)), this, SLOT(presentWindows(int)));

    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(handleActiveWindowChanged(WId)));

    configChanged();
}

void Tasks::changeSizeHint()
{
    emit sizeHintChanged(Qt::PreferredSize);
}

QSizeF Tasks::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which == Qt::PreferredSize && m_declarativeWidget && m_declarativeWidget->rootObject()) {
        return QSizeF(m_declarativeWidget->rootObject()->property("preferredWidth").toReal(),
                      m_declarativeWidget->rootObject()->property("preferredHeight").toReal());
    } else {
        return Plasma::Applet::sizeHint(which, constraint);
    }
}

void Tasks::constraintsEvent(Plasma::Constraints constraints)
{
    if (m_groupManager && (constraints & Plasma::ScreenConstraint)) {
        Plasma::Containment *c = containment();
        if (c) {
            m_groupManager->setScreen(c->screen());
        }
    }

    if (constraints & Plasma::FormFactorConstraint) {
        m_declarativeWidget->rootObject()->setProperty("horizontal", formFactor() == Plasma::Horizontal);
        m_declarativeWidget->rootObject()->setProperty("vertical", formFactor() == Plasma::Vertical);
    }

    if (constraints & Plasma::LocationConstraint) {
        m_declarativeWidget->rootObject()->setProperty("location", location());
    }

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
}

void Tasks::optimumCapacityChanged()
{
    m_groupManager->setFullLimit(m_declarativeWidget->rootObject()->property("optimumCapacity").toInt() + 1);
}

void Tasks::activateItem(int id, bool toggle)
{
    TaskManager::AbstractGroupableItem* item = m_groupManager->rootGroup()->getMemberById(id);

    if (!item) {
        return;
    }

    if (item->itemType() == TaskManager::TaskItemType && !item->isStartupItem()) {
        TaskManager::TaskItem* taskItem = static_cast<TaskManager::TaskItem*>(item);

        if (toggle) {
            taskItem->task()->activateRaiseOrIconify();
        } else {
            taskItem->task()->activate();
        }
    } else if (item->itemType() == TaskManager::LauncherItemType) {
        static_cast<TaskManager::LauncherItem*>(item)->launch();
    }
}

void Tasks::itemContextMenu(int id)
{
    TaskManager::AbstractGroupableItem* item = m_groupManager->rootGroup()->getMemberById(id);

    QDeclarativeItem* declItem = 0;
    QList<QDeclarativeItem*> declItems = m_declarativeWidget->rootObject()->findChildren<QDeclarativeItem*>();

    foreach(QDeclarativeItem* obj, declItems) {
        if (obj->property("itemId").toInt() == id) {
            declItem = obj;
            break;
        }
    }

    if (!KAuthorized::authorizeKAction("kwin_rmb") || !item || !declItem) {
        return;
    }

    QList <QAction*> actionList;

    QAction *configAction = action("configure");
    if (configAction && configAction->isEnabled()) {
        actionList.append(configAction);
    }

    TaskManager::BasicMenu* menu = 0;

    Q_ASSERT(containment());
    Q_ASSERT(containment()->corona());

    if (item->itemType() == TaskManager::TaskItemType && !item->isStartupItem()) {
        TaskManager::TaskItem* taskItem = static_cast<TaskManager::TaskItem*>(item);
/* FIXME (Un)collapse support is pending merge.
        QAction *a(0);
        if (taskItem->isGrouped()) {
            a = new QAction(i18n("Collapse Parent Group"), 0);
            connect(a, SIGNAL(triggered()), taskItem->parentGroup(), SLOT(collapse()));
            actionList.prepend(a);
        }
*/
        menu = new TaskManager::BasicMenu(0, taskItem, m_groupManager, actionList);
    } else if (item->itemType() == TaskManager::GroupItemType) {
        TaskManager::TaskGroup* taskGroup = static_cast<TaskManager::TaskGroup*>(item);
/* FIXME (Un)collapse support is pending merge.
        QAction *a;
        if (true) {
            a = new QAction(i18n("Collapse Group"), this);
            connect(a, SIGNAL(triggered()), taskGroup, SLOT(collapse()));
        } else {
            a = new QAction(i18n("Expand Group"), this);
            connect(a, SIGNAL(triggered()), taskGroup, SLOT(expand()));
        }
        actionList.prepend(a);
*/
        const int maxWidth = 0.8 * containment()->corona()->screenGeometry(containment()->screen()).width();
        menu = new TaskManager::BasicMenu(0, taskGroup, m_groupManager, actionList, QList <QAction*>(), maxWidth);
    } else if (item->itemType() == TaskManager::LauncherItemType) {
        menu = new TaskManager::BasicMenu(0, static_cast<TaskManager::LauncherItem*>(item),
            m_groupManager, actionList);
    }

    if (!menu) {
        return;
    }

    menu->adjustSize();

    if (formFactor() != Plasma::Vertical) {
        menu->setMinimumWidth(declItem->implicitWidth());
    }

    menu->exec(containment()->corona()->popupPosition(declItem, menu->size()));
    menu->deleteLater();
}

void Tasks::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QVariant ret;

    QMetaObject::invokeMethod(m_declarativeWidget->rootObject(), "isTaskAt",
        Q_RETURN_ARG(QVariant, ret), Q_ARG(QVariant, event->pos()));

    if (!ret.toBool()) {
        Plasma::Applet::contextMenuEvent(event);
    }
}

void Tasks::itemHovered(int id, bool hovered)
{
    TaskManager::AbstractGroupableItem* item = m_groupManager->rootGroup()->getMemberById(id);

    if (!item) {
        return;
    }

    if (hovered && m_highlightWindows && view()) {
        m_lastViewId = view()->winId();
        Plasma::WindowEffects::highlightWindows(m_lastViewId, QList<WId>::fromSet(item->winIds()));
    } else if (m_highlightWindows && m_lastViewId) {
        Plasma::WindowEffects::highlightWindows(m_lastViewId, QList<WId>());
    }
}

void Tasks::itemMove(int id, int newIndex)
{
    m_groupManager->manualSortingRequest(m_groupManager->rootGroup()->getMemberById(id), newIndex);
}

void Tasks::itemGeometryChanged(int id, int x, int y, int width, int height)
{
    TaskManager:: AbstractGroupableItem *item = m_groupManager->rootGroup()->getMemberById(id);

    if (!item || item->itemType() != TaskManager::TaskItemType || !scene())
    {
        return;
    }

    TaskManager::TaskItem *taskItem = static_cast<TaskManager::TaskItem *>(item);

    if (!taskItem->task()) {
        return;
    }

    QGraphicsView *parentView = 0;
    QGraphicsView *possibleParentView = 0;
    // The following was taken from Plasma::Applet, it doesn't make sense to make the item an applet, and this was the easiest way around it.
    foreach (QGraphicsView *view, scene()->views()) {
        if (view->sceneRect().intersects(sceneBoundingRect()) ||
            view->sceneRect().contains(scenePos())) {
            if (view->isActiveWindow()) {
                parentView = view;
                break;
            } else {
                possibleParentView = view;
            }
        }
    }

    if (!parentView) {
        parentView = possibleParentView;

        if (!parentView) {
            return;
        }
    }

    QRect iconRect(x, y, width, height);
    iconRect.moveTopLeft(parentView->mapFromScene(m_declarativeWidget->mapToScene(iconRect.topLeft())));
    iconRect.moveTopLeft(parentView->mapToGlobal(iconRect.topLeft()));

    taskItem->task()->publishIconGeometry(iconRect);
}

void Tasks::itemNeedsAttention(bool needs)
{
    if (needs) {
        setStatus(Plasma::NeedsAttentionStatus);
    } else {
        foreach(TaskManager::AbstractGroupableItem *item, m_groupManager->rootGroup()->members()) {
            if (item->demandsAttention()) {
                // not time to go passive yet! :)
                return;
            }
        }

        setStatus(Plasma::PassiveStatus);
    }
}

void Tasks::presentWindows(int groupParentId)
{
    TaskManager:: AbstractGroupableItem *item = m_groupManager->rootGroup()->getMemberById(groupParentId);

    if (item) {
        Plasma::WindowEffects::presentWindows(view()->winId(), QList<WId>::fromSet(item->winIds()));
    }
}

void Tasks::handleActiveWindowChanged(WId activeWindow)
{
    m_declarativeWidget->rootObject()->setProperty("activeWindowId", qulonglong(activeWindow));
}

void Tasks::configChanged()
{
    KConfigGroup cg = config();
    bool changed = false;

    // only update these if they have actually changed, because they make the
    // group manager reload its tasks list
    const bool showOnlyCurrentDesktop = cg.readEntry("showOnlyCurrentDesktop", false);
    if (showOnlyCurrentDesktop != m_groupManager->showOnlyCurrentDesktop()) {
        m_groupManager->setShowOnlyCurrentDesktop(showOnlyCurrentDesktop);
        m_declarativeWidget->rootObject()->setProperty("showOnlyCurrentDesktop", showOnlyCurrentDesktop);
        changed = true;
    }

    const bool showOnlyCurrentActivity = cg.readEntry("showOnlyCurrentActivity", true);
    if (showOnlyCurrentActivity != m_groupManager->showOnlyCurrentActivity()) {
        m_groupManager->setShowOnlyCurrentActivity(showOnlyCurrentActivity);
        m_declarativeWidget->rootObject()->setProperty("showOnlyCurrentActivity", showOnlyCurrentActivity);
        changed = true;
    }

    const bool showOnlyCurrentScreen = cg.readEntry("showOnlyCurrentScreen", false);
    if (showOnlyCurrentScreen != m_groupManager->showOnlyCurrentScreen()) {
        m_groupManager->setShowOnlyCurrentScreen(showOnlyCurrentScreen);
        changed = true;
    }

    const bool showOnlyMinimized = cg.readEntry("showOnlyMinimized", false);
    if (showOnlyMinimized != m_groupManager->showOnlyMinimized()) {
        m_groupManager->setShowOnlyMinimized(showOnlyMinimized);
        m_declarativeWidget->rootObject()->setProperty("showOnlyMinimized", showOnlyMinimized);
        changed = true;
    }

    TaskManager::GroupManager::TaskGroupingStrategy groupingStrategy =
        static_cast<TaskManager::GroupManager::TaskGroupingStrategy>(
            cg.readEntry("groupingStrategy",
                         static_cast<int>(TaskManager::GroupManager::ProgramGrouping))
        );
    if (groupingStrategy != m_groupManager->groupingStrategy()) {
        // FIXME: Add back support for manual grouping.
        if (groupingStrategy == TaskManager::GroupManager::ManualGrouping) {
            groupingStrategy = TaskManager::GroupManager::ProgramGrouping;
        }
        m_groupManager->setGroupingStrategy(groupingStrategy);
        changed = true;
    }

    const bool onlyGroupWhenFull = cg.readEntry("groupWhenFull", true);
    if (onlyGroupWhenFull != m_groupManager->onlyGroupWhenFull()) {
        m_groupManager->setOnlyGroupWhenFull(onlyGroupWhenFull);
        changed = true;
    }

    TaskManager::GroupManager::TaskSortingStrategy sortingStrategy =
        static_cast<TaskManager::GroupManager::TaskSortingStrategy>(
            cg.readEntry("sortingStrategy",
                         static_cast<int>(TaskManager::GroupManager::AlphaSorting))
        );

    if (sortingStrategy != m_groupManager->sortingStrategy()) {
        m_groupManager->setSortingStrategy(sortingStrategy);
        m_declarativeWidget->rootObject()->setProperty("manualSorting",
            (sortingStrategy == TaskManager::GroupManager::ManualSorting));
        changed = true;
    }

    const int maxRows = cg.readEntry("maxRows", 2);
    if (maxRows != m_declarativeWidget->rootObject()->property("maxStripes").toInt()) {
        m_declarativeWidget->rootObject()->setProperty("maxStripes", maxRows);
        changed = true;
    }

    const bool forceRows = cg.readEntry("forceRows", false);
    if (forceRows != m_declarativeWidget->rootObject()->property("forceStripes").toBool()) {
        m_declarativeWidget->rootObject()->setProperty("forceStripes", forceRows);
        changed = true;
    }

    const bool showTooltip = cg.readEntry("showToolTip", true);
    if (showTooltip != m_declarativeWidget->rootObject()->property("showToolTip").toBool()) {
        m_declarativeWidget->rootObject()->setProperty("showToolTip", showTooltip);
        changed = true;
    }

    const bool highlightWindows = cg.readEntry("highlightWindows", false);
    if (highlightWindows != m_highlightWindows) {
        m_highlightWindows = highlightWindows;
        m_declarativeWidget->rootObject()->setProperty("highlightWindows", m_highlightWindows);
        if (m_highlightWindows) {
            connect(m_declarativeWidget->rootObject(), SIGNAL(itemHovered(int,bool)), this, SLOT(itemHovered(int,bool)));
        } else {
            disconnect(m_declarativeWidget->rootObject(), SIGNAL(itemHovered(int,bool)), this, SLOT(itemHovered(int,bool)));
        }
        changed = true;
    }

    m_groupManager->readLauncherConfig();

    if (changed) {
        emit settingsChanged();
    }
}

void Tasks::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget;
    m_ui.setupUi(widget);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(widget, i18n("General"), icon());

    m_ui.showTooltip->setChecked(m_declarativeWidget->rootObject()->property("showToolTip").toBool());
    m_ui.highlightWindows->setChecked(m_highlightWindows);
    m_ui.showOnlyCurrentDesktop->setChecked(m_groupManager->showOnlyCurrentDesktop());
    m_ui.showOnlyCurrentActivity->setChecked(m_groupManager->showOnlyCurrentActivity());
    m_ui.showOnlyCurrentScreen->setChecked(m_groupManager->showOnlyCurrentScreen());
    m_ui.showOnlyMinimized->setChecked(m_groupManager->showOnlyMinimized());
    m_ui.fillRows->setChecked(m_declarativeWidget->rootObject()->property("forceStripes").toBool());

    m_ui.groupingStrategy->addItem(i18n("Do Not Group"),QVariant(TaskManager::GroupManager::NoGrouping));
    // FIXME: Add back support for manual grouping.
    // m_ui.groupingStrategy->addItem(i18n("Manually"),QVariant(TaskManager::GroupManager::ManualGrouping));
    m_ui.groupingStrategy->addItem(i18n("By Program Name"),QVariant(TaskManager::GroupManager::ProgramGrouping));

    connect(m_ui.groupingStrategy, SIGNAL(currentIndexChanged(int)), this, SLOT(dialogGroupingChanged(int)));

    switch (m_groupManager->groupingStrategy()) {
        case TaskManager::GroupManager::NoGrouping:
            m_ui.groupingStrategy->setCurrentIndex(0);
            break;
/* FIXME: Add back support for manual grouping.
        case TaskManager::GroupManager::ManualGrouping:
            m_ui.groupingStrategy->setCurrentIndex(1);
            break;
*/
        case TaskManager::GroupManager::ProgramGrouping:
            m_ui.groupingStrategy->setCurrentIndex(1);
            break;
        default:
             m_ui.groupingStrategy->setCurrentIndex(-1);
    }

    m_ui.groupWhenFull->setChecked(m_groupManager->onlyGroupWhenFull());

    m_ui.sortingStrategy->addItem(i18n("Do Not Sort"),QVariant(TaskManager::GroupManager::NoSorting));
    m_ui.sortingStrategy->addItem(i18n("Manually"),QVariant(TaskManager::GroupManager::ManualSorting));
    m_ui.sortingStrategy->addItem(i18n("Alphabetically"),QVariant(TaskManager::GroupManager::AlphaSorting));
    m_ui.sortingStrategy->addItem(i18n("By Desktop"),QVariant(TaskManager::GroupManager::DesktopSorting));
    m_ui.sortingStrategy->addItem(i18n("By Activity"),QVariant(TaskManager::GroupManager::ActivitySorting));

    switch (m_groupManager->sortingStrategy()) {
        case TaskManager::GroupManager::NoSorting:
            m_ui.sortingStrategy->setCurrentIndex(0);
            break;
        case TaskManager::GroupManager::ManualSorting:
            m_ui.sortingStrategy->setCurrentIndex(1);
            break;
        case TaskManager::GroupManager::AlphaSorting:
            m_ui.sortingStrategy->setCurrentIndex(2);
            break;
        case TaskManager::GroupManager::DesktopSorting:
            m_ui.sortingStrategy->setCurrentIndex(3);
            break;
        case TaskManager::GroupManager::ActivitySorting:
            m_ui.sortingStrategy->setCurrentIndex(4);
            break;
        default:
             m_ui.sortingStrategy->setCurrentIndex(-1);
    }

    m_ui.maxRows->setValue(m_declarativeWidget->rootObject()->property("maxStripes").toInt());

    connect(m_ui.fillRows, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_ui.showTooltip, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_ui.highlightWindows, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_ui.maxRows, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(m_ui.groupingStrategy, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
    connect(m_ui.groupWhenFull, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_ui.sortingStrategy, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
    connect(m_ui.showOnlyCurrentScreen, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_ui.showOnlyCurrentDesktop, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_ui.showOnlyCurrentActivity, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(m_ui.showOnlyMinimized, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
}

void Tasks::dialogGroupingChanged(int index)
{
     m_ui.groupWhenFull->setEnabled(static_cast<TaskManager::GroupManager::TaskGroupingStrategy>(m_ui.groupingStrategy->itemData(index).toInt()) == TaskManager::GroupManager::ProgramGrouping);
}

void Tasks::configAccepted()
{
    KConfigGroup cg = config();

    cg.writeEntry("showOnlyCurrentDesktop", m_ui.showOnlyCurrentDesktop->isChecked());
    cg.writeEntry("showOnlyCurrentActivity", m_ui.showOnlyCurrentActivity->isChecked());
    cg.writeEntry("showOnlyCurrentScreen", m_ui.showOnlyCurrentScreen->isChecked());
    cg.writeEntry("showOnlyMinimized", m_ui.showOnlyMinimized->isChecked());

    cg.writeEntry("groupingStrategy", m_ui.groupingStrategy->itemData(m_ui.groupingStrategy->currentIndex()).toInt());
    cg.writeEntry("groupWhenFull", m_ui.groupWhenFull->isChecked());

    cg.writeEntry("sortingStrategy", m_ui.sortingStrategy->itemData(m_ui.sortingStrategy->currentIndex()).toInt());

    cg.writeEntry("maxRows", m_ui.maxRows->value());
    cg.writeEntry("forceRows", m_ui.fillRows->isChecked());

    cg.writeEntry("showToolTip", m_ui.showTooltip->isChecked());
    cg.writeEntry("highlightWindows", m_ui.highlightWindows->isChecked());

    emit configNeedsSaving();
}

#include "tasks.moc"
