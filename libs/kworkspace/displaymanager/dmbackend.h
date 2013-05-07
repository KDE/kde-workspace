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

#ifndef DMBACKEND_H
#define DMBACKEND_H

#include "../kworkspace.h"
#include "../kdisplaymanager.h"

#include <QtCore/qstring.h>
#include <QtCore/qsharedpointer.h>

class NullSMBackend {
public:
    NullSMBackend() { }
    virtual ~NullSMBackend() { }
    virtual bool canShutdown() { return false; }
    virtual void shutdown(KWorkSpace::ShutdownType, KWorkSpace::ShutdownMode, const QString&) { }
    virtual bool isSwitchable() { return false; }
    virtual bool localSessions(SessList&) { return false; }
    virtual bool switchVT(int) { return false; }
    virtual void lockSwitchVT(int) { }
};

class NullDMBackend {
public:
    NullDMBackend() { }
    virtual ~NullDMBackend() { }
    virtual void setLock(bool) { }
    virtual int numReserve() { return -1; }
    virtual void startReserve() { }
    virtual bool bootOptions(QStringList&, int&, int&) { return false; }
    virtual NullSMBackend *provideSM() { return new NullSMBackend(); }
};

#ifdef Q_WS_X11
class KDMSocketHelper;

enum DMType { Dunno, NoDM, KDM, GDM, LightDM };

class BasicSMBackend : public NullSMBackend {
public:
    BasicSMBackend(KDMSocketHelper *helper, DMType type);
    virtual ~BasicSMBackend();
    virtual bool canShutdown();
    virtual void shutdown(KWorkSpace::ShutdownType shutdownType, KWorkSpace::ShutdownMode shutdownMode, const QString &bootOption = QString());
    virtual bool isSwitchable();
    virtual bool localSessions(SessList &list);
    virtual bool switchVT(int vt);
    virtual void lockSwitchVT(int vt);
private:
    QSharedPointer<KDMSocketHelper> d;
    DMType m_DMType;
};

class BasicDMBackend : public NullDMBackend {
public:
    BasicDMBackend();
    virtual ~BasicDMBackend();
    virtual void setLock(bool on);
    virtual int numReserve();
    virtual void startReserve();
    virtual bool bootOptions(QStringList &opts, int &dflt, int &curr);
    virtual BasicSMBackend *provideSM();
private:
    QSharedPointer<KDMSocketHelper> d;
    DMType m_DMType;
    void GDMAuthenticate();
};
#endif // Q_WS_X11
#endif // DMBACKEND_H
