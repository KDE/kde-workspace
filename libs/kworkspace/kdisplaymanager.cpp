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
#include "displaymanager/dmbackend.h"
#include "displaymanager/login1smbackend.h"
#include "displaymanager/consolekitsmbackend.h"
#include "displaymanager/kdmbackends.h"

#include <klocale.h>
#include <QtDBus>

NullDMBackend *KDisplayManager::m_DMBackend = NULL;
NullSMBackend *KDisplayManager::m_SMBackend = NULL;

KDisplayManager::KDisplayManager()
{
    if (!m_DMBackend) {
#ifdef Q_WS_X11
        if (QDBusConnection::systemBus().interface()->isServiceRegistered(QLatin1String("org.freedesktop.login1")))
            m_SMBackend = new Login1SMBackend();
        else if (QDBusConnection::systemBus().interface()->isServiceRegistered(QLatin1String("org.freedesktop.ConsoleKit")))
            m_SMBackend = new ConsoleKitSMBackend();

        if (::getenv("DM_CONTROL"))
            m_DMBackend = new KDMBackend();
        else
            m_DMBackend = new BasicDMBackend();

        if (!m_SMBackend)
            m_SMBackend = m_DMBackend->provideSM();
#else
        m_DMBackend = new NullDMBackend();
        m_SMBackend = m_DMBackend->provideSM();
#endif
    }
}

KDisplayManager::~KDisplayManager()
{
    delete m_SMBackend;
    delete m_DMBackend;
}

#ifndef KDM_NO_SHUTDOWN
bool
KDisplayManager::canShutdown()
{
    return m_SMBackend->canShutdown();
}

void
KDisplayManager::shutdown(KWorkSpace::ShutdownType shutdownType,
                          KWorkSpace::ShutdownMode shutdownMode, /* NOT Default */
                          const QString &bootOption)
{
    if (shutdownType == KWorkSpace::ShutdownTypeNone || shutdownType == KWorkSpace::ShutdownTypeLogout)
        return;

    m_SMBackend->shutdown(shutdownType, shutdownMode, bootOption);
}

bool
KDisplayManager::bootOptions(QStringList &opts, int &defopt, int &current)
{
    return m_DMBackend->bootOptions(opts, defopt, current);
}
#endif // KDM_NO_SHUTDOWN

// This only tells KDM to not auto-re-login upon session crash
void
KDisplayManager::setLock(bool on)
{
    m_DMBackend->setLock(on);
}

bool
KDisplayManager::isSwitchable()
{
    return m_SMBackend->isSwitchable();
}

int
KDisplayManager::numReserve()
{
    return m_DMBackend->numReserve();
}

void
KDisplayManager::startReserve()
{
    m_DMBackend->startReserve();
}

bool
KDisplayManager::localSessions(SessList &list)
{
    return m_SMBackend->localSessions(list);
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
    return m_SMBackend->switchVT(vt);
}

void
KDisplayManager::lockSwitchVT(int vt)
{
    m_SMBackend->lockSwitchVT(vt);
}
