/***********************************************************************************************************************
 * System Tray (KDE Plasmoid)
 * Copyright ⓒ 2011-2012 ROSA  <support@rosalab.ru>
 * License: GPLv3+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **********************************************************************************************************************/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "taskspool.h"

#include "uitask.h"
#include "../core/task.h"

#include <inttypes.h>

#include <QtCore/QMap>

#include <KDE/Plasma/Applet>


namespace SystemTray
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace
{

struct _TaskData
{
    QString task_id;
    UiTask *task;

    _TaskData(): task(0) {}
    _TaskData(QString task_id, UiTask *task): task_id(task_id), task(task) {}
};

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// struct TasksPool::_Private
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct TasksPool::_Private
{
    typedef QMap<Task*, _TaskData> Tasks;

    Plasma::Applet *host;
    Tasks tasks;
    QVariantHash tasks_hash;

    _Private(Plasma::Applet *host): host(host) {}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class TasksPool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TasksPool::TasksPool(Plasma::Applet *host):
    QObject(host),
    d(new _Private(host))
{
}


TasksPool::~TasksPool()
{
    for (_Private::Tasks::const_iterator it = d->tasks.constBegin(), e = d->tasks.constEnd(); it != e; ++it) {
        emit deletedTask(it.value().task_id);
        delete it.value().task;
    }
    delete d;
}


bool TasksPool::addTask(Task *task, UiTask::TaskHideState hide_state)
{
    if ( !task || d->tasks.contains(task) ) {
        return false;
    }

    UiTask::TaskType task_type = UiTask::DefineTaskType(task);
    Q_ASSERT(task_type >= 0 && task_type < 4);

    QString task_id = QString::number(reinterpret_cast<uintmax_t>(task));

    UiTask *ui_task = new(std::nothrow) UiTask(*this, task_id, task);
    if (!ui_task) {
        return false;
    }

    ui_task->setHideState(hide_state);
    d->tasks.insert(task, _TaskData(task_id, ui_task));
    d->tasks_hash.insert(task_id, QVariant::fromValue(static_cast<QObject*>(ui_task)));
    emit newTask(task_id, QVariant::fromValue(static_cast<QObject*>(ui_task)), int(task_type));
    return true;
}


void TasksPool::removeTask(Task *task)
{
    if ( !task || !d->tasks.contains(task) )
        return;
    _TaskData data = d->tasks.value(task);
    emit deletedTask(data.task_id);;
    d->tasks.remove(task);
    d->tasks_hash.remove(data.task_id);
    delete data.task;
}


bool TasksPool::hasTask(Task *task) const
{
    return d->tasks.contains(task);
}


UiTask *TasksPool::uiTask(Task *task) const
{
    if (!hasTask(task))
        return 0;
    return d->tasks[task].task;
}


QVariantHash TasksPool::tasks() const
{
    return d->tasks_hash;
}


Plasma::Applet *TasksPool::host() const
{
    return d->host;
}



} // namespace SystemTray
