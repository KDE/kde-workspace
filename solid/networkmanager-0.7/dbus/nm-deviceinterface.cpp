/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -N -m -p nm-deviceinterface /space/kde/sources/trunk/KDE/kdebase/workspace/solid/networkmanager-0.7/dbus/introspection/nm-device.xml
 *
 * qdbusxml2cpp is Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "nm-deviceinterface.h"

/*
 * Implementation of interface class OrgFreedesktopNetworkManagerDeviceInterface
 */

OrgFreedesktopNetworkManagerDeviceInterface::OrgFreedesktopNetworkManagerDeviceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OrgFreedesktopNetworkManagerDeviceInterface::~OrgFreedesktopNetworkManagerDeviceInterface()
{
}


#include "nm-deviceinterface.moc"
