/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
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

// Own
#include "tasks.h"
#include "windowtaskitem.h"
#include "taskgroupitem.h"
#include "ui_tasksConfig.h"

//Taskmanager
#include <taskmanager/groupmanager.h>
#include <taskmanager/taskgroup.h>
#include <taskmanager/taskitem.h>

// KDE
#include <KConfigDialog>
#include <KDebug>

// Qt
#include <QGraphicsScene>
#include <QGraphicsLinearLayout>
#include <QVariant>
#include <QBuffer>
#include <QIODevice>

// Plasma
#include <Plasma/Containment>
#include <Plasma/FrameSvg>
#include <Plasma/Theme>

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

Tasks::Tasks(QObject* parent, const QVariantList &arguments)
     : Plasma::Applet(parent, arguments),
       m_showTooltip(false),
       m_highlightWindows(false),
       m_arrows(0),
       m_taskItemBackground(0),
       m_leftMargin(0),
       m_topMargin(0),
       m_rightMargin(0),
       m_bottomMargin(0),
       m_offscreenLeftMargin(0),
       m_offscreenTopMargin(0),
       m_offscreenRightMargin(0),
       m_offscreenBottomMargin(0),
       m_rootGroupItem(0),
       m_groupManager(0),
       m_groupModifierKey(Qt::AltModifier)
{
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    m_screenTimer.setSingleShot(true);
    m_screenTimer.setInterval(300);
    resize(500, 58);

    setAcceptDrops(true);

}

Tasks::~Tasks()
{
    delete m_rootGroupItem;
    delete m_groupManager;
}

void Tasks::init()
{
    m_groupManager = new GroupManager(this);
    Plasma::Containment* appletContainment = containment();
    if (appletContainment) {
        m_groupManager->setScreen(appletContainment->screen());
    }

    connect(m_groupManager, SIGNAL(reload()), this, SLOT(reload()));
    connect(m_groupManager, SIGNAL(configChanged()), this, SIGNAL(configNeedsSaving()));

    m_rootGroupItem = new TaskGroupItem(this, this);
    m_rootGroupItem->expand();
    m_rootGroupItem->setGroup(m_groupManager->rootGroup());

    /*
    foreach (TaskManager::AbstractGroupableItem *item, m_groupManager->rootGroup()->members()) {
        kDebug() << item->name();
    }
    */

    connect(m_rootGroupItem, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(changeSizeHint(Qt::SizeHint)));

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMaximumSize(INT_MAX, INT_MAX);

    layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    layout->setMaximumSize(INT_MAX, INT_MAX);
    layout->setOrientation(Qt::Vertical);
    layout->addItem(m_rootGroupItem);
    setLayout(layout);

    configChanged();
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
        changed = true;
    }

    const bool showOnlyCurrentActivity = cg.readEntry("showOnlyCurrentActivity", true);
    if (showOnlyCurrentActivity != m_groupManager->showOnlyCurrentActivity()) {
        m_groupManager->setShowOnlyCurrentActivity(showOnlyCurrentActivity);
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
        changed = true;
    }

    TaskManager::GroupManager::TaskGroupingStrategy groupingStrategy =
        static_cast<TaskManager::GroupManager::TaskGroupingStrategy>(
            cg.readEntry("groupingStrategy",
                         static_cast<int>(TaskManager::GroupManager::ProgramGrouping))
        );
    if (groupingStrategy != m_groupManager->groupingStrategy()) {
        m_groupManager->setGroupingStrategy(groupingStrategy);
        changed = true;
    }

    const bool onlyGroupWhenFull = cg.readEntry("groupWhenFull", true);
    if (onlyGroupWhenFull != m_groupManager->onlyGroupWhenFull()) {
        adjustGroupingStrategy();
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
        changed = true;
    }

    const int maxRows = cg.readEntry("maxRows", 2);
    if (maxRows != m_rootGroupItem->maxRows()) {
        m_rootGroupItem->setMaxRows(maxRows);
        changed = true;
    }

    const bool forceRows = cg.readEntry("forceRows", false);
    if (forceRows != m_rootGroupItem->forceRows()) {
        m_rootGroupItem->setForceRows(forceRows);
        changed = true;
    }

    const bool showTooltip = cg.readEntry("showTooltip", true);
    if (showTooltip != m_showTooltip) {
        m_showTooltip = showTooltip;
        changed = true;
    }

    const bool highlightWindows = cg.readEntry("highlightWindows", false);
    if (highlightWindows != m_highlightWindows) {
        m_highlightWindows = highlightWindows;
        changed = true;
    }

    m_groupManager->readLauncherConfig();

    if (changed) {
        emit settingsChanged();
        update();
    }
}

