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
// Includes
#include "uitask.h"

#include "taskspool.h"

#include "../protocols/plasmoid/plasmoidtask.h"
#include "../protocols/fdo/fdotask.h"
#include "../protocols/dbussystemtray/dbussystemtraytask.h"


namespace SystemTray
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// struct UiTask::_Private
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct UiTask::_Private
{
    TasksPool &pool;
    UiTask::TaskType const task_type;
    QString const task_id;
    Task * const task;
    UiTask::TaskHideState hide_state;

    _Private(TasksPool &pool, QString task_id, Task *task);
};


UiTask::_Private::_Private(TasksPool &pool, QString task_id, Task *task):
    pool(pool),
    task_type(UiTask::DefineTaskType(task)),
    task_id(task_id),
    task(task),
    hide_state(UiTask::TaskHideStateAuto)
{

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class UiTask
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UiTask::UiTask(TasksPool &pool, QString task_id, Task *task):
    d(new _Private(pool, task_id, task))
{
    connect(task, SIGNAL(changedStatus()), this, SLOT(_onChangedStatus()));
}


UiTask::~UiTask()
{
    disconnect(d->task, 0, this, 0);
    delete d;
}


UiTask::TaskHideState UiTask::hideState() const
{
    return d->hide_state;
}


void UiTask::setHideState(UiTask::TaskHideState state)
{
    if (d->hide_state == state)
        return;
    d->hide_state = state;
    emit changedHideState();
}


UiTask::TaskType UiTask::type() const
{
    return d->task_type;
}


UiTask::TaskStatus UiTask::status() const
{
    return TaskStatus(d->task->status());
}


QVariant UiTask::task() const
{
    return QVariant::fromValue(static_cast<QObject*>(d->task));
}

QString UiTask::taskId() const
{
    return d->task_id;
}


QString UiTask::name() const {
    return  d->task ? d->task->typeId() : "";
}


QGraphicsWidget *UiTask::widget() const
{
    return d->task->widget(d->pool.host(), false);
}


Plasma::Applet *UiTask::host() const
{
    return d->pool.host();
}


UiTask::TaskType UiTask::DefineTaskType(Task *t)
{
    if (qobject_cast<DBusSystemTrayTask*>(t)) {
        return UiTask::TaskTypeStatusItem;
    } else if (qobject_cast<PlasmoidTask*>(t)) {
        return UiTask::TaskTypePlasmoid;
    } else if (qobject_cast<FdoTask*>(t)) {
        return UiTask::TaskTypeX11Task;
    }
    return UiTask::TaskTypeUnknown;
}


void UiTask::_onChangedStatus()
{
    emit changedStatus();
}



} // namespace SystemTray
