/***************************************************************************
 *                                                                         *
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
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


#include "host.h"
#include "manager.h"
#include "task.h"

#include <QDebug>
#include <QTimer>
#include <QVariant>

namespace SystemTray
{

Manager *SystemTray::Host::s_manager = 0;
int SystemTray::Host::s_managerUsage = 0;

class HostPrivate {
public:
    Host *q;
    QList<SystemTray::Task*> tasks;
};

Host::Host(QObject* parent) :
    QObject(parent)
{
    d = new HostPrivate;
    QTimer::singleShot(500, this, SLOT(init())); // FIXME: remove
    //init();
}

Host::~Host()
{
    delete d;
}


void Host::init()
{
    if (!s_manager) {
        s_manager = new SystemTray::Manager();
        connect(s_manager, SIGNAL(tasksChanged()), this, SIGNAL(tasksChanged()));
    }
    ++s_managerUsage;
    emit tasksChanged();
}

QQmlListProperty<SystemTray::Task> Host::tasks()
{
    if (s_manager) {
        // We need to keep a reference to the list we're passing to the runtime
        d->tasks = s_manager->tasks();
        QQmlListProperty<SystemTray::Task> l(this, d->tasks);
        return l;
    }
    QQmlListProperty<SystemTray::Task> l;
    return l;
}

} // namespace

#include "host.moc"
