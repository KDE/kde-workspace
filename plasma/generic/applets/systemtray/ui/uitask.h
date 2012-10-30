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


#ifndef __SYSTEMTRAY__UITASK_H
#define __SYSTEMTRAY__UITASK_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <QtCore/QObject>
#include <QtCore/QVariant>

#include "../core/task.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class QGraphicsWidget;

namespace Plasma
{
class Applet;
}


namespace SystemTray
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
class TasksPool;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class UiTask
/// Provides access to properties of every task from QML code
class UiTask: public QObject
{
    Q_OBJECT

    Q_ENUMS(TaskType)

    Q_PROPERTY(TaskType type READ type CONSTANT)
    Q_PROPERTY(QGraphicsWidget *widget READ widget CONSTANT)
    Q_PROPERTY(QVariant task READ task CONSTANT)
    Q_PROPERTY(QString typeId READ typeId CONSTANT) // TODO: it may change
    Q_PROPERTY(QString taskId READ taskId CONSTANT)
    Q_PROPERTY(QString name READ name NOTIFY changedName)

public:

    enum TaskType
    {
        TaskTypeUnknown = 0,
        TaskTypePlasmoid,
        TaskTypeX11Task,
        TaskTypeStatusItem
    };

    explicit UiTask(TasksPool &pool, QString task_id, Task *task);
    virtual ~UiTask();

    TaskType type() const;
    QVariant task() const;
    QString taskId() const;
    QGraphicsWidget *widget() const;
    Plasma::Applet *host() const;

    QString name() const;
    QString typeId() const;

    static UiTask::TaskType DefineTaskType(Task *t);


signals:
    void changedName();

private:
    struct _Private;
    _Private * const d;
};


} // namespace SystemTray

#endif // __SYSTEMTRAY__UITASK_H
