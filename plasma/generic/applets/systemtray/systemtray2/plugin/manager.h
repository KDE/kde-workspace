/***************************************************************************
 *   manager.h                                                             *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#ifndef SYSTEMTRAYMANAGER_H
#define SYSTEMTRAYMANAGER_H

#include <QQuickItem>

#include <KConfigGroup>

#include <plasma/plasma.h>

// namespace Plasma
// {
// class Applet;
// }

namespace SystemTray
{

// class Applet;
class Notification;
class Task;
class Job;

/**
 * w
 * @short Creator and amalgamator of the supported system tray specifications
 **/
class Manager : public QObject
{
    Q_OBJECT

public:
    Manager();
    ~Manager();

    /**
     * @return a list of all known Task instances
     **/
    QList<Task*> tasks() const;

    QQuickItem* rootItem() const;
    void setRootItem(QQuickItem *item);

Q_SIGNALS:
    /**
     * Emitted whenever the list of tasks has changed
     **/
    void tasksChanged();
    /**
     * Emitted when a new task has been added
     **/
    void taskAdded(SystemTray::Task *task);

    /**
     * Emitted when status of task changes (such as it changing from
     * Passive to NeedsAttention)
     **/
    void taskStatusChanged(SystemTray::Task *task);

    /**
     * Emitted when a task has been removed
     **/
    void taskRemoved(SystemTray::Task *task);

private Q_SLOTS:
    void init();
    void addTask(SystemTray::Task *task);
    void removeTask(SystemTray::Task *task);
    void slotTaskStatusChanged();

private:
    class Private;
    Private* const d;

//     friend class Applet;
};

}


#endif
