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

class NullDMBackend {
public:
    NullDMBackend() { }
    virtual ~NullDMBackend() { }
    virtual bool canShutdown() { return false; }
    virtual void shutdown(KWorkSpace::ShutdownType, KWorkSpace::ShutdownMode, const QString&) { }
    virtual void setLock(bool) { }
    virtual bool isSwitchable() { return false; }
    virtual int numReserve() { return -1; }
    virtual void startReserve() { }
    virtual bool localSessions(SessList&) { return false; }
    virtual bool switchVT(int) { return false; }
    virtual void lockSwitchVT(int) { }
    virtual bool bootOptions(QStringList&, int&, int&) { return false; }
};

#ifdef Q_WS_X11
class BasicDMBackend : public NullDMBackend {
private:
    class Private;
    Private *d;
    enum { Dunno, NoDM, KDM, GDM, LightDM } DMType;

    void GDMAuthenticate();
public:
    BasicDMBackend();
    virtual ~BasicDMBackend();
    virtual bool canShutdown();
    virtual void shutdown(KWorkSpace::ShutdownType shutdownType, KWorkSpace::ShutdownMode shutdownMode, const QString &bootOption = QString());
    virtual void setLock(bool on);
    virtual bool isSwitchable();
    virtual int numReserve();
    virtual void startReserve();
    virtual bool localSessions(SessList &list);
    virtual bool switchVT(int vt);
    virtual void lockSwitchVT(int vt);
    virtual bool bootOptions(QStringList &opts, int &dflt, int &curr);
};
#endif // Q_WS_X11
#endif // DMBACKEND_H
