/*
 * Copyright (C) 2012 Rex Dieter <rdieter@fedoraproject.org>
 * Copyright (C) 2012 Kevin Kofler <Kevin@tigcc.ticalc.org>
 * Copyright (C) 2013 Martin Bříza <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "login1smbackend.h"

#include <QtDBus>

#include <kuser.h>

#define _DBUS_PROPERTIES_IFACE "org.freedesktop.DBus.Properties"
#define _DBUS_PROPERTIES_GET "Get"

#define DBUS_PROPERTIES_IFACE QLatin1String(_DBUS_PROPERTIES_IFACE)
#define DBUS_PROPERTIES_GET QLatin1String(_DBUS_PROPERTIES_GET)

#define _LOGIN1_SERVICE "org.freedesktop.login1"
#define _LOGIN1_BASE_PATH "/org/freedesktop/login1"
#define _LOGIN1_MANAGER_IFACE _LOGIN1_SERVICE ".Manager"
#define _LOGIN1_SESSION_BASE_PATH _LOGIN1_BASE_PATH "/Session"
#define _LOGIN1_SEAT_IFACE _LOGIN1_SERVICE ".Seat"
#define _LOGIN1_SEAT_BASE_PATH _LOGIN1_BASE_PATH "/Seat"
#define _LOGIN1_SESSION_IFACE _LOGIN1_SERVICE ".Session"
#define _LOGIN1_USER_PROPERTY "User"
#define _LOGIN1_SEAT_PROPERTY "Seat"
#define _LOGIN1_SESSIONS_PROPERTY "Sessions"
#define _LOGIN1_SWITCH_PROPERTY "Activate"

#define LOGIN1_SERVICE QLatin1String(_LOGIN1_SERVICE)
#define LOGIN1_BASE_PATH QLatin1String(_LOGIN1_BASE_PATH)
#define LOGIN1_MANAGER_IFACE QLatin1String(_LOGIN1_MANAGER_IFACE)
#define LOGIN1_SESSION_BASE_PATH QLatin1String(_LOGIN1_SESSION_BASE_PATH)
#define LOGIN1_SEAT_IFACE QLatin1String(_LOGIN1_SEAT_IFACE)
#define LOGIN1_SEAT_BASE_PATH QLatin1String(_LOGIN1_SEAT_BASE_PATH)
#define LOGIN1_SESSION_IFACE QLatin1String(_LOGIN1_SESSION_IFACE)
#define LOGIN1_USER_PROPERTY QLatin1String(_LOGIN1_USER_PROPERTY)
#define LOGIN1_SEAT_PROPERTY QLatin1String(_LOGIN1_SEAT_PROPERTY)
#define LOGIN1_SESSIONS_PROPERTY QLatin1String(_LOGIN1_SESSIONS_PROPERTY)
#define LOGIN1_SWITCH_CALL QLatin1String(_LOGIN1_SWITCH_PROPERTY)

struct NamedDBusObjectPath
{
    QString name;
    QDBusObjectPath path;
};
Q_DECLARE_METATYPE(NamedDBusObjectPath)
Q_DECLARE_METATYPE(QList<NamedDBusObjectPath>)

// Marshall the NamedDBusObjectPath data into a D-Bus argument
QDBusArgument &operator<<(QDBusArgument &argument, const NamedDBusObjectPath &namedPath)
{
    argument.beginStructure();
    argument << namedPath.name << namedPath.path;
    argument.endStructure();
    return argument;
}

// Retrieve the NamedDBusObjectPath data from the D-Bus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, NamedDBusObjectPath &namedPath)
{
    argument.beginStructure();
    argument >> namedPath.name >> namedPath.path;
    argument.endStructure();
    return argument;
}

struct NumberedDBusObjectPath
{
    uint num;
    QDBusObjectPath path;
};
Q_DECLARE_METATYPE(NumberedDBusObjectPath)

// Marshall the NumberedDBusObjectPath data into a D-Bus argument
QDBusArgument &operator<<(QDBusArgument &argument, const NumberedDBusObjectPath &numberedPath)
{
    argument.beginStructure();
    argument << numberedPath.num << numberedPath.path;
    argument.endStructure();
    return argument;
}

// Retrieve the NumberedDBusObjectPath data from the D-Bus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, NumberedDBusObjectPath &numberedPath)
{
    argument.beginStructure();
    argument >> numberedPath.num >> numberedPath.path;
    argument.endStructure();
    return argument;
}

class Login1Manager : public QDBusInterface
{
public:
    Login1Manager() :
        QDBusInterface(
                LOGIN1_SERVICE,
                LOGIN1_BASE_PATH,
                LOGIN1_MANAGER_IFACE,
                QDBusConnection::systemBus()) {}
};

class Login1Seat : public QDBusInterface
{
public:
    Login1Seat(const QDBusObjectPath &path) :
        QDBusInterface(
                LOGIN1_SERVICE,
                path.path(),
                LOGIN1_SEAT_IFACE,
                QDBusConnection::systemBus()) {}
    /* HACK to be able to extract a(so) type from QDBus, property doesn't do the trick */
    QList<NamedDBusObjectPath> getSessions() {
        QDBusMessage message = QDBusMessage::createMethodCall(service(), path(), DBUS_PROPERTIES_IFACE, DBUS_PROPERTIES_GET);
        message <<  interface() << LOGIN1_SESSIONS_PROPERTY;
        QDBusMessage reply = QDBusConnection::systemBus().call(message);

        QVariantList args = reply.arguments();
        if (!args.isEmpty()) {
            QList<NamedDBusObjectPath> namedPathList = qdbus_cast< QList<NamedDBusObjectPath> >(args.at(0).value<QDBusVariant>().variant().value<QDBusArgument>());
            return namedPathList;
        }
        return QList<NamedDBusObjectPath>();
    }
};

