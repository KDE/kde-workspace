/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>
    Copyright (C) 2008 Tom Patzig <tpatzig@suse.de>


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

#include <QMap>
#include <QPair>
#include <QStringList>
#include <QVariant>

#include <KDebug>

#include "ifaces/bluetoothinterface.h"

#include "frontendobject_p.h"

#include "soliddefs_p.h"
#include "bluetoothmanager.h"
#include "bluetoothinterface.h"
#include "bluetoothremotedevice.h"

namespace Solid
{
namespace Control
{
class BluetoothInterfacePrivate : public FrontendObjectPrivate
{
public:
    BluetoothInterfacePrivate(QObject *parent)
        : FrontendObjectPrivate(parent) { }

    void setBackendObject(QObject *object);

    QPair<BluetoothRemoteDevice *, Ifaces::BluetoothRemoteDevice *> findRegisteredBluetoothRemoteDevice(const QString &ubi) const;
    QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> findRegisteredBluetoothInputDevice(const QString &ubi) const;

    mutable QMap<QString, QPair<BluetoothRemoteDevice *, Ifaces::BluetoothRemoteDevice *> > remoteDeviceMap;
    mutable QMap<QString, QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> > inputDeviceMap;
    mutable BluetoothRemoteDevice invalidDevice;
    mutable BluetoothInputDevice invalidInputDevice;
};
}
}

Solid::Control::BluetoothInterface::BluetoothInterface()
        : QObject(), d(new BluetoothInterfacePrivate(this))
{}

Solid::Control::BluetoothInterface::BluetoothInterface(const QString &ubi)
        : QObject(), d(new BluetoothInterfacePrivate(this))
{
    const BluetoothInterface &device = BluetoothManager::self().findBluetoothInterface(ubi);
    d->setBackendObject(device.d->backendObject());
    QObject::connect(d->backendObject(), SIGNAL(deviceCreated(const QString &)),this, SLOT(slotDeviceCreated(const QString &)));
}

Solid::Control::BluetoothInterface::BluetoothInterface(QObject *backendObject)
        : QObject(), d(new BluetoothInterfacePrivate(this))
{
    d->setBackendObject(backendObject);
    QObject::connect(d->backendObject(), SIGNAL(deviceCreated(const QString &)),this, SLOT(slotDeviceCreated(const QString &)));
}

Solid::Control::BluetoothInterface::BluetoothInterface(const BluetoothInterface &device)
        : QObject(), d(new BluetoothInterfacePrivate(this))
{
    d->setBackendObject(device.d->backendObject());
    QObject::connect(d->backendObject(), SIGNAL(deviceCreated(const QString &)),this, SLOT(slotDeviceCreated(const QString &)));
}

Solid::Control::BluetoothInterface::~BluetoothInterface()
{
    // Delete all the interfaces, they are now outdated
    typedef QPair<BluetoothRemoteDevice *, Ifaces::BluetoothRemoteDevice *> BluetoothRemoteDeviceIfacePair;
    typedef QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> BluetoothInputDeviceIfacePair;

    // Delete all the devices, they are now outdated
    foreach (const BluetoothRemoteDeviceIfacePair &pair, d->remoteDeviceMap.values()) {
        delete pair.first;
        delete pair.second;
    }
    foreach (const BluetoothInputDeviceIfacePair &pair, d->inputDeviceMap.values()) {
        delete pair.first;
        delete pair.second;
    }

}

Solid::Control::BluetoothInterface &Solid::Control::BluetoothInterface::operator=(const Solid::Control::BluetoothInterface  & dev)
{
    d->setBackendObject(dev.d->backendObject());

    return *this;
}

QString Solid::Control::BluetoothInterface::ubi() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QString(), ubi());
}

void Solid::Control::BluetoothInterface::cancelDeviceCreation(const QString &address) const
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), cancelDeviceCreation(address));
}

void Solid::Control::BluetoothInterface::createPairedDevice(const QString &address,const QString &adapterPath, const QString &capab) const
{
    Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface *>(d->backendObject());
    if (backend == 0) 
        return;
    backend->createPairedDevice(address,adapterPath,capab);
}

QMap<QString, QVariant> Solid::Control::BluetoothInterface::getProperties() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), (QMap<QString,QVariant>()), getProperties());
}

QVariant Solid::Control::BluetoothInterface::getProperty(const QString &key) const
{
    QMap<QString, QVariant> props = getProperties();
    if (props.contains(key))
        return props[key];
    else
        return QVariant();
}

