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

#include "basicdmbackend.h"
#include "support.h"

#include <QDBusInterface>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

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


BasicDMBackend::BasicDMBackend(KDMBackendPrivate *p)
: d(p)
, DMType(Dunno)
{
    const char *dpy;
    const char *ctl;
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
    }
}

BasicDMBackend::~BasicDMBackend()
{
}

int
BasicDMBackend::numReserve()
{
    if (DMType == GDM || DMType == LightDM)
        return 1; /* Bleh */

    QByteArray re;
    int p = 0;

    if (d && !(d->exec("caps\n", re) && (p = re.indexOf("\treserve ")) >= 0))
        return -1;
    return atoi(re.data() + p + 9);
}

void
BasicDMBackend::startReserve()
{
    if (DMType == GDM)
        GDMFactory().call(QLatin1String("CreateTransientDisplay"));
    else if (DMType == LightDM) {
        LightDMDBus lightDM;
        lightDM.call("SwitchToGreeter");
    }
    else if (d)
        d->exec("reserve\n");
}

