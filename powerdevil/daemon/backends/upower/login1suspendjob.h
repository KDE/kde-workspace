/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2010 Alejandro Fiestas <alex@eyeos.org>
    Copyright (C) 2013 Lukáš Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef LOGIN1SUSPENDJOB_H
#define LOGIN1SUSPENDJOB_H

#include <kjob.h>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingCallWatcher>

#include "powerdevilbackendinterface.h"

class Login1SuspendJob : public KJob
{
    Q_OBJECT
public:
    Login1SuspendJob(QDBusInterface *login1Interface,
                     PowerDevil::BackendInterface::SuspendMethod method,
                     PowerDevil::BackendInterface::SuspendMethods supported);
    virtual ~Login1SuspendJob();

    void start();
    void kill(bool quietly);

private Q_SLOTS:
    void doStart();
    void sendResult(QDBusPendingCallWatcher* watcher);
    void slotLogin1Resuming(bool active);

private:
    QDBusInterface *m_login1Interface;
    PowerDevil::BackendInterface::SuspendMethod m_method;
    PowerDevil::BackendInterface::SuspendMethods m_supported;
};

#endif //LOGIN1SUSPENDJOB_H
