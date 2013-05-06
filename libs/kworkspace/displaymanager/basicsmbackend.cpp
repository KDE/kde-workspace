/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Martin Bříza <mbriza@redhat.com>
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

#include "basicsmbackend.h"
#include "support.h"

#include <QtDBus>
#include <cstdlib>

#include <kuser.h>


#define _DBUS_PROPERTIES_IFACE "org.freedesktop.DBus.Properties"
#define _DBUS_PROPERTIES_GET "Get"

#define DBUS_PROPERTIES_IFACE QLatin1String(_DBUS_PROPERTIES_IFACE)
#define DBUS_PROPERTIES_GET QLatin1String(_DBUS_PROPERTIES_GET)

#define _SYSTEMD_SERVICE "org.freedesktop.login1"
#define _SYSTEMD_BASE_PATH "/org/freedesktop/login1"
#define _SYSTEMD_MANAGER_IFACE _SYSTEMD_SERVICE ".Manager"
#define _SYSTEMD_SESSION_BASE_PATH _SYSTEMD_BASE_PATH "/Session"
#define _SYSTEMD_SEAT_IFACE _SYSTEMD_SERVICE ".Seat"
#define _SYSTEMD_SEAT_BASE_PATH _SYSTEMD_BASE_PATH "/Seat"
#define _SYSTEMD_SESSION_IFACE _SYSTEMD_SERVICE ".Session"
#define _SYSTEMD_USER_PROPERTY "User"
#define _SYSTEMD_SEAT_PROPERTY "Seat"
#define _SYSTEMD_SESSIONS_PROPERTY "Sessions"
#define _SYSTEMD_SWITCH_PROPERTY "Activate"

#define SYSTEMD_SERVICE QLatin1String(_SYSTEMD_SERVICE)
#define SYSTEMD_BASE_PATH QLatin1String(_SYSTEMD_BASE_PATH)
#define SYSTEMD_MANAGER_IFACE QLatin1String(_SYSTEMD_MANAGER_IFACE)
#define SYSTEMD_SESSION_BASE_PATH QLatin1String(_SYSTEMD_SESSION_BASE_PATH)
#define SYSTEMD_SEAT_IFACE QLatin1String(_SYSTEMD_SEAT_IFACE)
#define SYSTEMD_SEAT_BASE_PATH QLatin1String(_SYSTEMD_SEAT_BASE_PATH)
#define SYSTEMD_SESSION_IFACE QLatin1String(_SYSTEMD_SESSION_IFACE)
#define SYSTEMD_USER_PROPERTY QLatin1String(_SYSTEMD_USER_PROPERTY)
#define SYSTEMD_SEAT_PROPERTY QLatin1String(_SYSTEMD_SEAT_PROPERTY)
#define SYSTEMD_SESSIONS_PROPERTY QLatin1String(_SYSTEMD_SESSIONS_PROPERTY)
#define SYSTEMD_SWITCH_CALL QLatin1String(_SYSTEMD_SWITCH_PROPERTY)

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

class SystemdManager : public QDBusInterface
{
public:
    SystemdManager() :
        QDBusInterface(
                SYSTEMD_SERVICE,
                SYSTEMD_BASE_PATH,
                SYSTEMD_MANAGER_IFACE,
                QDBusConnection::systemBus()) {}
};

class SystemdSeat : public QDBusInterface
{
public:
    SystemdSeat(const QDBusObjectPath &path) :
        QDBusInterface(
                SYSTEMD_SERVICE,
                path.path(),
                SYSTEMD_SEAT_IFACE,
                QDBusConnection::systemBus()) {}
    /* HACK to be able to extract a(so) type from QDBus, property doesn't do the trick */
    QList<NamedDBusObjectPath> getSessions() {
        QDBusMessage message = QDBusMessage::createMethodCall(service(), path(), DBUS_PROPERTIES_IFACE, DBUS_PROPERTIES_GET);
        message <<  interface() << SYSTEMD_SESSIONS_PROPERTY;
        QDBusMessage reply = QDBusConnection::systemBus().call(message);

        QVariantList args = reply.arguments();
        if (!args.isEmpty()) {
            QList<NamedDBusObjectPath> namedPathList = qdbus_cast< QList<NamedDBusObjectPath> >(args.at(0).value<QDBusVariant>().variant().value<QDBusArgument>());
            return namedPathList;
        }
        return QList<NamedDBusObjectPath>();
    }
};

