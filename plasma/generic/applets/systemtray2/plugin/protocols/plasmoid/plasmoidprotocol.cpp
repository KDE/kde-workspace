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
