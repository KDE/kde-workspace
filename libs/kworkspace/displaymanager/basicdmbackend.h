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

#ifndef BASICDMBACKEND_H
#define BASICDMBACKEND_H

#include <QByteArray>
#include <QStringList>
class KDMBackendPrivate;

class NullDMBackend {
public:
    NullDMBackend() { }
    virtual ~NullDMBackend() { }
    virtual int numReserve() { return -1; }
    virtual void startReserve() { }
    virtual bool bootOptions(QStringList&, int&, int&) { return false; }
    virtual void setLock(bool) { }
};

class BasicDMBackend : public NullDMBackend {
private:
    KDMBackendPrivate * const d;
    enum { Dunno, NoDM, KDM, GDM } DMType;
public:
    BasicDMBackend( KDMBackendPrivate* p );
    virtual ~BasicDMBackend();
    virtual int numReserve();
    virtual void startReserve();
    virtual bool bootOptions(QStringList &opts, int &dflt, int &curr);
    virtual void setLock(bool on);
};

class DBusDMBackend : public NullDMBackend {
public:
    DBusDMBackend();
    virtual ~DBusDMBackend();
    virtual void startReserve();
};

#endif // BASICDMBACKEND_H
