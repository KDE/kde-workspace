/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "daemon.h"

#include <memory>

#include "khotkeysglobal.h"

#include <QtDBus/QtDBus>

#include <KDE/KConfigGroup>
#include <KDE/KConfig>
#include "KDE/KDebug"
#include "KDE/KLocale"


namespace KHotKeys { namespace Daemon {


bool isEnabled()
    {
    KConfig khotkeysrc( KHOTKEYS_CONFIG_FILE );
    KConfigGroup main( &khotkeysrc, "Main" );
    return ! main.readEntry( "Disabled", false );
    }

static QDBusInterface* Kded()
    {
    QDBusInterface *iface = new QDBusInterface( "org.kde.kded5", "/kded","org.kde.kded5" );
    if (!iface->isValid())
        {
        QDBusError err = iface->lastError();
        if (err.isValid())
            {
            kError() << "Failed to contact kded [" << err.name() << "]:" << err.message();
            }
        }
    return iface;
    }


bool isRunning()
    {
    std::auto_ptr<QDBusInterface> kded( Kded() );
    if (!kded->isValid())
        {
        return false;
        }

    // I started with checking if i could get a valid /KHotKeys Interface. But
    // it resisted to work. So lets do the other thing.
    QDBusReply<QStringList> modules = kded->call( "loadedModules" );
    return modules.value().contains("khotkeys");
    }


bool reload()
    {
    // No kded no reload
    std::auto_ptr<QDBusInterface> kded( Kded() );
    if (!kded->isValid())
        {
        return false;
        }

    // Inform kdedkhotkeys demon to reload settings
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusInterface iface(
        "org.kde.kded5",
        "/modules/khotkeys",
        "org.kde.khotkeys",
        bus );
    if(!iface.isValid())
        {
        QDBusError err = iface.lastError();
        if (err.isValid())
            {
            kError() << err.name() << ":" << err.message();
            }
        return start();
        }

    QDBusMessage reply = iface.call("reread_configuration");
    QDBusError err = iface.lastError();
    if (err.isValid())
        {
        kError() << err.name() << ":" << err.message();
        return false;
        }

    return true;
    }


bool start()
    {
    std::auto_ptr<QDBusInterface> kded( Kded() );
    if (!kded->isValid())
        {
        return false;
        }
    QDBusReply<bool> reply = kded->call( "loadModule", "khotkeys"  );
    QDBusError err = reply.error();

    if (err.isValid())
        {
        kError() << "Unable to start server org.kde.khotkeys (kded module) [" 
                 << err.name() << "]:" << err.message();
        return false;
        }

    Q_ASSERT( reply.isValid() );

    if ( reply.value() )
        {
        return true;
        }
    else
        {
        kError() << "Unable to start server org.kde.khotkeys (kded module)";
        return false;
        }
    }


bool stop()
    {
    if (!isRunning())
        {
        return true;
        }

    std::auto_ptr<QDBusInterface> kded( Kded() );
    if (!kded->isValid())
        {
        return false;
        }

    QDBusReply<bool> reply = kded->call( "unloadModule", "khotkeys"  );
    QDBusError err = reply.error();

    if (err.isValid())
        {

        kError() << "Error when stopping khotkeys kded module [" << err.name() << "]:" << err.message();
        return false;
        }

    Q_ASSERT( reply.isValid() );

    if ( reply.value() )
        {
        return true;
        }
    else
        {
        kError() << "Failed to stop server org.kde.khotkeys (kded module)";
        return false;
        }
    }

}} // namespace KHotKeys::Daemon