class SystemdSession : public QDBusInterface
{
public:
    SystemdSession(const QDBusObjectPath &path) :
        QDBusInterface(
                SYSTEMD_SERVICE,
                path.path(),
                SYSTEMD_SESSION_IFACE,
                QDBusConnection::systemBus()) {}
    /* HACK to be able to extract (so) type from QDBus, property doesn't do the trick */
    NamedDBusObjectPath getSeat() {
        QDBusMessage message = QDBusMessage::createMethodCall(service(), path(), DBUS_PROPERTIES_IFACE, DBUS_PROPERTIES_GET);
        message <<  interface() <<  SYSTEMD_SEAT_PROPERTY;
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
        message <<  interface() <<  SYSTEMD_USER_PROPERTY;
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

Login1SMBackend::Login1SMBackend() : BasicSMBackend(NULL) {

}

Login1SMBackend::~Login1SMBackend() {

}


BasicSMBackend::BasicSMBackend ( KDMBackendPrivate* p )
: NullSMBackend()
, d(p)
, DMType(Dunno)
{
    const char *dpy;
    const char *ctl;
    
    qDBusRegisterMetaType<NamedDBusObjectPath>();
    qDBusRegisterMetaType<QList<NamedDBusObjectPath> >();
    qDBusRegisterMetaType<NumberedDBusObjectPath>();
    
    if (DMType == Dunno) {
        if (!(dpy = ::getenv("DISPLAY")))
            DMType = NoDM;
        else if ((ctl = ::getenv("DM_CONTROL")))
            DMType = KDM;
        else if ((ctl = ::getenv("XDM_MANAGED")) && ctl[0] == '/')
            DMType = LightDM;
        else if (::getenv("GDMSESSION"))
            DMType = GDM;
        else
            DMType = NoDM;
    }
}

BasicSMBackend::~BasicSMBackend() {

}


static bool getCurrentSeat(QDBusObjectPath *currentSession, QDBusObjectPath *currentSeat)
{
    SystemdManager man;
    QDBusReply<QDBusObjectPath> r = man.call(QLatin1String("GetSessionByPID"), (uint) QCoreApplication::applicationPid());
    if (r.isValid()) {
        SystemdSession sess(r.value());
        if (sess.isValid()) {
            NamedDBusObjectPath namedPath = sess.getSeat();
            if (currentSession)
                *currentSession = r.value();
            *currentSeat = namedPath.path;
            return true;
        }
    }
    else {
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
    }
    return false;
}

static QList<QDBusObjectPath> getSessionsForSeat(const QDBusObjectPath &path)
{
    if (path.path().startsWith(SYSTEMD_BASE_PATH)) { // systemd path incoming
        SystemdSeat seat(path);
        if (seat.isValid()) {
            QList<NamedDBusObjectPath> r = seat.getSessions();
            QList<QDBusObjectPath> result;
            foreach (const NamedDBusObjectPath &namedPath, r)
                result.append(namedPath.path);
            // This pretty much can't contain any other than local sessions as the seat is retrieved from the current session
            return result;
        }
    }
    else if (path.path().startsWith("/org/freedesktop/ConsoleKit")) {
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

#ifndef KDM_NO_SHUTDOWN
bool 
Login1SMBackend::canShutdown() {
    QDBusReply<QString> canPowerOff = SystemdManager().call(QLatin1String("CanPowerOff"));
    if (canPowerOff.isValid())
        return canPowerOff.value() != QLatin1String("no");
    return false;
}

bool
BasicSMBackend::canShutdown()
{
    if (DMType == GDM || DMType == NoDM || DMType == LightDM) {
        QDBusReply<bool> canStop = CKManager().call(QLatin1String("CanStop"));
        if (canStop.isValid())
            return canStop.value();
        return false;
    }

    QByteArray re;

    return d && d->exec("caps\n", re) && re.indexOf("\tshutdown") >= 0;
}

void 
Login1SMBackend::shutdown ( KWorkSpace::ShutdownType shutdownType, KWorkSpace::ShutdownMode shutdownMode, const QString& bootOption ) 
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
    QDBusReply<QString> check = SystemdManager().call(QLatin1String(
            shutdownType == KWorkSpace::ShutdownTypeReboot ? "Reboot" : "PowerOff"), interactive);
    // if the login1 call fails, try the legacy way of turning the computer off
    if (!check.isValid()) {
        BasicSMBackend::shutdown(shutdownType, shutdownMode, bootOption);
    }
}


void
BasicSMBackend::shutdown(KWorkSpace::ShutdownType shutdownType,
                          KWorkSpace::ShutdownMode shutdownMode, /* NOT Default */
                          const QString &bootOption)
{
    bool cap_ask;
    if (DMType == KDM) {
        QByteArray re;
        cap_ask = d && d->exec("caps\n", re) && re.indexOf("\tshutdown ask") >= 0;
    } else {
        if (!bootOption.isEmpty())
            return;

        if (DMType == GDM || DMType == NoDM || DMType == LightDM) {
            // FIXME: entirely ignoring shutdownMode
            CKManager().call(QLatin1String(
                    shutdownType == KWorkSpace::ShutdownTypeReboot ? "Restart" : "Stop"));
            // if even CKManager call fails, there is nothing more to be done
            return;
        }

        cap_ask = false;
    }
    if (!cap_ask && shutdownMode == KWorkSpace::ShutdownModeInteractive)
        shutdownMode = KWorkSpace::ShutdownModeForceNow;

    QByteArray cmd;
    cmd.append("shutdown\t");
    cmd.append(shutdownType == KWorkSpace::ShutdownTypeReboot ?
                "reboot\t" : "halt\t");
    if (!bootOption.isEmpty())
        cmd.append("=").append(bootOption.toLocal8Bit()).append("\t");
    cmd.append(shutdownMode == KWorkSpace::ShutdownModeInteractive ?
                "ask\n" :
                shutdownMode == KWorkSpace::ShutdownModeForceNow ?
                "forcenow\n" :
                shutdownMode == KWorkSpace::ShutdownModeTryNow ?
                "trynow\n" : "schedule\n");
    if (d)
        d->exec(cmd.data());
}
#endif // KDM_NO_SHUTDOWN

void 
Login1SMBackend::setLock ( bool  ) 
{
    
}

// This only tells KDM to not auto-re-login upon session crash
void
BasicSMBackend::setLock(bool on)
{
    if (DMType == KDM)
        d->exec(on ? "lock\n" : "unlock\n");
}

bool
Login1SMBackend::isSwitchable() {
    QDBusObjectPath currentSeat;
    if (getCurrentSeat(0, &currentSeat)) {
        SystemdSeat SDseat(currentSeat);
        if (SDseat.isValid()) {
            QVariant prop = SDseat.property("CanMultiSession");
            if (prop.isValid())
                return prop.toBool();
        }
    }
    return false;
}


bool
BasicSMBackend::isSwitchable()
{
    if (DMType == GDM || DMType == LightDM) {
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

    QByteArray re;
    
    return d && d->exec("caps\n", re) && re.indexOf("\tlocal") >= 0;
}

bool
Login1SMBackend::localSessions ( SessList& list ) {
    QDBusObjectPath currentSession, currentSeat;
    if (getCurrentSeat(&currentSession, &currentSeat)) {
        foreach (const QDBusObjectPath &sp, getSessionsForSeat(currentSeat)) {
            SystemdSession lsess(sp);
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
                    list.append(se);
                }
            }
        }
        return true;
    }
    return false;
}

bool
BasicSMBackend::localSessions(SessList &list)
{
    if (DMType == GDM || DMType == LightDM) {
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

    QByteArray re;
    
    if (!d || !d->exec("list\talllocal\n", re))
        return false;
    const QStringList sess = QString(re.data() + 3).split(QChar('\t'), QString::SkipEmptyParts);
    for (QStringList::ConstIterator it = sess.constBegin(); it != sess.constEnd(); ++it) {
        QStringList ts = (*it).split(QChar(','));
        SessEnt se;
        se.display = ts[0];
        se.vt = ts[1].mid(2).toInt();
        se.user = ts[2];
        se.session = ts[3];
        se.self = (ts[4].indexOf('*') >= 0);
        se.tty = (ts[4].indexOf('t') >= 0);
        list.append(se);
    }
    return true;
}

bool
Login1SMBackend::switchVT(int vt)
{
    QDBusObjectPath currentSeat;
    if (getCurrentSeat(0, &currentSeat)) {
        if (QDBusConnection::systemBus().interface()->isServiceRegistered(SYSTEMD_SERVICE)) {
            foreach (const QDBusObjectPath &sp, getSessionsForSeat(currentSeat)) {
                SystemdSession lsess(sp);
                if (lsess.isValid()) {
                    SessEnt se;
                    lsess.getSessionLocation(se);
                    if (se.vt == vt) {
                        lsess.call(SYSTEMD_SWITCH_CALL);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool
BasicSMBackend::switchVT(int vt)
{
    if (DMType == GDM || DMType == LightDM) {
        QDBusObjectPath currentSession, currentSeat;
        if (getCurrentSeat(&currentSession, &currentSeat)) {
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

    return d && d->exec(QString("activate\tvt%1\n").arg(vt).toLatin1());
}
