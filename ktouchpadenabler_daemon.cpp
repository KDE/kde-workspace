/***************************************************************************
 *   Copyright (C) 2011 Albert Astals Cid <aacid@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "ktouchpadenabler_daemon.h"

#include <KApplication>
#include <KDebug>
#include <KLocalizedString>
#include <KNotification>
#include <KPluginFactory>

#include <QWidget>

#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <X11/XF86keysym.h>

class TouchpadEnablerDaemonPrivate : public QWidget
{
    public:
        TouchpadEnablerDaemonPrivate();
        ~TouchpadEnablerDaemonPrivate();
        
        bool initSuccessful() const { return m_keyCode != 0; }
        
        bool x11Event(XEvent *event);
    
    private:
        enum TouchpadKey { ToggleKey = 0, OnKey, OffKey };
        static const int nKeys = OffKey + 1;
        
        Display *m_display;
        KeyCode m_keyCode[nKeys];
        int m_deviceId;
        Atom m_enabledProperty;
};

TouchpadEnablerDaemonPrivate::TouchpadEnablerDaemonPrivate()
{
    bool foundTouchpad = false;
    bool foundMoreThanOneTouchpad = false;
    
    for (int i = 0; i < nKeys; ++i) {
        m_keyCode[i] = 0;
    }
    
    m_display = QX11Info::display();
    if (!m_display) {
        kWarning() << "Did not find a display to use. This should never happen, thus doing nothing. Please report a bug against ktouchpadenabler in http://bugs.kde.org";
        return;
    }
    
    Atom synapticsProperty = XInternAtom (m_display, "Synaptics Off", False);
    m_enabledProperty = XInternAtom (m_display, "Device Enabled", False);

    if (synapticsProperty && m_enabledProperty) {
        int nDevices;
        XIDeviceInfo *devices = XIQueryDevice(m_display, XIAllDevices, &nDevices);
        for (int i = 0; i < nDevices; ++i) {
            Atom realtype;
            int realformat;
            unsigned long nitems, bytes_after;
            unsigned char *data;
            if ((XIGetProperty (m_display, devices[i].deviceid, synapticsProperty, 0, 1, False, XA_INTEGER, &realtype, &realformat, &nitems, &bytes_after, &data) == Success) && (realtype != None)) {
                XFree (data);
                if ((XIGetProperty (m_display, devices[i].deviceid, m_enabledProperty, 0, 1, False, XA_INTEGER, &realtype, &realformat, &nitems, &bytes_after, &data) == Success) && (realtype != None)) {
                    XFree (data);
                    if (foundTouchpad) {
                        foundMoreThanOneTouchpad = true;
                    } else {
                        m_deviceId = devices[i].deviceid;
                        foundTouchpad = true;
                    }
                }
            }
        }
        XIFreeDeviceInfo(devices);
    } else {
        kWarning() << "Could not get atoms for 'Synaptics Off' or 'Device Enabled'. This should never happen, thus doing nothing. Please report a bug against ktouchpadenabler in http://bugs.kde.org";
    }
    
    if (foundTouchpad) {
        if (!foundMoreThanOneTouchpad) {
            m_keyCode[ToggleKey] = XKeysymToKeycode(m_display, XF86XK_TouchpadToggle);
            m_keyCode[OnKey] = XKeysymToKeycode(m_display, XF86XK_TouchpadOn);
            m_keyCode[OffKey] = XKeysymToKeycode(m_display, XF86XK_TouchpadOff);
            for (int i = 0; i < nKeys; ++i) {
                if (m_keyCode[i] != 0) {
                    const int grabResult = XGrabKey(m_display, m_keyCode[i], 0 /* No modifiers */, QX11Info::appRootWindow(), False, GrabModeAsync, GrabModeAsync);
                    if (grabResult == BadAccess || grabResult == BadValue || grabResult == BadWindow) {
                        kDebug() << "Could not grab ktouchpadenabler key index" << i <<". You probably have some other program grabbig it, if you are sure you don't have any, please report a bug against ktouchpadenabler in http://bugs.kde.org";
                        m_keyCode[i] = 0;
                    }
                } else {
                    kWarning() << "Could not match ktouchpadenabler key index" << i << "to a Keycode. This should never happen. Please report a bug against ktouchpadenabler in http://bugs.kde.org";
                }
            }
        } else {
            KNotification *notification = KNotification::event(KNotification::Warning, i18n("Touchpad status"), i18n("Found more than one touchpad. Touchpad Enabler Daemon does not handle this configuration"));
            notification->sendEvent();
        }
    } else {
        kDebug() << "Did not find a touchpad. If you have one, please report a bug against ktouchpadenabler in http://bugs.kde.org";
    }
}

TouchpadEnablerDaemonPrivate::~TouchpadEnablerDaemonPrivate()
{
    for (int i = 0; i < nKeys; ++i) {
        if (m_keyCode[i] != 0) {
            XUngrabKey(m_display, m_keyCode[i], 0 /* No modifiers */, QX11Info::appRootWindow());
        }
    }
}

bool TouchpadEnablerDaemonPrivate::x11Event(XEvent *event)
{
    if (event->type == KeyPress) {
        for (int i = 0; i < nKeys; ++i) {
            if (event->xkey.keycode == m_keyCode[i]) {
                unsigned char newValue;
            
                Atom realtype;
                int realformat;
                unsigned long nitems, bytes_after;
                unsigned char *currentValue;
                if ((XIGetProperty (m_display, m_deviceId, m_enabledProperty, 0, 1, False, XA_INTEGER, &realtype, &realformat, &nitems, &bytes_after, &currentValue) == Success) && (realtype != None)) {
                    switch (i) {
                        case ToggleKey: newValue = (*currentValue == 0) ? 1 : 0; break;
                        case OnKey:     newValue = 1; break;
                        case OffKey:    newValue = 0; break;
                    }
                    if (newValue != *currentValue) {
                        XIChangeProperty (m_display, m_deviceId, m_enabledProperty, XA_INTEGER, 8, PropModeReplace, &newValue, 1);
                        XFlush(m_display);
                    
                        KNotification *notification = KNotification::event(KNotification::Notification, i18n("Touchpad status"), newValue == 1 ? i18n("Touchpad enabled") : i18n("Touchpad disabled"));
                        notification->sendEvent();
                    }
                    XFree (currentValue);
                    return true;
                }
            }
        }
    }
    return false;
}

TouchpadEnablerDaemon::TouchpadEnablerDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
{
    d = new TouchpadEnablerDaemonPrivate();
    
    if (d->initSuccessful()) {
        kapp->installX11EventFilter(d);
    } else {
        delete d;
        d = 0;
    }
}

TouchpadEnablerDaemon::~TouchpadEnablerDaemon()
{
    delete d;
}

K_PLUGIN_FACTORY(TouchpadEnablerFactory, registerPlugin<TouchpadEnablerDaemon>();)
K_EXPORT_PLUGIN(TouchpadEnablerFactory("ktouchpadenabler"))
