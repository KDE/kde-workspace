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
#include <zlib.h>

#define TIMEOUT 200

namespace SystemTray
{


bool taskLessThan(const Task *lhs, const Task *rhs)
{
    /* Sorting of systemtray icons
     *
     * We sort (and thus group) in the following order, from high to low priority
     * - Notifications always comes first
     * - Category
     * - Name
     */

    const QString _not = QStringLiteral("org.kde.notifications");
    if (lhs->taskId() == _not) {
        return true;
    }
    if (rhs->taskId() == _not) {
        return false;
    }

    if (lhs->category() != rhs->category()) {
        QMap<Task::Category, int> weights;
        weights.insert(Task::Communications, 0);
        weights.insert(Task::SystemServices, 1);
        weights.insert(Task::Hardware, 2);
        weights.insert(Task::ApplicationStatus, 3);
        weights.insert(Task::UnknownCategory, 4);

        return weights.value(lhs->category()) < weights.value(rhs->category());
    }

    return lhs->name() < rhs->name();
}

Manager *SystemTray::Host::s_manager = 0;
int SystemTray::Host::s_managerUsage = 0;

class HostPrivate {
public:
    Host *q;

    // Keep references to the list to avoid full refreshes
    //QList<SystemTray::Task*> tasks;
    QList<SystemTray::Task*> shownTasks;
    QList<SystemTray::Task*> hiddenTasks;
    QQmlListProperty<SystemTray::Task> shownTasksDeclarative;
    QQmlListProperty<SystemTray::Task> hiddenTasksDeclarative;
    //QQmlListProperty<SystemTray::Task> tasksDeclarative;

    QStringList categories;

    bool showTask(Task *task);
    QTimer compressionTimer;
};

Host::Host(QObject* parent) :
    QObject(parent)
{
    d = new HostPrivate;
    init();
    connect(&d->compressionTimer, &QTimer::timeout, this, &Host::compressionTimeout);
}

Host::~Host()
{
    delete d;
}

void Host::init()
{
    if (!s_manager) {
        s_manager = new SystemTray::Manager();
//        connect(s_manager, &Manager::tasksChanged, this, &Host::tasksChanged);
        connect(s_manager, &Manager::taskAdded, this, &Host::taskAdded);
        connect(s_manager, &Manager::taskRemoved, this, &Host::taskRemoved);
        connect(s_manager, &Manager::taskStatusChanged, this, &Host::taskStatusChanged);
    }
    ++s_managerUsage;

    initTasks();
    emit categoriesChanged();
}

void Host::compressionTimeout()
{
    qDebug() << "ST2 tasksChanged";
    emit tasksChanged();
    d->compressionTimer.stop();

}


void Host::initTasks()
{
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

    qSort(d->shownTasks.begin(), d->shownTasks.end(), taskLessThan);
    qSort(d->hiddenTasks.begin(), d->hiddenTasks.end(), taskLessThan);

    QQmlListProperty<SystemTray::Task> _shown(this, d->shownTasks);
    d->shownTasksDeclarative = _shown;

    QQmlListProperty<SystemTray::Task> _hidden(this, d->hiddenTasks);
    d->hiddenTasksDeclarative = _hidden;
    qDebug() << "ST2 init starting timer" << TIMEOUT;
    d->compressionTimer.start(TIMEOUT);
}

void Host::setRootItem(QQuickItem* rootItem)
{
    //qDebug() << "Set root item";
    if (s_manager && s_manager->rootItem() != rootItem) {
        s_manager->setRootItem(rootItem);
        //emit rootItemChanged();
    }
}

QQuickItem* Host::rootItem()
{
    if (s_manager) {
        return s_manager->rootItem();
    } else {
        return 0;
    }
}

QQmlListProperty<SystemTray::Task> Host::hiddenTasks()
{
    return d->hiddenTasksDeclarative;
}

QQmlListProperty< Task > Host::shownTasks()
{
    return d->shownTasksDeclarative;

}

void Host::taskAdded(Task* task)
{
    if (d->showTask(task)) {
        d->shownTasks.append(task);
        qSort(d->shownTasks.begin(), d->shownTasks.end(), taskLessThan);
    } else {
        d->hiddenTasks.append(task);
        qSort(d->hiddenTasks.begin(), d->hiddenTasks.end(), taskLessThan);
    }
    d->compressionTimer.start(TIMEOUT);
    emit tasksChanged();
}

void Host::taskRemoved(Task* task)
{
    if (d->showTask(task)) {
        d->shownTasks.removeAll(task);
    } else {
        d->hiddenTasks.removeAll(task);
    }
    d->compressionTimer.start(TIMEOUT);
}

bool HostPrivate::showTask(Task *task) {
    return task->shown() && task->status() != SystemTray::Task::Passive;
}

void Host::taskStatusChanged(SystemTray::Task *task)
{
    if (task) {
        if (d->shownTasks.contains(task)) {
            if (!d->showTask(task)) {
                qDebug() << "ST2 Migrating shown -> hidden" << task->name();
                d->shownTasks.removeAll(task);
                d->hiddenTasks.append(task);
                qSort(d->hiddenTasks.begin(), d->hiddenTasks.end(), taskLessThan);
                d->compressionTimer.start(TIMEOUT);
            }
        } else if (d->hiddenTasks.contains(task)) {
            if (d->showTask(task)) {
                qDebug() << "ST2 Migrating hidden -> shown" << task->name();
                d->hiddenTasks.removeAll(task);
                d->shownTasks.append(task);
                qSort(d->shownTasks.begin(), d->shownTasks.end(), taskLessThan);
                d->compressionTimer.start(TIMEOUT);
            }
        }
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
