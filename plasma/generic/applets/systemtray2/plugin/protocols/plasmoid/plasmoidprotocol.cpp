/***************************************************************************
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

#include "plasmoidtask.h"
#include "plasmoidprotocol.h"
#include "plasmoidinterface.h"

#include "../../manager.h"

#include <Plasma/PluginLoader>
#include <kdeclarative/qmlobject.h>
#include <KLocalizedString>
#include <kplugintrader.h>

#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

namespace SystemTray
{

PlasmoidProtocol::PlasmoidProtocol(QObject *parent)
    : Protocol(parent),
      m_tasks()
{
}

PlasmoidProtocol::~PlasmoidProtocol()
{
}

void PlasmoidProtocol::init()
{
    Plasma::Package package = Plasma::PluginLoader::self()->loadPackage("Plasma/Applet");
    package.setPath("org.kde.systemtray");
    m_systrayPackageRoot = package.path();

    qDebug() << "ST2 PackagePathQml: " << m_systrayPackageRoot;

    //X-Plasma-NotificationArea
    KPluginInfo::List applets = Plasma::PluginLoader::self()->listAppletInfo(QString());
    const QString constraint = QString("[X-Plasma-NotificationArea] == 'true'");
    //KPluginTrader::applyConstraints(applets, constraint);

    QStringList ownApplets;

    QStringList blacklist;
    blacklist << "notifier";
    //blacklist << "org.kde.notifications";
    //blacklist << "org.kde.systrayplasmoidtest";

    QMap<QString, KPluginInfo> sortedApplets;
    foreach (const KPluginInfo &info, applets) {
        KService::Ptr service = info.service();
        if (!blacklist.contains(info.pluginName()) && service->property("X-Plasma-NotificationArea", QVariant::Bool).toBool()) {
            // if we already have a plugin with this exact name in it, then check if it is the
            // same plugin and skip it if it is indeed already listed
            if (sortedApplets.contains(info.name())) {

                bool dupe = false;
                // it is possible (though poor form) to have multiple applets
                // with the same visible name but different plugins, so we hve to check all values
                foreach (const KPluginInfo &existingInfo, sortedApplets.values(info.name())) {
                    if (existingInfo.pluginName() == info.pluginName()) {
                        dupe = true;
                        break;
                    }
                }

                if (dupe) {
                    continue;
                }
            }

            // insertMulti becase it is possible (though poor form) to have multiple applets
            // with the same visible name but different plugins
            sortedApplets.insertMulti(info.name(), info);
        }
    }

    foreach (const KPluginInfo &info, sortedApplets) {
        //qDebug() << " Adding applet: " << info.name();
        qDebug() << "\n\n ==========================================================================================";
        if (!blacklist.contains(info.pluginName())) {
            newTask(info.pluginName());
        }
    }

}

void PlasmoidProtocol::newTask(const QString &service)
{
    qDebug() << "ST new task " << service;
    if (m_tasks.contains(service)) {
        return;
    }

    Manager* m = qobject_cast<Manager*>(parent());
    QQuickItem* rootItem = m->rootItem();

    PlasmoidTask *task = new PlasmoidTask(rootItem, service, m_systrayPackageRoot, this);

    m_tasks[service] = task;
    emit taskCreated(task);
}

void PlasmoidProtocol::cleanupTask(const QString &service)
{
    PlasmoidTask *task = m_tasks.value(service);

    if (task) {
        m_tasks.remove(service);
        if (task->isValid()) {
            emit task->destroyed(task);
        }
        task->deleteLater();
    }
}

}

#include "plasmoidprotocol.moc"
