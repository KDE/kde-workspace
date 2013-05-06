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

#include "kdisplaymanager.h"
#include "displaymanager/basicdmbackend.h"
#include "displaymanager/support.h"
#include "displaymanager/basicsmbackend.h"

#include <kapplication.h>
#include <klocale.h>
#include <kuser.h>

#include <QtDBus/QtDBus>
#include <QRegExp>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

KDisplayManager::KDisplayManager()
{
<<<<<<< HEAD
#ifdef Q_WS_X11
    if (QDBusConnection::systemBus().interface()->isServiceRegistered(QLatin1String("org.freedesktop.login1"))) {
        m_SMBackend = new Login1SMBackend();
    }
    else if (QDBusConnection::systemBus().interface()->isServiceRegistered(QLatin1String("org.freedesktop.ConsoleKit"))) {
        m_SMBackend = new ConsoleKitSMBackend();
    }
    const char *dpy = NULL;
    const char *ctl = NULL;
    // yeah yeah yeah i know this is messy, it'll go away eventually
    if (!(dpy = ::getenv("DISPLAY"))) {
        KDMBackendPrivate *priv = new KDMBackendPrivate(dpy, ctl);
        m_DMBackend = new BasicDMBackend(priv);
        if (!m_SMBackend)
            m_SMBackend = new BasicSMBackend(priv);
    }
    else if ((ctl = ::getenv("DM_CONTROL"))) {
        KDMBackendPrivate *priv = new KDMBackendPrivate(dpy, ctl);
        m_DMBackend = new BasicDMBackend(priv);
        if (!m_SMBackend)
            m_SMBackend = new BasicSMBackend(priv);
    }
    else if ((ctl = ::getenv("XDM_MANAGED")) && ctl[0] == '/') {
        KDMBackendPrivate *priv = new KDMBackendPrivate(dpy, ctl);
        m_DMBackend = new DBusDMBackend(priv);
        if (!m_SMBackend)
            m_SMBackend = new BasicSMBackend(priv);
    }
    else if (::getenv("GDMSESSION")) {
        KDMBackendPrivate *priv = new KDMBackendPrivate(dpy, ctl);
        m_DMBackend = new BasicDMBackend(priv);
        if (!m_SMBackend)
            m_SMBackend = new BasicSMBackend(priv);
=======
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

class GDMFactory : public QDBusInterface
{
public:
    GDMFactory() :
        QDBusInterface(
                QLatin1String("org.gnome.DisplayManager"),
                QLatin1String("/org/gnome/DisplayManager/LocalDisplayFactory"),
                QLatin1String("org.gnome.DisplayManager.LocalDisplayFactory"),
                QDBusConnection::systemBus()) {}
};

class LightDMDBus : public QDBusInterface
{
public:
    LightDMDBus() :
        QDBusInterface(
                QLatin1String("org.freedesktop.DisplayManager"),
                qgetenv("XDG_SEAT_PATH"),
                QLatin1String("org.freedesktop.DisplayManager.Seat"),
                QDBusConnection::systemBus()) {}
};

static enum { Dunno, NoDM, KDM, GDM, LightDM } DMType = Dunno;
static const char *ctl, *dpy;

class KDisplayManager::Private
{
public:
    Private() : fd(-1) {}
    ~Private() {
        if (fd >= 0)
            close(fd);
    }

    int fd;
};

KDisplayManager::KDisplayManager() : d(new Private)
{
    const char *ptr;
    struct sockaddr_un sa;

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
    switch (DMType) {
    default:
        return;
    case KDM:
        if ((d->fd = ::socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
            return;
        sa.sun_family = AF_UNIX;
        if ((ptr = strchr(dpy, ':')))
            ptr = strchr(ptr, '.');
        snprintf(sa.sun_path, sizeof(sa.sun_path),
                    "%s/dmctl-%.*s/socket",
                    ctl, ptr ? int(ptr - dpy) : 512, dpy);
        if (::connect(d->fd, (struct sockaddr *)&sa, sizeof(sa))) {
            ::close(d->fd);
            d->fd = -1;
        }
        break;
    }
}

KDisplayManager::~KDisplayManager()
{
    delete d;
}

bool
KDisplayManager::exec(const char *cmd)
{
    QByteArray buf;

    return exec(cmd, buf);
}

/**
 * Execute a KDM/GDM remote control command.
 * @param cmd the command to execute. FIXME: undocumented yet.
 * @param buf the result buffer.
 * @return result:
 *  @li If true, the command was successfully executed.
 *   @p ret might contain addional results.
 *  @li If false and @p ret is empty, a communication error occurred
 *   (most probably KDM is not running).
 *  @li If false and @p ret is non-empty, it contains the error message
 *   from KDM.
 */
bool
KDisplayManager::exec(const char *cmd, QByteArray &buf)
{
    bool ret = false;
    int tl;
    int len = 0;

    if (d->fd < 0)
        goto busted;

    tl = strlen(cmd);
    if (::write(d->fd, cmd, tl) != tl) {
      bust:
        ::close(d->fd);
        d->fd = -1;
      busted:
        buf.resize(0);
        return false;
    }
    for (;;) {
        if (buf.size() < 128)
            buf.resize(128);
        else if (buf.size() < len * 2)
            buf.resize(len * 2);
        if ((tl = ::read(d->fd, buf.data() + len, buf.size() - len)) <= 0) {
            if (tl < 0 && errno == EINTR)
                continue;
            goto bust;
        }
        len += tl;
        if (buf[len - 1] == '\n') {
            buf[len - 1] = 0;
            if (len > 2 && (buf[0] == 'o' || buf[0] == 'O') &&
                (buf[1] == 'k' || buf[1] == 'K') && buf[2] <= ' ')
                ret = true;
            break;
        }
    }
    return ret;
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
>>>>>>> Remove OldKDM and OldGDM support from KDisplayManager
    }
    else {
        KDMBackendPrivate *priv = new KDMBackendPrivate(dpy, ctl);
        m_DMBackend = new BasicDMBackend(priv);
        if (!m_SMBackend)
            m_SMBackend = new BasicSMBackend(priv);
    }
#else
    m_SMBackend = new NullSMBackend();
    m_DMBackend = new NullDMBackend();
#endif
}

KDisplayManager::~KDisplayManager()
{
}

#ifndef KDM_NO_SHUTDOWN
bool
KDisplayManager::canShutdown()
{
<<<<<<< HEAD
    return m_SMBackend->canShutdown();
=======
    if (DMType == GDM || DMType == NoDM || DMType == LightDM) {
        QDBusReply<QString> canPowerOff = SystemdManager().call(QLatin1String("CanPowerOff"));
        if (canPowerOff.isValid())
            return canPowerOff.value() != QLatin1String("no");
        QDBusReply<bool> canStop = CKManager().call(QLatin1String("CanStop"));
        if (canStop.isValid())
            return canStop.value();
        return false;
    }

    QByteArray re;

    return exec("caps\n", re) && re.indexOf("\tshutdown") >= 0;
>>>>>>> Remove OldKDM and OldGDM support from KDisplayManager
}

void
KDisplayManager::shutdown(KWorkSpace::ShutdownType shutdownType,
                          KWorkSpace::ShutdownMode shutdownMode, /* NOT Default */
                          const QString &bootOption)
{
    if (shutdownType == KWorkSpace::ShutdownTypeNone || shutdownType == KWorkSpace::ShutdownTypeLogout)
        return;
<<<<<<< HEAD
    
    m_SMBackend->shutdown(shutdownType, shutdownMode, bootOption);
=======

    bool cap_ask;
    if (DMType == KDM) {
        QByteArray re;
        cap_ask = exec("caps\n", re) && re.indexOf("\tshutdown ask") >= 0;
    } else {
        if (!bootOption.isEmpty())
            return;

        if (DMType == GDM || DMType == NoDM || DMType == LightDM) {
            // systemd supports only 2 modes:
            // * interactive = true: brings up a PolicyKit prompt if other sessions are active
            // * interactive = false: rejects the shutdown if other sessions are active
            // There are no schedule or force modes.
            // We try to map our 4 shutdown modes in the sanest way.
            bool interactive = (shutdownMode == KWorkSpace::ShutdownModeInteractive
                                || shutdownMode == KWorkSpace::ShutdownModeForceNow);
            QDBusReply<QString> check = SystemdManager().call(QLatin1String(
                    shutdownType == KWorkSpace::ShutdownTypeReboot ? "Reboot" : "PowerOff"), interactive);
            if (!check.isValid()) {
                // FIXME: entirely ignoring shutdownMode
                CKManager().call(QLatin1String(
                        shutdownType == KWorkSpace::ShutdownTypeReboot ? "Restart" : "Stop"));
                // if even CKManager call fails, there is nothing more to be done
            }
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
    exec(cmd.data());
>>>>>>> Remove OldKDM and OldGDM support from KDisplayManager
}

bool
KDisplayManager::bootOptions(QStringList &opts, int &defopt, int &current)
{
<<<<<<< HEAD
    return m_DMBackend->bootOptions(opts, defopt, current);
=======
    if (DMType != KDM)
        return false;

    QByteArray re;
    if (!exec("listbootoptions\n", re))
        return false;

    opts = QString::fromLocal8Bit(re.data()).split('\t', QString::SkipEmptyParts);
    if (opts.size() < 4)
        return false;

    bool ok;
    defopt = opts[2].toInt(&ok);
    if (!ok)
        return false;
    current = opts[3].toInt(&ok);
    if (!ok)
        return false;

    opts = opts[1].split(' ', QString::SkipEmptyParts);
    for (QStringList::Iterator it = opts.begin(); it != opts.end(); ++it)
        (*it).replace("\\s", " ");

    return true;
>>>>>>> Remove OldKDM and OldGDM support from KDisplayManager
}
#endif // KDM_NO_SHUTDOWN

// This only tells KDM to not auto-re-login upon session crash
void
KDisplayManager::setLock(bool on)
{
<<<<<<< HEAD
    m_DMBackend->setLock(on);
=======
    if (DMType == KDM)
        exec(on ? "lock\n" : "unlock\n");
>>>>>>> Remove OldKDM and OldGDM support from KDisplayManager
}

bool
KDisplayManager::isSwitchable()
{
<<<<<<< HEAD
    return m_SMBackend->isSwitchable();
=======
    if (DMType == GDM || DMType == LightDM) {
        QDBusObjectPath currentSeat;
        if (getCurrentSeat(0, &currentSeat)) {
            SystemdSeat SDseat(currentSeat);
            if (SDseat.isValid()) {
                QVariant prop = SDseat.property("CanMultiSession");
                if (prop.isValid())
                    return prop.toBool();
            }
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

    return exec("caps\n", re) && re.indexOf("\tlocal") >= 0;
>>>>>>> Remove OldKDM and OldGDM support from KDisplayManager
}

int
KDisplayManager::numReserve()
{
<<<<<<< HEAD
    return m_DMBackend->numReserve();
=======
    if (DMType == GDM || DMType == LightDM)
        return 1; /* Bleh */

    QByteArray re;
    int p;

    if (!(exec("caps\n", re) && (p = re.indexOf("\treserve ")) >= 0))
        return -1;
    return atoi(re.data() + p + 9);
>>>>>>> Remove OldKDM and OldGDM support from KDisplayManager
}

void
KDisplayManager::startReserve()
{
<<<<<<< HEAD
    m_DMBackend->startReserve();
=======
    if (DMType == GDM)
        GDMFactory().call(QLatin1String("CreateTransientDisplay"));
    else if (DMType == LightDM) {
        LightDMDBus lightDM;
        lightDM.call("SwitchToGreeter");
    }
    else
        exec("reserve\n");
>>>>>>> Remove OldKDM and OldGDM support from KDisplayManager
}

bool
KDisplayManager::localSessions(SessList &list)
{
<<<<<<< HEAD
    return m_SMBackend->localSessions(list);
=======
    if (DMType == GDM || DMType == LightDM) {
        QDBusObjectPath currentSession, currentSeat;
        if (getCurrentSeat(&currentSession, &currentSeat)) {
            // we'll divide the code in two branches to reduce the overhead of calls to non-existent services
            // systemd part // preferred
            if (QDBusConnection::systemBus().interface()->isServiceRegistered(SYSTEMD_SERVICE)) {
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
                        }
                        list.append(se);
                    }
                }
            }
            // ConsoleKit part
            else if (QDBusConnection::systemBus().interface()->isServiceRegistered("org.freedesktop.ConsoleKit")) {
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
    
    if (!exec("list\talllocal\n", re))
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
>>>>>>> Remove OldKDM and OldGDM support from KDisplayManager
}

void
KDisplayManager::sess2Str2(const SessEnt &se, QString &user, QString &loc)
{
    if (se.tty) {
        user = i18nc("user: ...", "%1: TTY login", se.user);
        loc = se.vt ? QString("vt%1").arg(se.vt) : se.display ;
    } else {
        user =
            se.user.isEmpty() ?
                se.session.isEmpty() ?
                    i18nc("... location (TTY or X display)", "Unused") :
                    se.session == "<remote>" ?
                        i18n("X login on remote host") :
                        i18nc("... host", "X login on %1", se.session) :
                se.session == "<unknown>" ?
                    se.user :
                    i18nc("user: session type", "%1: %2",
                          se.user, se.session);
        loc =
            se.vt ?
                QString("%1, vt%2").arg(se.display).arg(se.vt) :
                se.display;
    }
}

QString
KDisplayManager::sess2Str(const SessEnt &se)
{
    QString user, loc;

    sess2Str2(se, user, loc);
    return i18nc("session (location)", "%1 (%2)", user, loc);
}

bool
KDisplayManager::switchVT(int vt)
{
<<<<<<< HEAD
    return m_SMBackend->switchVT(vt);
=======
    if (DMType == GDM || DMType == LightDM) {
        QDBusObjectPath currentSeat;
        if (getCurrentSeat(0, &currentSeat)) {
            // systemd part // preferred
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
            // ConsoleKit part
            else if (QDBusConnection::systemBus().interface()->isServiceRegistered("org.freedesktop.ConsoleKit")) {
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

    return exec(QString("activate\tvt%1\n").arg(vt).toLatin1());
>>>>>>> Remove OldKDM and OldGDM support from KDisplayManager
}

void
KDisplayManager::lockSwitchVT(int vt)
{
    // Lock first, otherwise the lock won't be able to kick in until the session is re-activated.
    QDBusInterface screensaver("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
    screensaver.call("Lock");

    switchVT(vt);
}