Solid::Control::BluetoothRemoteDeviceList Solid::Control::BluetoothInterface::listDevices() const
{
    Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface *>(d->backendObject());
    if (backend == 0) 
        return Solid::Control::BluetoothRemoteDeviceList();

    QStringList ubis = backend->listDevices();

    Solid::Control::BluetoothRemoteDeviceList list;
    foreach (const QString& ubi,ubis) {
        BluetoothRemoteDevice* remoteDevice = findBluetoothRemoteDeviceUBI(ubi);
        list.append(remoteDevice);
    }
    return list;
}

void Solid::Control::BluetoothInterface::registerAgent(const QString &path, const QString &capab) const
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), registerAgent(path,capab));
}

void Solid::Control::BluetoothInterface::releaseSession() const
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), releaseSession());
}

void Solid::Control::BluetoothInterface::requestSession() const
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), requestSession());
}

void Solid::Control::BluetoothInterface::removeDevice(const QString &path) const
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), removeDevice(path));
}

void Solid::Control::BluetoothInterface::setProperty(const QString &property, const QVariant &value) const
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), setProperty(property,value));
}

void Solid::Control::BluetoothInterface::startDiscovery() const
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), startDiscovery());
}

void Solid::Control::BluetoothInterface::stopDiscovery() const
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), stopDiscovery());
}

void Solid::Control::BluetoothInterface::unregisterAgent(const QString &path) const
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), unregisterAgent(path));
}


Solid::Control::BluetoothRemoteDevice Solid::Control::BluetoothInterface::findBluetoothRemoteDeviceAddr(const QString &address) const
{
    Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface *>(d->backendObject());
    if (backend == 0) 
        return d->invalidDevice;

    const QString ubi = getBluetoothRemoteDeviceUBI(address);

    QPair<BluetoothRemoteDevice *, Ifaces::BluetoothRemoteDevice *> pair = d->findRegisteredBluetoothRemoteDevice(ubi);

    if (pair.first != 0) {
        return *pair.first;
    } else {
        return d->invalidDevice;
    }
}


const QString Solid::Control::BluetoothInterface::getBluetoothRemoteDeviceUBI(const QString &address) const
{
    Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface *>(d->backendObject());
    if (backend == 0) 
        return "";

    const QString ubi = backend->findDevice(address);
    return ubi;
}


void Solid::Control::BluetoothInterface::createBluetoothRemoteDevice(const QString &address)
{
    Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface *>(d->backendObject());
    if (backend == 0) 
        return;
    backend->createDevice(address);
}


Solid::Control::BluetoothRemoteDevice* Solid::Control::BluetoothInterface::findBluetoothRemoteDeviceUBI(const QString &ubi) const
{
    QPair<BluetoothRemoteDevice *, Ifaces::BluetoothRemoteDevice *> pair = d->findRegisteredBluetoothRemoteDevice(ubi);
    return pair.first;
}

Solid::Control::BluetoothInputDevice* Solid::Control::BluetoothInterface::findBluetoothInputDeviceUBI(const QString &ubi) const
{
    QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> pair = d->findRegisteredBluetoothInputDevice(ubi);
    return pair.first;
}

void Solid::Control::BluetoothInterface::slotDeviceCreated(const QString& ubi)
{

    Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface *>(d->backendObject());
    Ifaces::BluetoothRemoteDevice *iface = 0;

    if (backend != 0) {
        iface = qobject_cast<Ifaces::BluetoothRemoteDevice *>(backend->createBluetoothRemoteDevice(ubi));
    }

    if (iface != 0) {
        BluetoothRemoteDevice *device = new BluetoothRemoteDevice(iface);

        QPair<BluetoothRemoteDevice *, Ifaces::BluetoothRemoteDevice *> pair(device, iface);
        d->remoteDeviceMap[ubi] = pair;
    }
}

QString Solid::Control::BluetoothInterface::address() const
{
    QVariant var = getProperty("Address");

    if (var.isValid())
        return var.value<QString>();
    else
        return QString();

//    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QString(), address());
}

/*
QString Solid::Control::BluetoothInterface::version() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QString(), version());
}

QString Solid::Control::BluetoothInterface::revision() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QString(), revision());
}

QString Solid::Control::BluetoothInterface::manufacturer() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QString(), manufacturer());
}

QString Solid::Control::BluetoothInterface::company() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QString(), company());
}
*/


int Solid::Control::BluetoothInterface::discoverableTimeout() const
{
    QVariant var = getProperty("DiscoverableTimeout");

    if (var.isValid())
        return var.value<int>();
    else
        return 0;

//  return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), 0, discoverableTimeout());
}

bool Solid::Control::BluetoothInterface::isDiscoverable() const
{
    QVariant var = getProperty("Discoverable");

    if (var.isValid())
        return var.value<bool>();
    else
        return false;

//   return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), false, isDiscoverable());
}

bool Solid::Control::BluetoothInterface::isDiscovering() const
{
    QVariant var = getProperty("Discovering");

    if (var.isValid())
        return var.value<bool>();
    else
        return false;
}

