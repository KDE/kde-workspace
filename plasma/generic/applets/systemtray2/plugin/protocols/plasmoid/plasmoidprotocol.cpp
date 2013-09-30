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

#include <Plasma/PluginLoader>

#include <kplugintrader.h>

#include <QDebug>


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
    //X-Plasma-NotificationArea
    KPluginInfo::List applets = Plasma::PluginLoader::self()->listAppletInfo(QString());
    const QString constraint = QString("[X-Plasma-NotificationArea] == 'true'");
    //KPluginTrader::applyConstraints(applets, constraint);

    QStringList ownApplets;

    QMap<QString, KPluginInfo> sortedApplets;
    foreach (const KPluginInfo &info, applets) {
        KService::Ptr service = info.service();
        if (service->property("X-Plasma-NotificationArea", QVariant::Bool).toBool()) {
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
        qDebug() << " Adding applet: " << info.name();
    }

}


void PlasmoidProtocol::newTask(const QString &service)
{
    qDebug() << "ST new task " << service;
    if (m_tasks.contains(service)) {
        return;
    }

    PlasmoidTask *task = new PlasmoidTask(service, this);

    m_tasks[service] = task;
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

void PlasmoidProtocol::initedTask(PlasmoidTask *task)
{
    emit taskCreated(task);
}

}

#include "plasmoidprotocol.moc"
