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

#include "support.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

KDMBackendPrivate::KDMBackendPrivate(const char *dpy, const char *ctl) 
: fd(-1)
{
    if (!dpy || !ctl)
        return;

    const char *ptr;
    struct sockaddr_un sa;
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

KDMBackendPrivate::~KDMBackendPrivate() 
{
    if (fd >= 0)
        close(fd);
}

bool
KDMBackendPrivate::exec(const char *cmd)
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
KDMBackendPrivate::exec(const char *cmd, QByteArray &buf)
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