/*
Solid::Control::BluetoothRemoteDeviceList Solid::Control::BluetoothInterface::listConnections() const
{
    BluetoothRemoteDeviceList list;
    Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface *>(d->backendObject());

    if (backend == 0) return list;

    QStringList ubis = backend->listConnections();

    foreach (const QString &ubi, ubis) {
        BluetoothRemoteDevice remoteDevice = findBluetoothRemoteDevice(ubi);
        list.append(remoteDevice);
    }

    return list;
}

QString Solid::Control::BluetoothInterface::majorClass() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QString(), majorClass());
}

QStringList Solid::Control::BluetoothInterface::listAvailableMinorClasses() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QStringList(), listAvailableMinorClasses());
}

QString Solid::Control::BluetoothInterface::minorClass() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QString(), minorClass());
}

QStringList Solid::Control::BluetoothInterface::serviceClasses() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QStringList(), serviceClasses());
}
*/

QString Solid::Control::BluetoothInterface::name() const
{
    QVariant var = getProperty("Name");

    if (var.isValid())
        return var.value<QString>();
    else
        return QString();

//    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QString(), name());
}

/*
QString Solid::Control::BluetoothInterface::getRemoteName(const QString &mac)
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QString(), getRemoteName(mac));
}

QStringList Solid::Control::BluetoothInterface::listBondings() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QStringList(), listBondings());
}

bool Solid::Control::BluetoothInterface::isPeriodicDiscoveryActive() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), false, isPeriodicDiscoveryActive());
}

bool Solid::Control::BluetoothInterface::isPeriodicDiscoveryNameResolvingActive() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), false,
                      isPeriodicDiscoveryNameResolvingActive());
}

bool  Solid::Control::BluetoothInterface::isTrusted(const QString& address)
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), false, isTrusted(address));
}

// TODO: QStringList or BluetoothRemoteDeviceList?
QStringList Solid::Control::BluetoothInterface::listRemoteDevices() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QStringList(),
                      listRemoteDevices());
}

// TODO: QStringList or BluetoothRemoteDeviceList?
QStringList Solid::Control::BluetoothInterface::listRecentRemoteDevices(const QDateTime &date) const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), QStringList(),
                      listRecentRemoteDevices(date));
}

*/

/***************************************************************/

void Solid::Control::BluetoothInterface::setDiscoverable(bool status)
{
    QVariant var(status);
    setProperty("Discoverable",var);
}

void Solid::Control::BluetoothInterface::setDiscoverableTimeout(int timeout)
{
    QVariant var(timeout);
    setProperty("DiscoverableTimeout",var);
}

/*
void Solid::Control::BluetoothInterface::setMinorClass(const QString &minor)
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), setMinorClass(minor));
}
*/
void Solid::Control::BluetoothInterface::setName(const QString &name)
{
    QVariant var(name);
    setProperty("Name",var);
}
/*
void Solid::Control::BluetoothInterface::discoverDevices()
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), discoverDevices());
}

void Solid::Control::BluetoothInterface::discoverDevicesWithoutNameResolving()
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), discoverDevicesWithoutNameResolving());
}

void Solid::Control::BluetoothInterface::cancelDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), cancelDiscovery());
}

void Solid::Control::BluetoothInterface::startPeriodicDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), startPeriodicDiscovery());
}

void Solid::Control::BluetoothInterface::stopPeriodicDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), stopPeriodicDiscovery());
}

void Solid::Control::BluetoothInterface::setPeriodicDiscoveryNameResolving(bool resolveNames)
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), setPeriodicDiscoveryNameResolving(resolveNames));
}

void Solid::Control::BluetoothInterface::setTrusted(const QString& address)
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), setTrusted(address));
}

void Solid::Control::BluetoothInterface::removeTrust(const QString& address)
{
    SOLID_CALL(Ifaces::BluetoothInterface *, d->backendObject(), removeTrust(address));
}

*/

