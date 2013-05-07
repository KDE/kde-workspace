/*
   Copyright (C) 2004,2005 Oswald Buddenhagen <ossi@kde.org>
   Copyright (C) 2005 Stephan Kulow <coolo@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#ifndef CONSOLEKITSMBACKEND_H
#define CONSOLEKITSMBACKEND_H

#include "displaymanager/dmbackend.h"

#include <QtDBus>

class ConsoleKitSMBackend : public NullSMBackend {
private:
    QList<QDBusObjectPath> getSessionsForSeat(const QDBusObjectPath &path);
    bool getCurrentSeat(QDBusObjectPath *currentSession, QDBusObjectPath *currentSeat);
public:
    ConsoleKitSMBackend();
    virtual ~ConsoleKitSMBackend();
    virtual bool canShutdown();
    virtual void shutdown(KWorkSpace::ShutdownType shutdownType, KWorkSpace::ShutdownMode shutdownMode, const QString &bootOption = QString());
    virtual bool isSwitchable();
    virtual bool localSessions(SessList &list);
    virtual bool switchVT(int vt);
    virtual void lockSwitchVT(int vt);
};

#endif // CONSOLEKITSMBACKEND_H
