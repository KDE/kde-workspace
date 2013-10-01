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
#include "protocols/plasmoid/plasmoidprotocol.h"

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
    QmlObject* notificationsPlasmoid = 0;
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
        d->notificationsPlasmoid = PlasmoidProtocol::loadPlasmoid(plugin, QVariantHash(),
                                                      s_manager->rootItem());
        if (!d->notificationsPlasmoid) {
            return new QQuickItem(s_manager->rootItem());
        }
    }
    return qobject_cast<QQuickItem*>(d->notificationsPlasmoid->rootObject());
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

void Host::setRootItem(QQuickItem* rootItem)
{
    qDebug() << "Set root item";
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

} // namespace

#include "host.moc"
