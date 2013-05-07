/*
   Copyright (C) 2004,2005 Oswald Buddenhagen <ossi@kde.org>
   Copyright (C) 2005 Stephan Kulow <coolo@kde.org>

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

#ifndef KDMBACKENDS_H
#define KDMBACKENDS_H

#include "dmbackend.h"

class KDMSocketHelper;

class KDMSMBackend : public BasicSMBackend {
public:
    KDMSMBackend(KDMSocketHelper *helper);
    virtual ~KDMSMBackend();
    virtual bool canShutdown();
    virtual void shutdown(KWorkSpace::ShutdownType shutdownType, KWorkSpace::ShutdownMode shutdownMode, const QString &bootOption = QString());
    virtual bool isSwitchable();
    virtual bool localSessions(SessList &list);
    virtual bool switchVT(int vt);
private:
    QSharedPointer<KDMSocketHelper> d;
};

class KDMBackend : public NullDMBackend {
public:
    KDMBackend();
    virtual ~KDMBackend();
    virtual void setLock(bool on);
    virtual int numReserve();
    virtual void startReserve();
    virtual bool bootOptions(QStringList &opts, int &dflt, int &curr);
    virtual KDMSMBackend *provideSM();
private:
    QSharedPointer<KDMSocketHelper> d;
    void GDMAuthenticate();
};

#endif // KDMBACKENDS_H
