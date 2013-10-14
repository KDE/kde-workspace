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
#include "../../host.h"

#include <QtCore/QMetaEnum>
#include <kdeclarative/qmlobject.h>

#include <QDebug>

namespace SystemTray
{

PlasmoidTask::PlasmoidTask(QQuickItem* rootItem, const QString &packageName, QObject *parent)
    : Task(parent),
      m_taskId(packageName),
      m_taskItem(0),
      m_rootItem(rootItem),
      m_valid(true)
{
    qDebug() << "Loading applet: " << packageName;
    m_taskItem = new PlasmoidInterface(packageName, m_rootItem);
    if (!m_taskItem) {
        qDebug() << "Invalid applet taskitem";
        m_valid = false;
        return;
    }
}

PlasmoidTask::~PlasmoidTask()
{
}

bool PlasmoidTask::isValid() const
{
    return m_valid;
}

bool PlasmoidTask::isEmbeddable() const
{
    return false; // this task cannot be embed because it only provides information to GUI part
}

bool PlasmoidTask::isWidget() const
{
    return false; // isn't a widget
}

void PlasmoidTask::setShortcut(QString text) {
    if (m_shortcut != text) {
        m_shortcut = text;
        emit changedShortcut();
    }
}


QString PlasmoidTask::taskId() const
{
    return m_taskId;
}

QQuickItem* PlasmoidTask::taskItem()
{
    if (!m_taskItem) {

    }
    return m_taskItem;
}

QIcon PlasmoidTask::icon() const
{
    return m_icon;
}
//Status

void PlasmoidTask::syncStatus(QString newStatus)
{
    Task::Status status = (Task::Status)metaObject()->enumerator(metaObject()->indexOfEnumerator("Status")).keyToValue(newStatus.toLatin1());

    if (this->status() == status) {
        return;
    }

    setStatus(status);
}

}

#include "plasmoidtask.moc"