class Login1Session : public QDBusInterface
{
public:
    Login1Session(const QDBusObjectPath &path) :
        QDBusInterface(
                LOGIN1_SERVICE,
                path.path(),
                LOGIN1_SESSION_IFACE,
                QDBusConnection::systemBus()) {}
    /* HACK to be able to extract (so) type from QDBus, property doesn't do the trick */
    NamedDBusObjectPath getSeat() {
        QDBusMessage message = QDBusMessage::createMethodCall(service(), path(), DBUS_PROPERTIES_IFACE, DBUS_PROPERTIES_GET);
        message <<  interface() <<  LOGIN1_SEAT_PROPERTY;
        QDBusMessage reply = QDBusConnection::systemBus().call(message);

        QVariantList args = reply.arguments();
        if (!args.isEmpty()) {
            NamedDBusObjectPath namedPath;
            args.at(0).value<QDBusVariant>().variant().value<QDBusArgument>() >> namedPath;
            return namedPath;
        }
        return NamedDBusObjectPath();
    }
    NumberedDBusObjectPath getUser() {
        QDBusMessage message = QDBusMessage::createMethodCall(service(), path(), DBUS_PROPERTIES_IFACE, DBUS_PROPERTIES_GET);
        message <<  interface() <<  LOGIN1_USER_PROPERTY;
        QDBusMessage reply = QDBusConnection::systemBus().call(message);

        QVariantList args = reply.arguments();
        if (!args.isEmpty()) {
            NumberedDBusObjectPath numberedPath;
            args.at(0).value<QDBusVariant>().variant().value<QDBusArgument>() >> numberedPath;
            return numberedPath;
        }
        return NumberedDBusObjectPath();
    }
    void getSessionLocation(SessEnt &se)
    {
        se.tty = (property("Type").toString() != QLatin1String("x11"));
        se.display = property(se.tty ? "TTY" : "Display").toString();
        se.vt = property("VTNr").toInt();
    }
};

Login1SMBackend::Login1SMBackend()
{
    qDBusRegisterMetaType<NamedDBusObjectPath>();
    qDBusRegisterMetaType<QList<NamedDBusObjectPath> >();
    qDBusRegisterMetaType<NumberedDBusObjectPath>();
}

Login1SMBackend::~Login1SMBackend()
{

}

bool
Login1SMBackend::getCurrentSeat(QDBusObjectPath* currentSession, QDBusObjectPath* currentSeat)
{
    Login1Manager man;
    QDBusReply<QDBusObjectPath> r = man.call(QLatin1String("GetSessionByPID"), (uint) QCoreApplication::applicationPid());
    if (r.isValid()) {
        Login1Session sess(r.value());
        if (sess.isValid()) {
            NamedDBusObjectPath namedPath = sess.getSeat();
            if (currentSession)
                *currentSession = r.value();
            *currentSeat = namedPath.path;
            return true;
        }
    }
    return false;
}

