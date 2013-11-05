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

#include <KPluginInfo>

#include <QDebug>

namespace SystemTray
{

PlasmoidTask::PlasmoidTask(QQuickItem* rootItem, const QString &packageName, const QString &systrayPackageRoot, QObject *parent)
    : Task(parent),
      m_taskId(packageName),
      m_taskItem(0),
      m_rootItem(rootItem),
      m_valid(true)
{
    qDebug() << "Loading applet: " << packageName;
    m_taskItem = new PlasmoidInterface(packageName, systrayPackageRoot, m_rootItem);
    if (!m_taskItem) {
        qDebug() << "Invalid applet taskitem";
        m_valid = false;
        return;
    }
    connect(m_taskItem, &PlasmoidInterface::statusChanged, this, &PlasmoidTask::updateStatus);
    connect(m_taskItem, &PlasmoidInterface::defaultRepresentationChanged, this, &PlasmoidTask::taskItemExpandedChanged);
    KPluginInfo info = m_taskItem->pluginInfo();
    setName(info.name());
    updateStatus();
}

PlasmoidTask::~PlasmoidTask()
{
}

void PlasmoidTask::updateStatus()
{
// enum ItemStatus {
//     UnknownStatus = 0, /**< The status is unknown **/
//     PassiveStatus = 1, /**< The Item is passive **/
//     ActiveStatus = 2, /**< The Item is active **/
//     NeedsAttentionStatus = 3, /**< The Item needs attention **/
//     AcceptingInputStatus = 4 /**< The Item is accepting input **/
// };
//     enum Status {
//         UnknownStatus = 0,
//         Passive = 1,
//         Active = 2,
//         NeedsAttention = 3
//     };
    qDebug() << "Updateing Plasmoid status";
    if (!m_taskItem) {
        return;
    }
    const Plasma::Types::ItemStatus ps = m_taskItem->status();
    if (ps == Plasma::Types::UnknownStatus) {
        setStatus(Task::UnknownStatus);
    } else if (ps == Plasma::Types::PassiveStatus) {
        setStatus(Task::Passive);
    } else if (ps == Plasma::Types::NeedsAttentionStatus) {
        setStatus(Task::NeedsAttention);
    } else {
        setStatus(Task::Active);
    }
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

QQuickItem* PlasmoidTask::taskItemExpanded()
{
    if (!m_taskItem) {
        return 0;
    }
    return m_taskItem->defaultRepresentation();
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
