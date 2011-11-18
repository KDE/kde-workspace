/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "interface.h"
#include "ksldapp.h"
#include "screensaveradaptor.h"
// Qt
#include <QtDBus/QDBusConnection>

namespace ScreenLocker
{
Interface::Interface(KSldApp *parent)
    : QObject(parent)
    , m_daemon(parent)
{
    (void) new ScreenSaverAdaptor( this );
    QDBusConnection::sessionBus().registerService(QLatin1String("org.freedesktop.ScreenSaver")) ;
    QDBusConnection::sessionBus().registerObject(QLatin1String("/ScreenSaver"), this);
    connect(m_daemon, SIGNAL(locked()), SLOT(slotLocked()));
    connect(m_daemon, SIGNAL(unlocked()), SLOT(slotUnlocked()));
}

Interface::~Interface()
{
}

bool Interface::GetActive()
{
    return m_daemon->isLocked();
}

uint Interface::GetActiveTime()
{
    return m_daemon->activeTime();
}

uint Interface::GetSessionIdleTime()
{
    // TODO: implement me
    return 0;
}

void Interface::Lock()
{
    m_daemon->lock();
}

bool Interface::SetActive (bool state)
{
    // TODO: what should the return value be?
    if (state) {
        Lock();
        return true;
    }
    // set inactive is ignored
    return false;
}

uint Interface::Inhibit(const QString &application_name, const QString &reason_for_inhibit)
{
    Q_UNUSED(application_name)
    Q_UNUSED(reason_for_inhibit)
    // TODO: implement me, makes only sense when we have the idle support
    return 0;
}

void Interface::UnInhibit(uint cookie)
{
    Q_UNUSED(cookie)
    // TODO: implement me
}

void Interface::SimulateUserActivity()
{
    // TODO: implement me when we support user activity interaction or the autolock
}

uint Interface::Throttle(const QString &application_name, const QString &reason_for_inhibit)
{
    Q_UNUSED(application_name)
    Q_UNUSED(reason_for_inhibit)
    // TODO: implement me
    return 0;
}

void Interface::UnThrottle(uint cookie)
{
    Q_UNUSED(cookie)
    // TODO: implement me
}

void Interface::slotLocked()
{
    emit ActiveChanged(true);
}

void Interface::slotUnlocked()
{
    emit ActiveChanged(false);
}

} // namespace

#include "interface.moc"
