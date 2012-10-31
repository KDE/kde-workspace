/***********************************************************************************************************************
 * KDE System Tray (Plasmoid)
 *
 * Copyright (C) 2012 ROSA  <support@rosalab.ru>
 * License: GPLv2+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
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
    Task * const task;

    _Private(TasksPool &pool, Task *task);
};


UiTask::_Private::_Private(TasksPool &pool, Task *task):
    pool(pool),
    task(task)
{

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class UiTask
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UiTask::UiTask(TasksPool &pool, Task *task):
    d(new _Private(pool, task))
{
}


UiTask::~UiTask()
{
    delete d;
}


QVariant UiTask::task() const
{
    return QVariant::fromValue(static_cast<QObject*>(d->task));
}


QGraphicsWidget *UiTask::widget() const
{
    return d->task->widget(d->pool.host(), false);
}


Plasma::Applet *UiTask::host() const
{
    return d->pool.host();
}




} // namespace SystemTray
