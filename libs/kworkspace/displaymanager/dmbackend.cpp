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

#include "dmbackend.h"

#ifdef Q_WS_X11

#include <QtDBus/QtDBus>
#include <QRegExp>

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

BasicSMBackend::BasicSMBackend ()
: NullSMBackend()
{

}

BasicSMBackend::~BasicSMBackend()
{

}

void
BasicSMBackend::lockSwitchVT (int vt)
{
    // Lock first, otherwise the lock won't be able to kick in until the session is re-activated.
    QDBusInterface screensaver("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
    screensaver.call("Lock");

    switchVT(vt);
}

BasicDMBackend::BasicDMBackend()
: NullDMBackend()
{
    if (!(::getenv("DISPLAY")))
        m_DMType = NoDM;
    else if (::getenv("XDG_SEAT_PATH") && LightDMDBus().isValid())
        m_DMType = LightDM;
    else if (::getenv("GDMSESSION"))
        m_DMType = GDM;
    else
        m_DMType = NoDM;
}

BasicDMBackend::~BasicDMBackend()
{

}

int
BasicDMBackend::numReserve()
{
    return 1; /* Bleh */
}

void
BasicDMBackend::startReserve()
{
    if (m_DMType == GDM)
        GDMFactory().call(QLatin1String("CreateTransientDisplay"));
    else if (m_DMType == LightDM) {
        LightDMDBus lightDM;
        lightDM.call("SwitchToGreeter");
    }
}

BasicSMBackend* BasicDMBackend::provideSM() {
    return new BasicSMBackend();
}

#endif // Q_WS_X11

