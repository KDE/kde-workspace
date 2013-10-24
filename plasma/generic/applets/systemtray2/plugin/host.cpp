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
#include "protocols/plasmoid/plasmoidinterface.h"

#include <klocalizedstring.h>

#include <Plasma/Package>
#include <Plasma/PluginLoader>

#include <QDebug>
#include <QQuickItem>
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
    QList<SystemTray::Task*> shownTasks;
    QList<SystemTray::Task*> hiddenTasks;
    PlasmoidInterface* notificationsPlasmoid = 0;
    QStringList categories;
};

Host::Host(QObject* parent) :
    QObject(parent)
{
    d = new HostPrivate;
    init();
}

Host::~Host()
{
    delete d;
}


QQuickItem* Host::notificationsPlasmoid(const QString &plugin)
{
    if (!d->notificationsPlasmoid) {
//         const QString plugin = QStringLiteral("org.kde.notifications");
        //const QString plugin = QStringLiteral("org.kde.systrayplasmoidtest");
        d->notificationsPlasmoid = new PlasmoidInterface(plugin, s_manager->rootItem());
        if (!d->notificationsPlasmoid) {
            qDebug() << "Notifications failed to load.";
            return new QQuickItem(s_manager->rootItem());
        }
    }
    return d->notificationsPlasmoid;
}


void Host::init()
{
    if (!s_manager) {
        s_manager = new SystemTray::Manager();
        connect(s_manager, SIGNAL(tasksChanged()), this, SIGNAL(tasksChanged()));
    }
    ++s_managerUsage;
    emit tasksChanged();
    emit categoriesChanged();
}

void Host::setRootItem(QQuickItem* rootItem)
{
    //qDebug() << "Set root item";
    s_manager->setRootItem(rootItem);
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

QQmlListProperty<SystemTray::Task> Host::hiddenTasks()
{
    d->hiddenTasks.clear();
    if (s_manager) {
        QList<SystemTray::Task*> allTasks = s_manager->tasks();
        foreach (SystemTray::Task *task, allTasks) {
            if (task->shown() || task->status() == SystemTray::Task::Passive) {
                d->hiddenTasks.append(task);
            }
        }
    }
    QQmlListProperty<SystemTray::Task> l(this, d->hiddenTasks);
    return l;
}

QQmlListProperty< Task > Host::shownTasks()
{
    d->shownTasks.clear();
    if (s_manager) {
        QList<SystemTray::Task*> allTasks = s_manager->tasks();
        foreach (SystemTray::Task *task, allTasks) {
            if (task->shown() && task->status() != SystemTray::Task::Passive) {
                d->shownTasks.append(task);
            }
        }
    }
    QQmlListProperty<SystemTray::Task> l(this, d->shownTasks);
    return l;
}

QStringList Host::categories() const
{
    QList<SystemTray::Task*> allTasks = s_manager->tasks();
    QStringList cats;
    QList<SystemTray::Task::Category> cnt;
    foreach (SystemTray::Task *task, allTasks) {
        const SystemTray::Task::Category c = task->category();
        if (cnt.contains(c)) {
            continue;
        }
        cnt.append(c);

        if (c == SystemTray::Task::UnknownCategory) {
            cats.append(i18n("Unknown Category"));
        } else if (c == SystemTray::Task::ApplicationStatus) {
            cats.append(i18n("Application Status"));
        } else if (c == SystemTray::Task::Communications) {
            cats.append(i18n("Communications"));
        } else if (c == SystemTray::Task::SystemServices) {
            cats.append(i18n("System Services"));
        } else if (c == SystemTray::Task::Hardware) {
            cats.append(i18n("Hardware"));
        }
    }
    qDebug() << "ST " << cats;
    return cats;
}


} // namespace

#include "host.moc"
