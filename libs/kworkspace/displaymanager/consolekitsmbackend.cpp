/*
   Copyright (C) 2004 Oswald Buddenhagen <ossi@kde.org>

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

#include "consolekitsmbackend.h"

#include <kuser.h>

class CKManager : public QDBusInterface
{
public:
    CKManager() :
        QDBusInterface(
                QLatin1String("org.freedesktop.ConsoleKit"),
                QLatin1String("/org/freedesktop/ConsoleKit/Manager"),
                QLatin1String("org.freedesktop.ConsoleKit.Manager"),
                QDBusConnection::systemBus()) {}
};

class CKSeat : public QDBusInterface
{
public:
    CKSeat(const QDBusObjectPath &path) :
        QDBusInterface(
                QLatin1String("org.freedesktop.ConsoleKit"),
                path.path(),
                QLatin1String("org.freedesktop.ConsoleKit.Seat"),
                QDBusConnection::systemBus()) {}
};

class CKSession : public QDBusInterface
{
public:
    CKSession(const QDBusObjectPath &path) :
        QDBusInterface(
                QLatin1String("org.freedesktop.ConsoleKit"),
                path.path(),
                QLatin1String("org.freedesktop.ConsoleKit.Session"),
                QDBusConnection::systemBus()) {}
    void getSessionLocation(SessEnt &se)
    {
        QString tty;
        QDBusReply<QString> r = call(QLatin1String("GetX11Display"));
        if (r.isValid() && !r.value().isEmpty()) {
            QDBusReply<QString> r2 = call(QLatin1String("GetX11DisplayDevice"));
            tty = r2.value();
            se.display = r.value();
            se.tty = false;
        } else {
            QDBusReply<QString> r2 = call(QLatin1String("GetDisplayDevice"));
            tty = r2.value();
            se.display = tty;
            se.tty = true;
        }
        se.vt = tty.mid(strlen("/dev/tty")).toInt();
    }
};

ConsoleKitSMBackend::ConsoleKitSMBackend()
{

}

ConsoleKitSMBackend::~ConsoleKitSMBackend()
{

}

bool
ConsoleKitSMBackend::canShutdown()
{
    QDBusReply<bool> canStop = CKManager().call(QLatin1String("CanStop"));
    if (canStop.isValid())
        return canStop.value();
    return false;
}

void
ConsoleKitSMBackend::shutdown(KWorkSpace::ShutdownType shutdownType, KWorkSpace::ShutdownMode /*shutdownMode*/, const QString& bootOption)
{
    if (!bootOption.isEmpty())
        return;
    // FIXME: entirely ignoring shutdownMode
    CKManager().call(QLatin1String(
            shutdownType == KWorkSpace::ShutdownTypeReboot ? "Restart" : "Stop"));
    // if even CKManager call fails, there is nothing more to be done
    return;

}

bool
ConsoleKitSMBackend::isSwitchable()
{
    QDBusObjectPath currentSeat;
    if (getCurrentSeat(0, &currentSeat)) {
        CKSeat CKseat(currentSeat);
        if (CKseat.isValid()) {
            QDBusReply<bool> r = CKseat.call(QLatin1String("CanActivateSessions"));
            if (r.isValid())
                return r.value();
        }
    }
    return false;
}

bool
ConsoleKitSMBackend::localSessions(SessList& list)
{
    QDBusObjectPath currentSession, currentSeat;
    if (getCurrentSeat(&currentSession, &currentSeat)) {
        // ConsoleKit part
        if (QDBusConnection::systemBus().interface()->isServiceRegistered("org.freedesktop.ConsoleKit")) {
            foreach (const QDBusObjectPath &sp, getSessionsForSeat(currentSeat)) {
                CKSession lsess(sp);
                if (lsess.isValid()) {
                    SessEnt se;
                    lsess.getSessionLocation(se);
                    // "Warning: we haven't yet defined the allowed values for this property.
                    // It is probably best to avoid this until we do."
                    QDBusReply<QString> r = lsess.call(QLatin1String("GetSessionType"));
                    if (r.value() != QLatin1String("LoginWindow")) {
                        QDBusReply<unsigned> r2 = lsess.call(QLatin1String("GetUnixUser"));
                        se.user = KUser(K_UID(r2.value())).loginName();
                        se.session = "<unknown>";
                    }
                    se.self = (sp == currentSession);
                    list.append(se);
                }
            }
        }
        else {
            return false;
        }
        return true;
    }
    return false;
}

bool
ConsoleKitSMBackend::switchVT(int vt)
{
    QDBusObjectPath currentSeat;
    if (getCurrentSeat(0, &currentSeat)) {
        // ConsoleKit part
        if (QDBusConnection::systemBus().interface()->isServiceRegistered("org.freedesktop.ConsoleKit")) {
            foreach (const QDBusObjectPath &sp, getSessionsForSeat(currentSeat)) {
                CKSession lsess(sp);
                if (lsess.isValid()) {
                    SessEnt se;
                    lsess.getSessionLocation(se);
                    if (se.vt == vt) {
                        if (se.tty) // ConsoleKit simply ignores these
                            return false;
                        lsess.call(QLatin1String("Activate"));
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool
ConsoleKitSMBackend::getCurrentSeat(QDBusObjectPath *currentSession, QDBusObjectPath *currentSeat)
{
    CKManager man;
    QDBusReply<QDBusObjectPath> r = man.call(QLatin1String("GetCurrentSession"));
    if (r.isValid()) {
        CKSession sess(r.value());
        if (sess.isValid()) {
            QDBusReply<QDBusObjectPath> r2 = sess.call(QLatin1String("GetSeatId"));
            if (r2.isValid()) {
                if (currentSession)
                    *currentSession = r.value();
                *currentSeat = r2.value();
                return true;
            }
        }
    }
    return false;
}

QList<QDBusObjectPath>
ConsoleKitSMBackend::getSessionsForSeat(const QDBusObjectPath &path)
{
    if (path.path().startsWith("/org/freedesktop/ConsoleKit")) {
        CKSeat seat(path);
        if (seat.isValid()) {
            QDBusReply<QList<QDBusObjectPath> > r = seat.call(QLatin1String("GetSessions"));
            if (r.isValid()) {
                // This will contain only local sessions:
                // - this is only ever called when isSwitchable() is true => local seat
                // - remote logins into the machine are assigned to other seats
                return r.value();
            }
        }
    }
    return QList<QDBusObjectPath>();
}
