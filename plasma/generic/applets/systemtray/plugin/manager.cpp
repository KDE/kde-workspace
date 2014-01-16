/***************************************************************************
 *   manager.cpp                                                           *
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

#include "manager.h"

#include <QLoggingCategory>
#include <KGlobal>

#include <plasma/applet.h>

#include "protocol.h"
#include "task.h"
#include "debug.h"

//#include "../protocols/fdo/fdoprotocol.h"
#include "protocols/plasmoid/plasmoidprotocol.h"
#include "protocols/dbussystemtray/dbussystemtrayprotocol.h"

#include <QTimer>

namespace SystemTray
{

class Manager::Private
{
public:
    Private(Manager *manager)
        : q(manager)
    {
    }

    void setupProtocol(Protocol *protocol);

    Manager *q;
    QList<Task *> tasks;
    QQuickItem* rootItem;
};


Manager::Manager()
    : d(new Private(this))
{
    d->rootItem = 0;
    //QTimer::singleShot(50, this, SLOT(init())); // FIXME: remove timer
    init();
}

void Manager::init()
{
    d->setupProtocol(new SystemTray::DBusSystemTrayProtocol(this));
    d->setupProtocol(new SystemTray::PlasmoidProtocol(this));
}

Manager::~Manager()
{
    delete d;
}

QQuickItem* Manager::rootItem() const
{
    return d->rootItem;
}

void Manager::setRootItem(QQuickItem* item)
{
    d->rootItem = item;
}

QList<Task*> Manager::tasks() const
{
    return d->tasks;
}

void Manager::addTask(Task *task)
{
    connect(task, SIGNAL(destroyed(SystemTray::Task*)), this, SLOT(removeTask(SystemTray::Task*)));
    connect(task, SIGNAL(changedStatus()), this, SLOT(slotTaskStatusChanged()));

    qCDebug(SYSTEMTRAY) << "ST2" << task->name() << "(" << task->taskId() << ")";

    d->tasks.append(task);
    emit taskAdded(task);
    emit tasksChanged();
}

void Manager::slotTaskStatusChanged()
{
    Task* task = qobject_cast<Task*>(sender());

    if (task) {
        qCDebug(SYSTEMTRAY) << "ST2 emit taskStatusChanged(task);";
        emit taskStatusChanged(task);
    } else {
        qCDebug(SYSTEMTRAY) << "ST2 changed, but invalid cast";
    }
}
void Manager::removeTask(Task *task)
{
    d->tasks.removeAll(task);
    disconnect(task, 0, this, 0);
    emit taskRemoved(task);
    emit tasksChanged();
}

void Manager::Private::setupProtocol(Protocol *protocol)
{
    connect(protocol, SIGNAL(taskCreated(SystemTray::Task*)), q, SLOT(addTask(SystemTray::Task*)));
    protocol->init();
}

}


#include "manager.moc"
