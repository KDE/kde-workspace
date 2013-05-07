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

#include "kdmbackends.h"

#include <QStringList>
#include <QX11Info>

#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <X11/Xauth.h>
#include <X11/Xlib.h>

class KDMSocketHelper
{
private:
    int fd;
public:
    KDMSocketHelper();
    ~KDMSocketHelper();
    bool exec(const char *cmd);
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
    bool exec(const char *cmd, QByteArray &buf);
};

KDMSocketHelper::KDMSocketHelper()
: fd(-1)
{
    const char *dpy = ::getenv("DISPLAY");
    const char *ctl = ::getenv("DM_CONTROL");
    const char *ptr;
    struct sockaddr_un sa;

    if (!dpy || !ctl)
        return;

    if ((fd = ::socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        return;
    sa.sun_family = AF_UNIX;
    if ((ptr = strchr(dpy, ':')))
        ptr = strchr(ptr, '.');
    snprintf(sa.sun_path, sizeof(sa.sun_path),
                "%s/dmctl-%.*s/socket",
                ctl, ptr ? int(ptr - dpy) : 512, dpy);
    if (::connect(fd, (struct sockaddr *)&sa, sizeof(sa))) {
        ::close(fd);
        fd = -1;
    }
}

KDMSocketHelper::~KDMSocketHelper()
{
    if (fd >= 0)
        close(fd);
}

bool
KDMSocketHelper::exec ( const char* cmd )
{
    QByteArray buf;
    return exec(cmd, buf);
}

bool
KDMSocketHelper::exec ( const char* cmd, QByteArray& buf )
{
    bool ret = false;
    int tl;
    int len = 0;

    if (fd < 0)
        goto busted;

    tl = strlen(cmd);
    if (::write(fd, cmd, tl) != tl) {
    bust:
        ::close(fd);
        fd = -1;
    busted:
        buf.resize(0);
        return false;
    }
    for (;;) {
        if (buf.size() < 128)
            buf.resize(128);
        else if (buf.size() < len * 2)
            buf.resize(len * 2);
        if ((tl = ::read(fd, buf.data() + len, buf.size() - len)) <= 0) {
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

KDMSMBackend::KDMSMBackend(KDMSocketHelper* helper)
: d(helper)
{

}

KDMSMBackend::~KDMSMBackend()
{

}

bool
KDMSMBackend::canShutdown()
{
    QByteArray re;

    return d->exec("caps\n", re) && re.indexOf("\tshutdown") >= 0;
}

void
KDMSMBackend::shutdown(KWorkSpace::ShutdownType shutdownType, KWorkSpace::ShutdownMode shutdownMode, const QString& bootOption)
{
    bool cap_ask;
    QByteArray re;
    cap_ask = d->exec("caps\n", re) && re.indexOf("\tshutdown ask") >= 0;
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
    d->exec(cmd.data());
}

bool
KDMSMBackend::isSwitchable()
{
    QByteArray re;

    return d->exec("caps\n", re) && re.indexOf("\tlocal") >= 0;
}

bool
KDMSMBackend::localSessions(SessList& list)
{
    QByteArray re;

    if (!d->exec("list\talllocal\n", re))
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
KDMSMBackend::switchVT ( int vt ) {
    return d->exec(QString("activate\tvt%1\n").arg(vt).toLatin1());
}

KDMBackend::KDMBackend() {

}

KDMBackend::~KDMBackend() {

}

void
KDMBackend::setLock(bool on)
{
    d->exec(on ? "lock\n" : "unlock\n");
}

int
KDMBackend::numReserve()
{
    QByteArray re;
    int p;

    if (!(d->exec("caps\n", re) && (p = re.indexOf("\treserve ")) >= 0))
        return -1;
    return atoi(re.data() + p + 9);
}

void
KDMBackend::startReserve()
{
    d->exec("reserve\n");
}

bool
KDMBackend::bootOptions ( QStringList& opts, int& dflt, int& curr )
{
    QByteArray re;
    if (!d->exec("listbootoptions\n", re))
        return false;

    opts = QString::fromLocal8Bit(re.data()).split('\t', QString::SkipEmptyParts);
    if (opts.size() < 4)
        return false;

    bool ok;
    dflt = opts[2].toInt(&ok);
    if (!ok)
        return false;
    curr = opts[3].toInt(&ok);
    if (!ok)
        return false;

    opts = opts[1].split(' ', QString::SkipEmptyParts);
    for (QStringList::Iterator it = opts.begin(); it != opts.end(); ++it)
        (*it).replace("\\s", " ");

    return true;
}

KDMSMBackend* KDMBackend::provideSM() {
    return new KDMSMBackend(d.data());
}

void
KDMBackend::GDMAuthenticate()
{
    FILE *fp;
    const char *dpy, *dnum, *dne;
    int dnl;
    Xauth *xau;

    dpy = DisplayString(QX11Info::display());
    if (!dpy) {
        dpy = ::getenv("DISPLAY");
        if (!dpy)
            return;
    }
    dnum = strchr(dpy, ':') + 1;
    dne = strchr(dpy, '.');
    dnl = dne ? dne - dnum : strlen(dnum);

    /* XXX should do locking */
    if (!(fp = fopen(XauFileName(), "r")))
        return;

    while ((xau = XauReadAuth(fp))) {
        if (xau->family == FamilyLocal &&
            xau->number_length == dnl && !memcmp(xau->number, dnum, dnl) &&
            xau->data_length == 16 &&
            xau->name_length == 18 && !memcmp(xau->name, "MIT-MAGIC-COOKIE-1", 18))
        {
            QString cmd("AUTH_LOCAL ");
            for (int i = 0; i < 16; i++)
                cmd += QString::number((uchar)xau->data[i], 16).rightJustified(2, '0');
            cmd += '\n';
            if (d->exec(cmd.toLatin1())) {
                XauDisposeAuth(xau);
                break;
            }
        }
        XauDisposeAuth(xau);
    }

    fclose (fp);
}