QList<QDBusObjectPath>
Login1SMBackend::getSessionsForSeat(const QDBusObjectPath& path)
{
    Login1Seat seat(path);
    if (seat.isValid()) {
        QList<NamedDBusObjectPath> r = seat.getSessions();
        QList<QDBusObjectPath> result;
        foreach (const NamedDBusObjectPath &namedPath, r)
            result.append(namedPath.path);
        // This pretty much can't contain any other than local sessions as the seat is retrieved from the current session
        return result;
    }
    return QList<QDBusObjectPath>();
}

bool
Login1SMBackend::canShutdown()
{
    QDBusReply<QString> canPowerOff = Login1Manager().call(QLatin1String("CanPowerOff"));
    if (canPowerOff.isValid())
        return canPowerOff.value() != QLatin1String("no");
    return false;
}

void
Login1SMBackend::shutdown(KWorkSpace::ShutdownType shutdownType, KWorkSpace::ShutdownMode shutdownMode, const QString& bootOption)
{
    if (!bootOption.isEmpty())
        return;
    // systemd supports only 2 modes:
    // * interactive = true: brings up a PolicyKit prompt if other sessions are active
    // * interactive = false: rejects the shutdown if other sessions are active
    // There are no schedule or force modes.
    // We try to map our 4 shutdown modes in the sanest way.
    bool interactive = (shutdownMode == KWorkSpace::ShutdownModeInteractive
                        || shutdownMode == KWorkSpace::ShutdownModeForceNow);
    QDBusReply<QString> check = Login1Manager().call(QLatin1String(
            shutdownType == KWorkSpace::ShutdownTypeReboot ? "Reboot" : "PowerOff"), interactive);
}

bool
Login1SMBackend::isSwitchable()
{
    QDBusObjectPath currentSeat;
    if (getCurrentSeat(0, &currentSeat)) {
        Login1Seat seat(currentSeat);
        if (seat.isValid()) {
            QVariant prop = seat.property("CanMultiSession");
            if (prop.isValid())
                return prop.toBool();
        }
    }
    return false;
}

bool
Login1SMBackend::localSessions(SessList& list)
{
    QDBusObjectPath currentSession, currentSeat;
    if (getCurrentSeat(&currentSession, &currentSeat)) {
        foreach (const QDBusObjectPath &sp, getSessionsForSeat(currentSeat)) {
            Login1Session lsess(sp);
            if (lsess.isValid()) {
                SessEnt se;
                lsess.getSessionLocation(se);
                if ((lsess.property("Class").toString() != QLatin1String("greeter")) &&
                        (lsess.property("State").toString() == QLatin1String("online") ||
                        lsess.property("State").toString() == QLatin1String("active"))) {
                    NumberedDBusObjectPath numberedPath = lsess.getUser();
                    se.display = lsess.property("Display").toString();
                    se.vt = lsess.property("VTNr").toInt();
                    se.user = KUser(K_UID(numberedPath.num)).loginName();
                    /* TODO:
                        * regarding the session name in this, it IS possible to find it out - logind tracks the session leader PID
                        * the problem is finding out the name of the process, I could come only with reading /proc/PID/comm which
                        * doesn't seem exactly... right to me --mbriza
                        */
                    se.session = "<unknown>";
                    se.self = lsess.property("Display").toString() == ::getenv("DISPLAY"); /* Bleh once again */
                    se.tty = !lsess.property("TTY").toString().isEmpty();
                }
                list.append(se);
            }
        }
        return true;
    }
    return false;
}

bool
Login1SMBackend::switchVT(int vt)
{
    QDBusObjectPath currentSeat;
    if (getCurrentSeat(0, &currentSeat)) {
        // systemd part // preferred
        if (QDBusConnection::systemBus().interface()->isServiceRegistered(LOGIN1_SERVICE)) {
            foreach (const QDBusObjectPath &sp, getSessionsForSeat(currentSeat)) {
                Login1Session lsess(sp);
                if (lsess.isValid()) {
                    SessEnt se;
                    lsess.getSessionLocation(se);
                    if (se.vt == vt) {
                        lsess.call(LOGIN1_SWITCH_CALL);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void
Login1SMBackend::lockSwitchVT(int vt)
{
    QDBusInterface screensaver("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
    if (screensaver.isValid()) {
        screensaver.call("Lock");
    }
    else {
        QDBusObjectPath currentSession;
        QDBusObjectPath currentSeat;
        if (getCurrentSeat(&currentSession, &currentSeat)) {
            Login1Session lsess(currentSession);
            if (lsess.isValid()) {
                lsess.call("Lock");
            }
        }
    }

    switchVT(vt);
}