void Solid::Control::BluetoothInterfacePrivate::setBackendObject(QObject *object)
{
    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
/*        
        QObject::connect(object, SIGNAL(modeChanged(Solid::Control::BluetoothInterface::Mode)),parent(), SIGNAL(modeChanged(Solid::Control::BluetoothInterface::Mode)));
        QObject::connect(object, SIGNAL(discoverableTimeoutChanged(int)),
                         parent(), SIGNAL(discoverableTimeoutChanged(int)));
        QObject::connect(object, SIGNAL(minorClassChanged(const QString &)),
                         parent(), SIGNAL(minorClassChanged(const QString &)));
        QObject::connect(object, SIGNAL(nameChanged(const QString &)),
                         parent(), SIGNAL(nameChanged(const QString &)));
        QObject::connect(object, SIGNAL(discoveryStarted()),
                         parent(), SIGNAL(discoveryStarted()));
        QObject::connect(object, SIGNAL(discoveryCompleted()),
                         parent(), SIGNAL(discoveryCompleted()));
        QObject::connect(object, SIGNAL(remoteDeviceFound(const QString &, int, int)),
                         parent(), SIGNAL(remoteDeviceFound(const QString &, int, int)));
        QObject::connect(object, SIGNAL(remoteDeviceDisappeared(const QString &)),
                         parent(), SIGNAL(remoteDeviceDisappeared(const QString &)));
        QObject::connect(object, SIGNAL(remoteNameUpdated(const QString &,const QString &)),
                         parent(), SIGNAL(remoteNameUpdated(const QString &,const QString &)));
        QObject::connect(object, SIGNAL(remoteDeviceConnected(const QString &)),
                         parent(), SIGNAL(remoteDeviceConnected(const QString &)));
        QObject::connect(object, SIGNAL(remoteDeviceDisconnected(const QString &)),
                         parent(), SIGNAL(remoteDeviceDisconnected(const QString &)));
        QObject::connect(object, SIGNAL(trustAdded(const QString &)),
                         parent(), SIGNAL(trustAdded(const QString &)));
        QObject::connect(object, SIGNAL(trustRemoved(const QString &)),
                         parent(), SIGNAL(trustRemoved(const QString &)));
        QObject::connect(object, SIGNAL(bondingCreated(const QString &)),
                         parent(), SIGNAL(bondingCreated(const QString &)));
        QObject::connect(object, SIGNAL(bondingRemoved(const QString &)),
                         parent(), SIGNAL(bondingRemoved(const QString &)));
*/
        QObject::connect(object, SIGNAL(deviceCreated(const QString &)),
                                 parent(), SIGNAL(deviceCreated(const QString &)));
        QObject::connect(object, SIGNAL(deviceDisappeared(const QString &)),
                                 parent(), SIGNAL(deviceDisappeared(const QString &)));
        QObject::connect(object, SIGNAL(deviceFound(const QString &, const QMap<QString,QVariant> &)),
                                 parent(), SIGNAL(deviceFound(const QString &, const QMap<QString,QVariant> &)));
        QObject::connect(object, SIGNAL(deviceRemoved(const QString &)),
                                 parent(), SIGNAL(deviceRemoved(const QString &)));
        QObject::connect(object, SIGNAL(propertyChanged(const QString &,const QVariant &)),
                                 parent(), SIGNAL(propertyChanged(const QString &, const QVariant &)));

    }
}

QPair<Solid::Control::BluetoothRemoteDevice *, Solid::Control::Ifaces::BluetoothRemoteDevice *> Solid::Control::BluetoothInterfacePrivate::findRegisteredBluetoothRemoteDevice(const QString &ubi) const
{
    if (remoteDeviceMap.contains(ubi)) {
        return remoteDeviceMap[ubi];
    } else {
        Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface *>(backendObject());
        Ifaces::BluetoothRemoteDevice *iface = 0;

        if (backend != 0) {
            iface = qobject_cast<Ifaces::BluetoothRemoteDevice *>(backend->createBluetoothRemoteDevice(ubi));
        }

        if (iface != 0) {
            BluetoothRemoteDevice *device = new BluetoothRemoteDevice(iface);

            QPair<BluetoothRemoteDevice *, Ifaces::BluetoothRemoteDevice *> pair(device, iface);
            remoteDeviceMap[ubi] = pair;

            return pair;
        } else {
            return QPair<BluetoothRemoteDevice *, Ifaces::BluetoothRemoteDevice *>(0, 0);
        }

    }
}

QPair<Solid::Control::BluetoothInputDevice *, Solid::Control::Ifaces::BluetoothInputDevice *> Solid::Control::BluetoothInterfacePrivate::findRegisteredBluetoothInputDevice(const QString &ubi) const
{
    if (inputDeviceMap.contains(ubi)) {
        return inputDeviceMap[ubi];
    } else {
        Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface *>(backendObject());
        Ifaces::BluetoothInputDevice *iface = 0;

        if (backend != 0) {
            iface = qobject_cast<Ifaces::BluetoothInputDevice *>(backend->createBluetoothInputDevice(ubi));
        }

        if (iface != 0) {
            BluetoothInputDevice *device = new BluetoothInputDevice(iface);

            QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *> pair(device, iface);
            inputDeviceMap[ubi] = pair;

            return pair;
        } else {
            return QPair<BluetoothInputDevice *, Ifaces::BluetoothInputDevice *>(0, 0);
        }

    }
}

#include "bluetoothinterface.moc"
