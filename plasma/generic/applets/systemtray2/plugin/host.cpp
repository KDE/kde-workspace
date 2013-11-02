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

    // Keep references to the list to avoid full refreshes
    QList<SystemTray::Task*> tasks;
    QList<SystemTray::Task*> shownTasks;
    QList<SystemTray::Task*> hiddenTasks;
    QQmlListProperty<SystemTray::Task> shownTasksDeclarative;
    QQmlListProperty<SystemTray::Task> hiddenTasksDeclarative;
    QQmlListProperty<SystemTray::Task> tasksDeclarative;

    PlasmoidInterface* notificationsPlasmoid = 0;
    QStringList categories;

    bool showTask(Task *task);
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
    qDebug() <<"ST2 manager init";
    if (!s_manager) {
        s_manager = new SystemTray::Manager();
        connect(s_manager, SIGNAL(tasksChanged()), this, SIGNAL(tasksChanged()));
//         connect(s_manager, SIGNAL(taskStatusChanged()), this, SIGNAL(taskStatusChanged()));
        connect(s_manager, &Manager::taskAdded, this, &Host::taskAdded);
        connect(s_manager, &Manager::taskRemoved, this, &Host::taskRemoved);
        connect(s_manager, &Manager::taskStatusChanged, this, &Host::taskStatusChanged);
//         foreach (SystemTray::Task* t, s_manager->tasks()) {
//             connect(t, SIGNAL(changedStatus()), this, SLOT(taskStatusChanged()));
//         }
    }
    ++s_managerUsage;

    initTasks();
    emit categoriesChanged();
}

void Host::initTasks()
{
    d->shownTasks.clear();
    d->hiddenTasks.clear();

    if (s_manager) {
        QList<SystemTray::Task*> allTasks = s_manager->tasks();
        foreach (SystemTray::Task *task, allTasks) {
            if (d->showTask(task)) {
                d->shownTasks.append(task);
            } else {
                d->hiddenTasks.append(task);
            }
        }
    }

    QQmlListProperty<SystemTray::Task> _shown(this, d->shownTasks);
    d->shownTasksDeclarative = _shown;

    QQmlListProperty<SystemTray::Task> _hidden(this, d->hiddenTasks);
    d->hiddenTasksDeclarative = _hidden;

    emit tasksChanged();
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
    return d->hiddenTasksDeclarative;
}

QQmlListProperty< Task > Host::shownTasks()
{
//     d->shownTasks.clear();
//     if (s_manager) {
//         QList<SystemTray::Task*> allTasks = s_manager->tasks();
//         foreach (SystemTray::Task *task, allTasks) {
//             if (d->showTask(task)) {
//                 d->shownTasks.append(task);
//             }
//         }
//     }
//     QQmlListProperty<SystemTray::Task> l(this, d->shownTasks);
//     return l;

    return d->shownTasksDeclarative;

}

void Host::taskAdded(Task* task)
{
    qDebug() << "ST2 Task added" << task->name();
    if (d->showTask(task)) {
        d->shownTasks.append(task);
    } else {
        d->hiddenTasks.append(task);
    }
    emit tasksChanged();
}

void Host::taskRemoved(Task* task)
{
    qDebug() << "ST2 Task removed" << task->name(); // crash?
    if (d->showTask(task)) {
        d->shownTasks.removeAll(task);
    } else {
        d->hiddenTasks.removeAll(task);
    }
    emit tasksChanged();
}


bool HostPrivate::showTask(Task *task) {
    return task->shown() && task->status() != SystemTray::Task::Passive;
}

void Host::taskStatusChanged(SystemTray::Task *task)
{
    if (task) {
        qDebug() << "==> ST2 Migrate changed Task?: " << task->name() << task->status();
        if (d->shownTasks.contains(task) && !d->showTask(task)) {
            qDebug() << "ST2 Migrating shown -> hidden" << task->name();
            d->shownTasks.removeAll(task);
            d->hiddenTasks.append(task);
            emit tasksChanged();
        } else if (d->hiddenTasks.contains(task) && d->showTask(task)) {
            qDebug() << "ST2 Migrating hidden -> shown" << task->name();
            d->hiddenTasks.removeAll(task);
            d->shownTasks.append(task);
            emit tasksChanged();
        }

    } else {
        Manager* man = qobject_cast<Manager*>(sender());
        qDebug() << "ST2 Task changed, but failed to cast to Task* " << man;
    }
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
    qDebug() << "ST2 " << cats;
    return cats;
}


} // namespace

#include "host.moc"
