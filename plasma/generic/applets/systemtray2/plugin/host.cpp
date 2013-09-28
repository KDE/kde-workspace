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

class SystemtrayManagerPrivate {
public:
    Host *q;
    //QHash<QString, Task*> tasks;
    QList<SystemTray::Task*> tasks;
    //QList<
};

Host::Host(QObject* parent) :
    QObject(parent)
{
    d = new SystemtrayManagerPrivate;
    //QTimer::singleShot(2000, this, SLOT(init()));
    init();
}

Host::~Host()
{
    delete d;
}


void Host::init()
{
    if (!s_manager) {
        qDebug() << "ST Initialising manager";
        s_manager = new SystemTray::Manager();
        connect(s_manager, SIGNAL(tasksChanged()), this, SIGNAL(tasksChanged()));
    }

    ++s_managerUsage;

    emit tasksChanged();
}

QQmlListProperty<SystemTray::Task> Host::tasks()
{
    if (s_manager) {
        qDebug() << "ST task begin";
        d->tasks = s_manager->tasks();
        //d->tasks.clear();
        QQmlListProperty<SystemTray::Task> l(this, d->tasks);
        //d->tasks = l;
        //d->tasks <<  s_manager->tasks();
        //Task* _t = s_manager->tasks().at(0);

        //qDebug() << " ======> ST: tasks: " << d->tasks.count();
        return l;
    }
    QQmlListProperty<SystemTray::Task> l;
    qDebug() << "ST Empty list";
    return l;
}

} // namespace

#include "host.moc"
