/***********************************************************************************************************************
 * ROSA System Tray (KDE Plasmoid)
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


#ifndef __SYSTEMTRAY__TASKSPOOL_H
#define __SYSTEMTRAY__TASKSPOOL_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "uitask.h"

#include <QtCore/QObject>
#include <QtCore/QVariantHash>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward decl
namespace Plasma
{
class Applet;
}


namespace SystemTray
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declaration
class Task;
class UiTask;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class TasksPool
// Provides access from QML to tasks, notifies about new tasks
class TasksPool: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantHash tasks READ tasks)
public:
    explicit TasksPool(Plasma::Applet *host);
    virtual ~TasksPool();

    bool addTask(Task *task, UiTask::TaskHideState hide_state = UiTask::TaskHideStateAuto);
    void removeTask(Task *task);
    bool hasTask(Task *task) const;
    UiTask *uiTask(Task *task) const;

    QVariantHash tasks() const;

    Plasma::Applet *host() const;

signals:
    void newTask(const QString &task_id, QVariant task, int task_type);
    void deletedTask(const QString &task_id);

public slots:

private:
    struct _Private;
    _Private * const d;
};

} // namespe SystemTray

#endif // __SYSTEMTRAY__TASKSPOOL_H