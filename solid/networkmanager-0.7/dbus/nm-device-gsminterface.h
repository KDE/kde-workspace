/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -N -m -p nm-device-gsminterface /space/kde/sources/trunk/KDE/kdebase/workspace/solid/networkmanager-0.7/dbus/introspection/nm-device-gsm.xml
 *
 * qdbusxml2cpp is Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef NM_DEVICE_GSMINTERFACE_H_1234457952
#define NM_DEVICE_GSMINTERFACE_H_1234457952

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.freedesktop.NetworkManager.Device.Gsm
 */
class OrgFreedesktopNetworkManagerDeviceGsmInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.NetworkManager.Device.Gsm"; }

public:
    OrgFreedesktopNetworkManagerDeviceGsmInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgFreedesktopNetworkManagerDeviceGsmInterface();

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
    void PropertiesChanged(const QVariantMap &properties);
};

#endif
