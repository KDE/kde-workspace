/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>

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
// own
#include "client_machine.h"
// KWin
#include "utils.h"
// KDE
#include <KDE/KDebug>
// Qt
#include <QtConcurrentRun>
#include <QFutureWatcher>
// system
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace KWin {

static QByteArray getHostName()
{
#ifdef HOST_NAME_MAX
    char hostnamebuf[HOST_NAME_MAX];
#else
    char hostnamebuf[256];
#endif
    if (gethostname(hostnamebuf, sizeof hostnamebuf) >= 0) {
        hostnamebuf[sizeof(hostnamebuf)-1] = 0;
        return QByteArray(hostnamebuf);
    }
    return QByteArray();
}

GetAddrInfo::GetAddrInfo(const QByteArray &hostName, QObject *parent)
    : QObject(parent)
    , m_resolving(false)
    , m_resolved(false)
    , m_ownResolved(false)
    , m_hostName(hostName)
    , m_addressHints(new addrinfo)
    , m_address(NULL)
    , m_ownAddress(NULL)
    , m_watcher(new QFutureWatcher<int>(this))
    , m_ownAddressWatcher(new QFutureWatcher<int>(this))
{
    // watcher will be deleted together with the GetAddrInfo once the future
    // got canceled or finished
    connect(m_watcher, SIGNAL(canceled()), SLOT(deleteLater()));
    connect(m_watcher, SIGNAL(finished()), SLOT(slotResolved()));
    connect(m_ownAddressWatcher, SIGNAL(canceled()), SLOT(deleteLater()));
    connect(m_ownAddressWatcher, SIGNAL(finished()), SLOT(slotOwnAddressResolved()));
}

GetAddrInfo::~GetAddrInfo()
{
    if (m_watcher && m_watcher->isRunning()) {
        m_watcher->cancel();
    }
    if (m_ownAddressWatcher && m_ownAddressWatcher->isRunning()) {
        m_ownAddressWatcher->cancel();
    }
    if (m_address) {
        freeaddrinfo(m_address);
    }
    if (m_ownAddress) {
        freeaddrinfo(m_ownAddress);
    }
    delete m_addressHints;
}

void GetAddrInfo::resolve()
{
    if (m_resolving) {
        return;
    }
    m_resolving = true;
    memset(m_addressHints, 0, sizeof(*m_addressHints));
    m_addressHints->ai_family = PF_UNSPEC;
    m_addressHints->ai_socktype = SOCK_STREAM;
    m_addressHints->ai_flags |= AI_CANONNAME;

    // TODO: C++11 nullptr
    const char* nullPtr = NULL;
    m_watcher->setFuture(QtConcurrent::run(getaddrinfo, m_hostName, nullPtr, m_addressHints, &m_address));
    m_ownAddressWatcher->setFuture(QtConcurrent::run(getaddrinfo, getHostName(), nullPtr, m_addressHints, &m_ownAddress));
}

void GetAddrInfo::slotResolved()
{
    if (resolved(m_watcher)) {
        m_resolved = true;
        compare();
    }
}

void GetAddrInfo::slotOwnAddressResolved()
{
    if (resolved(m_ownAddressWatcher)) {
        m_ownResolved = true;
        compare();
    }
}

bool GetAddrInfo::resolved(QFutureWatcher< int >* watcher)
{
    if (!watcher->isFinished()) {
        return false;
    }
    if (watcher->result() != 0) {
        kDebug(1212) << "getaddrinfo failed with error:" << gai_strerror(watcher->result());
        // call failed;
        deleteLater();
        return false;
    }
    return true;
}

void GetAddrInfo::compare()
{
    if (!m_resolved || !m_ownResolved) {
        return;
    }
    addrinfo *address = m_address;
    while (address) {
        if (address->ai_canonname && m_hostName == QByteArray(address->ai_canonname).toLower()) {
            addrinfo *ownAddress = m_ownAddress;
            bool localFound = false;
            while (ownAddress) {
                if (ownAddress->ai_canonname  && QByteArray(ownAddress->ai_canonname).toLower() == m_hostName) {
                    localFound = true;
                    break;
                }
                ownAddress = ownAddress->ai_next;
            }
            if (localFound) {
                emit local();
                break;
            }
        }
        address = address->ai_next;
    }
    deleteLater();
}


ClientMachine::ClientMachine(QObject *parent)
    : QObject(parent)
    , m_localhost(false)
    , m_resolved(false)
    , m_resolving(false)
{
}

ClientMachine::~ClientMachine()
{
}

void ClientMachine::resolve(xcb_window_t window, xcb_window_t clientLeader)
{
    if (m_resolved) {
        return;
    }
    QByteArray name = getStringProperty(window, XCB_ATOM_WM_CLIENT_MACHINE);
    if (name.isEmpty() && clientLeader && clientLeader != window) {
        name = getStringProperty(clientLeader, XCB_ATOM_WM_CLIENT_MACHINE);
    }
    if (name.isEmpty()) {
        name = localhost();
    }
    if (name == localhost()) {
        setLocal();
    }
    m_hostName = name;
    checkForLocalhost();
    m_resolved = true;
}

void ClientMachine::checkForLocalhost()
{
    if (isLocal()) {
        // nothing to do
        return;
    }
    QByteArray host = getHostName();

    if (!host.isEmpty()) {
        host = host.toLower();
        const QByteArray lowerHostName(m_hostName.toLower());
        if (host == lowerHostName) {
            setLocal();
            return;
        }
        if (char *dot = strchr(host.data(), '.')) {
            *dot = '\0';
            if (host == lowerHostName) {
                setLocal();
                return;
            }
        } else {
            m_resolving = true;
            // check using information from get addr info
            // GetAddrInfo gets automatically destroyed once it finished or not
            GetAddrInfo *info = new GetAddrInfo(lowerHostName, this);
            connect(info, SIGNAL(local()), SLOT(setLocal()));
            connect(info, SIGNAL(destroyed(QObject*)), SLOT(resolveFinished()));
            info->resolve();
        }
    }
}

void ClientMachine::setLocal()
{
    m_localhost = true;
    emit localhostChanged();
}

void ClientMachine::resolveFinished()
{
    m_resolving = false;
}

} // namespace
