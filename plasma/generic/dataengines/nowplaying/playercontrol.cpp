/*
 * Copyright 2008  Alex Merry <alex.merry@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "playercontrol.h"
#include "playeractionjob.h"

#include <kdebug.h>

PlayerControl::PlayerControl(QObject* parent, Player::Ptr player)
    : Plasma::Service(parent),
      m_player(player)
{
    setObjectName( QLatin1String("nowplaying controller" ));
    setName("nowplaying");
    if (m_player) {
        setDestination(m_player->name());
        setObjectName( QLatin1String("nowplaying controller for" ) + m_player->name());
        qDebug() << "Created a player control for" << m_player->name();
    } else {
        qDebug() << "Created a dead player control";
    }
    updateEnabledOperations();
}

void PlayerControl::updateEnabledOperations()
{
    if (m_player) {
        /*
        qDebug() << "Updating operations:"
                 << "\n    Play:" << m_player->canPlay()
                 << "\n    Pause:" << m_player->canPause()
                 << "\n    Stop:" << m_player->canStop()
                 << "\n    Next:" << m_player->canGoNext()
                 << "\n    Previous:" << m_player->canGoPrevious()
                 << "\n    Volume:" << m_player->canSetVolume()
                 << "\n    Seek:" << m_player->canSeek();
                 */
        setOperationEnabled("play", m_player->canPlay());
        setOperationEnabled("pause", m_player->canPause());
        setOperationEnabled("stop", m_player->canStop());
        setOperationEnabled("next", m_player->canGoNext());
        setOperationEnabled("previous", m_player->canGoPrevious());
        setOperationEnabled("volume", m_player->canSetVolume());
        setOperationEnabled("seek", m_player->canSeek());
    } else {
        qDebug() << "No player";
    }
}

Plasma::ServiceJob* PlayerControl::createJob(const QString& operation,
                                             QMap<QString,QVariant>& parameters)
{
    qDebug() << "Job" << operation << "with arguments" << parameters << "requested";
    return new PlayerActionJob(m_player, operation, parameters, this);
}

#include "playercontrol.moc"

// vim: sw=4 sts=4 et tw=100