void Tasks::reload()
{
    TaskGroup *newGroup = m_groupManager->rootGroup();
    if (newGroup != m_rootGroupItem->abstractItem()) {
        m_rootGroupItem->setGroup(newGroup);
    } else {
        m_rootGroupItem->reload();
    }
}

TaskManager::GroupManager &Tasks::groupManager() const
{
    return *m_groupManager;
}

Qt::KeyboardModifiers Tasks::groupModifierKey() const
{
    return m_groupModifierKey;
}

void Tasks::constraintsEvent(Plasma::Constraints constraints)
{
    //kDebug();
    if (m_groupManager && constraints & Plasma::ScreenConstraint) {
        Plasma::Containment* appletContainment = containment();
        if (appletContainment) {
            m_groupManager->setScreen(appletContainment->screen());
        }
    }

    if (constraints & Plasma::LocationConstraint) {
        QTimer::singleShot(500, this, SLOT(publishIconGeometry()));
    }

    if (constraints & Plasma::SizeConstraint) {
        adjustGroupingStrategy();
    }

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    emit constraintsChanged(constraints);
}

void Tasks::publishIconGeometry()
{
    foreach (AbstractTaskItem *item, m_rootGroupItem->members()) {
        item->publishIconGeometry();
    }
}

Plasma::Svg *Tasks::arrows()
{
    if (!m_arrows) {
        m_arrows = new Plasma::Svg(this);
        m_arrows->setImagePath("widgets/arrows");
        m_arrows->setContainsMultipleImages(true);
        m_arrows->resize(16, 16);
    }

    return m_arrows;
}

Plasma::FrameSvg *Tasks::itemBackground()
{
    if (!m_taskItemBackground) {
        m_taskItemBackground = new Plasma::FrameSvg(this);
        m_taskItemBackground->setImagePath("widgets/tasks");
        m_taskItemBackground->setCacheAllRenderedFrames(true);
    }

    return m_taskItemBackground;
}

void Tasks::resizeItemBackground(const QSizeF &size)
{
  //kDebug();
    if (!m_taskItemBackground) {
        itemBackground();
    }

    if (m_taskItemBackground->frameSize() == size) {
        //kDebug() << "Error2";
        return;
    }

    m_taskItemBackground->resizeFrame(size);

    QString oldPrefix = m_taskItemBackground->prefix();
    m_taskItemBackground->setElementPrefix("normal");
    //get the margins now
    m_taskItemBackground->getMargins(m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin);

    // the offscreen margins are always whatever the svg naturally is
    m_offscreenLeftMargin = m_leftMargin;
    m_offscreenTopMargin = m_topMargin;
    m_offscreenRightMargin = m_rightMargin;
    m_offscreenBottomMargin = m_bottomMargin;

    //if the task height is too little shrink the top and bottom margins
    if (size.height() - m_topMargin - m_bottomMargin < KIconLoader::SizeSmall) {
        m_topMargin = m_bottomMargin = qMax(1, int((size.height() - KIconLoader::SizeSmall)/2));
    }
    m_taskItemBackground->setElementPrefix(oldPrefix);
}

QSizeF Tasks::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (m_rootGroupItem && which == Qt::PreferredSize) {
        return m_rootGroupItem->preferredSize();
    } else {
        return Plasma::Applet::sizeHint(which, constraint);
    }
}

void Tasks::adjustGroupingStrategy()
{
    //FIXME: should use AbstractTaskItem::basicPreferredSize() but it seems to cause crashes
    //QSize itemSize = QSize(300, 30);
    //m_groupManager->setFullLimit(((size().width()*size().height()) / (itemSize.width()*itemSize.height())));
    //kDebug() << ((size().width()*size().height()) / (itemSize.width()*itemSize.height()));

    m_groupManager->setFullLimit(rootGroupItem()->optimumCapacity());
}

void Tasks::changeSizeHint(Qt::SizeHint which)
{
    emit sizeHintChanged(which);
    adjustGroupingStrategy();
}

