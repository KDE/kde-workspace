/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public 
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <KLocale>

#include "networkgsminterface.h"
#include "networkgsminterface_p.h"

#include "frontendobject_p.h"
#include "soliddefs_p.h"
#include "ifaces/networkgsminterface.h"
#include "ifaces/modemgsmnetworkinterface.h"

Solid::Control::GsmNetworkInterface::GsmNetworkInterface(QObject *backendObject)
    : SerialNetworkInterface(*new GsmNetworkInterfacePrivate(this), backendObject)
{
    Q_D(GsmNetworkInterface);
    d->setBackendObject(backendObject);
    makeConnections( backendObject );
}

Solid::Control::GsmNetworkInterface::GsmNetworkInterface(const GsmNetworkInterface &networkinterface)
    : SerialNetworkInterface(*new GsmNetworkInterfacePrivate(this), networkinterface)
{
    Q_D(GsmNetworkInterface);
    d->setBackendObject(networkinterface.d_ptr->backendObject());
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::GsmNetworkInterface::GsmNetworkInterface(GsmNetworkInterfacePrivate &dd, QObject *backendObject)
    : SerialNetworkInterface(dd, backendObject)
{
    makeConnections( backendObject );
}

Solid::Control::GsmNetworkInterface::GsmNetworkInterface(GsmNetworkInterfacePrivate &dd, const GsmNetworkInterface &networkinterface)
    : SerialNetworkInterface(dd, networkinterface.d_ptr->backendObject())
{
    makeConnections( networkinterface.d_ptr->backendObject() );
}

Solid::Control::GsmNetworkInterface::~GsmNetworkInterface()
{
}

Solid::Control::NetworkInterface::Type Solid::Control::GsmNetworkInterface::type() const
{
    return Gsm;
}

Solid::Control::ModemGsmNetworkInterface * Solid::Control::GsmNetworkInterface::getModemNetworkIface()
{
    Q_D(const GsmNetworkInterface);
    Ifaces::GsmNetworkInterface *t = qobject_cast<Ifaces::GsmNetworkInterface *>(d->backendObject());
    if (t != 0)
    {
        return t->getModemNetworkIface();
    }
    return 0;
}

Solid::Control::ModemGsmCardInterface * Solid::Control::GsmNetworkInterface::getModemCardIface()
{
    Q_D(const GsmNetworkInterface);
    Ifaces::GsmNetworkInterface *t = qobject_cast<Ifaces::GsmNetworkInterface *>(d->backendObject());
    if (t != 0)
    {
        return t->getModemCardIface();
    }
    return 0;
}

void Solid::Control::GsmNetworkInterface::setModemNetworkIface(Solid::Control::ModemGsmNetworkInterface * iface)
{
    Q_D(const GsmNetworkInterface);
    Ifaces::GsmNetworkInterface *t = qobject_cast<Ifaces::GsmNetworkInterface *>(d->backendObject());
    if (t != 0)
    {
        t->setModemNetworkIface(iface);
    }
}

void Solid::Control::GsmNetworkInterface::setModemCardIface(Solid::Control::ModemGsmCardInterface * iface)
{
    Q_D(const GsmNetworkInterface);
    Ifaces::GsmNetworkInterface *t = qobject_cast<Ifaces::GsmNetworkInterface *>(d->backendObject());
    if (t != 0)
    {
        t->setModemCardIface(iface);
    }
}

void Solid::Control::GsmNetworkInterface::makeConnections(QObject * source)
{
}

void Solid::Control::GsmNetworkInterfacePrivate::setBackendObject(QObject *object)
{
    SerialNetworkInterfacePrivate::setBackendObject(object);
}

void Solid::Control::GsmNetworkInterface::_k_destroyed(QObject *object)
{
    Q_UNUSED(object);
}
// vim: sw=4 sts=4 et tw=100
