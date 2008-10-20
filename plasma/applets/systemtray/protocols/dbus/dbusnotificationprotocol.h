/***************************************************************************
 *   dbusnotificationprotocol.h                                            *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Dmitry Suzdalev <dimsuz@gmail.com>                 *
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

#ifndef DBUSNOTIFICATIONPROTOCOL_H
#define DBUSNOTIFICATIONPROTOCOL_H

#include "../../core/notificationprotocol.h"

#include <plasma/dataengine.h>


namespace SystemTray
{
namespace DBus
{

class NotificationProtocol : public SystemTray::NotificationProtocol
{
    Q_OBJECT

public:
    NotificationProtocol(QObject *parent);
    ~NotificationProtocol();
    void init();

private slots:
    void prepareNotification(const QString &source);
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
    void removeNotification(const QString &source);
    void relayAction(const QString &source, const QString &actionId);

private:
    class Private;
    Private* const d;
};

}
}


#endif