void Tasks::createConfigurationInterface(KConfigDialog *parent)
{
     QWidget *widget = new QWidget;
     m_ui.setupUi(widget);
     connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
     connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
     parent->addPage(widget, i18n("General"), icon());

    m_ui.showTooltip->setChecked(m_showTooltip);
    m_ui.highlightWindows->setChecked(m_highlightWindows);
    m_ui.showOnlyCurrentDesktop->setChecked(m_groupManager->showOnlyCurrentDesktop());
    m_ui.showOnlyCurrentActivity->setChecked(m_groupManager->showOnlyCurrentActivity());
    m_ui.showOnlyCurrentScreen->setChecked(m_groupManager->showOnlyCurrentScreen());
    m_ui.showOnlyMinimized->setChecked(m_groupManager->showOnlyMinimized());
    m_ui.fillRows->setChecked(m_rootGroupItem->forceRows());

    m_ui.groupingStrategy->addItem(i18n("Do Not Group"),QVariant(TaskManager::GroupManager::NoGrouping));
    m_ui.groupingStrategy->addItem(i18n("Manually"),QVariant(TaskManager::GroupManager::ManualGrouping));
    m_ui.groupingStrategy->addItem(i18n("By Program Name"),QVariant(TaskManager::GroupManager::ProgramGrouping));

    connect(m_ui.groupingStrategy, SIGNAL(currentIndexChanged(int)), this, SLOT(dialogGroupingChanged(int)));

    switch (m_groupManager->groupingStrategy()) {
        case TaskManager::GroupManager::NoGrouping:
            m_ui.groupingStrategy->setCurrentIndex(0);
            break;
        case TaskManager::GroupManager::ManualGrouping:
            m_ui.groupingStrategy->setCurrentIndex(1);
            break;
        case TaskManager::GroupManager::ProgramGrouping:
            m_ui.groupingStrategy->setCurrentIndex(2);
            break;
        default:
             m_ui.groupingStrategy->setCurrentIndex(-1);
    }
    kDebug() << m_groupManager->groupingStrategy();

    m_ui.groupWhenFull->setChecked(m_groupManager->onlyGroupWhenFull());


    m_ui.sortingStrategy->addItem(i18n("Do Not Sort"),QVariant(TaskManager::GroupManager::NoSorting));
    m_ui.sortingStrategy->addItem(i18n("Manually"),QVariant(TaskManager::GroupManager::ManualSorting));
    m_ui.sortingStrategy->addItem(i18n("Alphabetically"),QVariant(TaskManager::GroupManager::AlphaSorting));
    m_ui.sortingStrategy->addItem(i18n("By Desktop"),QVariant(TaskManager::GroupManager::DesktopSorting));


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
        default:
             m_ui.sortingStrategy->setCurrentIndex(-1);
    }
 //   kDebug() << m_groupManager->sortingStrategy();
    m_ui.maxRows->setValue(m_rootGroupItem->maxRows());

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
    // just write the config here, and it will get applied in configChanged(),
    // which is called after this when the config dialog is accepted
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

    cg.writeEntry("showTooltip", m_ui.showTooltip->checkState() == Qt::Checked);
    cg.writeEntry("highlightWindows", m_ui.highlightWindows->checkState() == Qt::Checked);

    emit configNeedsSaving();
}

bool Tasks::showToolTip() const
{
    return m_showTooltip;
}

bool Tasks::highlightWindows() const
{
    return m_highlightWindows;
}

void Tasks::needsVisualFocus(bool focus)
{
    if (focus) {
        setStatus(Plasma::NeedsAttentionStatus);
    } else {
        foreach (AbstractTaskItem *item, m_rootGroupItem->members()) {
            if (item->taskFlags() & AbstractTaskItem::TaskWantsAttention) {
                // not time to go passive yet! :)
                return;
            }
        }
        setStatus(Plasma::PassiveStatus);
    }
}

TaskGroupItem* Tasks::rootGroupItem()
{
    return m_rootGroupItem;
}

QWidget *Tasks::popupDialog() const
{
    return m_popupDialog.data();
}

bool Tasks::isPopupShowing() const
{
    return m_popupDialog;
}

void Tasks::setPopupDialog(bool status)
{
    Q_UNUSED(status)
    QWidget *widget = qobject_cast<QWidget *>(sender());

    if (status && widget->isVisible()) {
        m_popupDialog = widget;
    } else if (m_popupDialog.data() == widget) {
        m_popupDialog.clear();
    }
}

K_EXPORT_PLASMA_APPLET(tasks, Tasks)

#include "tasks.moc"
