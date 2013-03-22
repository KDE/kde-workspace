/*
 * Copyright 2013 Jörg Ehrichs <joerg.ehrichs@gmx.de>
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
 */

#include "nepomukservicejob.h"

#include <KDE/KToolInvocation>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusServiceWatcher>
#include <QtDBus/QDBusReply>

NepomukServiceJob::NepomukServiceJob(const QString &id, const QString &operation, QMap<QString, QVariant> &parameters, QObject *parent)
    : ServiceJob(parent->objectName(), operation, parameters, parent)
    , m_id(id)
{

    m_dBusServer = new QDBusInterface(QLatin1String("org.kde.NepomukServer"),
                                      QLatin1String("/nepomukserver"),
                                      QLatin1String("org.kde.NepomukServer"),
                                      QDBusConnection::sessionBus());

    m_dbusFileIndexer = new QDBusInterface(QLatin1String("org.kde.nepomuk.services.nepomukfileindexer"),
                                           QLatin1String("/nepomukfileindexer"),
                                           QLatin1String("org.kde.nepomuk.FileIndexer"),
                                           QDBusConnection::sessionBus());

    m_serviceFileIndexer = new QDBusInterface(QLatin1String("org.kde.nepomuk.services.nepomukfileindexer"),
                                              QLatin1String("/servicecontrol"),
                                              QLatin1String("org.kde.nepomuk.ServiceControl"),
                                              QDBusConnection::sessionBus());

    m_dBusAkonadiFeeder = new QDBusInterface(QLatin1String("org.freedesktop.Akonadi.Agent.akonadi_nepomuk_feeder"),
                                             QLatin1String("/"),
                                             QLatin1String("org.freedesktop.Akonadi.Agent.Status"),
                                             QDBusConnection::sessionBus());

    m_dbusWebMiner = new QDBusInterface(QLatin1String("org.kde.nepomuk.services.nepomuk-webminerservice"),
                                        QLatin1String("/WebMiner"),
                                        QLatin1String("org.kde.nepomuk.WebMiner"),
                                        QDBusConnection::sessionBus());

    m_serviceWebMiner = new QDBusInterface(QLatin1String("org.kde.nepomuk.services.nepomuk-webminerservice"),
                                           QLatin1String("/servicecontrol"),
                                           QLatin1String("org.kde.nepomuk.ServiceControl"),
                                           QDBusConnection::sessionBus());
}

NepomukServiceJob::~NepomukServiceJob()
{
    delete m_dBusServer;
    delete m_dbusFileIndexer;
    delete m_serviceFileIndexer;
    delete m_dBusAkonadiFeeder;
    delete m_dbusWebMiner;
    delete m_serviceWebMiner;
}

void NepomukServiceJob::start()
{
    if(m_id == QLatin1String("Nepomuk")) {
        if(operationName() == QLatin1String("enableAll")) {
            enableFileIndexer(true);
            enableAkonadiFeeder(true);
            enableWebMiner(true);
        }
        else if(operationName() == QLatin1String("disableAll")) {
            enableFileIndexer(false);
            enableWebMiner(false);
            enableAkonadiFeeder(false);
        }
        else if(operationName() == QLatin1String("suspendAll")) {
            suspendFileIndexer(true);
            suspendAkonadiFeeder(true);
            suspendWebMiner(true);
        }
        else if(operationName() == QLatin1String("resumeAll")) {
            suspendFileIndexer(false);
            suspendWebMiner(false);
            suspendAkonadiFeeder(false);
        }
    }
    else if(m_id == QLatin1String("FileWatch")) {
        if(operationName() == QLatin1String("settings")) {
            openFileWatcherSettings();
        }
        else {
            // the file watcher should not be disabled/suspended
            // it has to watch for moved files and change the metadata accordingly
            setResult(false);
        }
    }
    else if(m_id == QLatin1String("FileIndexer")) {
        if(operationName() == QLatin1String("start")) {
            enableFileIndexer(true);
        }
        else if(operationName() == QLatin1String("stop")) {
            enableFileIndexer(false);
        }
        else if(operationName() == QLatin1String("suspend")) {
            suspendFileIndexer(true);
        }
        else if(operationName() == QLatin1String("resume")) {
            suspendFileIndexer(false);
        }
        else if(operationName() == QLatin1String("settings")) {
            openFileIndexerSettings();
        }
    }
    else if(m_id == QLatin1String("PIM")) {
        if(operationName() == QLatin1String("start")) {
            enableAkonadiFeeder(true);
        }
        else if(operationName() == QLatin1String("stop")) {
            enableAkonadiFeeder(false);
        }
        else if(operationName() == QLatin1String("suspend")) {
            suspendAkonadiFeeder(true);
        }
        else if(operationName() == QLatin1String("resume")) {
            suspendAkonadiFeeder(false);
        }
        else if(operationName() == QLatin1String("settings")) {
            openAkonadiFeederSettings();
        }
    }
    else if(m_id == QLatin1String("WebMiner")) {
        if(operationName() == QLatin1String("start")) {
            enableWebMiner(true);
        }
        else if(operationName() == QLatin1String("stop")) {
            enableWebMiner(false);
        }
        else if(operationName() == QLatin1String("suspend")) {
            suspendWebMiner(true);
        }
        else if(operationName() == QLatin1String("resume")) {
            suspendWebMiner(false);
        }
        else if(operationName() == QLatin1String("settings")) {
            openWebMinerSettings();
        }
    }
}

void NepomukServiceJob::openFileWatcherSettings()
{
    KToolInvocation::kdeinitExec("kcmshell4", QStringList() << "kcm_nepomuk");
    setResult(true);
}

void NepomukServiceJob::openFileIndexerSettings()
{
    KToolInvocation::kdeinitExec("kcmshell4", QStringList() << "kcm_nepomuk");
    setResult(true);
}

void NepomukServiceJob::openAkonadiFeederSettings()
{
    KToolInvocation::kdeinitExec("kcmshell4", QStringList() << "kcm_akonadi");
    setResult(true);
}

void NepomukServiceJob::openWebMinerSettings()
{
    KToolInvocation::kdeinitExec("kcmshell4", QStringList() << "nepomuk-webminer");
    setResult(true);
}

void NepomukServiceJob::suspendFileIndexer(bool suspend)
{
    if(!m_dbusFileIndexer) {
        setResult(false);
        return;
    }

    if ( suspend ) {
        m_dbusFileIndexer->asyncCall("suspend");
    }
    else {
        m_dbusFileIndexer->asyncCall("resume");
    }

    setResult(true);
}

void NepomukServiceJob::enableFileIndexer(bool enable)
{
    if(!m_dBusServer || !m_serviceFileIndexer) {
        setResult(false);
        return;
    }

    if ( enable ) {
        //enable file indexer if not running yet
        m_dBusServer->asyncCall("enableFileIndexer", true);
    }
    else {
        //shutdown the file indexer
        m_serviceFileIndexer->asyncCall("shutdown");
    }

    setResult(true);
}

void NepomukServiceJob::suspendAkonadiFeeder(bool suspend)
{
    enableAkonadiFeeder(!suspend);
}

void NepomukServiceJob::enableAkonadiFeeder(bool enable)
{
    // we do not quit the service, as we are not support to do this
    if(!m_dBusAkonadiFeeder) {
        setResult(false);
        return;
    }

    if( enable ) {
        //enable akonadi feeder
        m_dBusAkonadiFeeder->asyncCall("setOnline", true);
    }
    else {
        //disable akonadi feeder
        m_dBusAkonadiFeeder->asyncCall("setOnline", false);
    }

    setResult(true);
}

void NepomukServiceJob::suspendWebMiner(bool suspend)
{
    if(!m_dbusWebMiner) {
        setResult(false);
        return;
    }

    if ( suspend ) {
        m_dbusWebMiner->asyncCall("suspend");
    }
    else {
        m_dbusWebMiner->asyncCall("resume");
    }

    setResult(true);
}

void NepomukServiceJob::enableWebMiner(bool enable)
{
    if(!m_serviceWebMiner) {
        setResult(false);
        return;
    }

    if ( enable ) {
        //enable webminer if not running yet
        KToolInvocation::kdeinitExec("nepomukservicestub", QStringList() << "nepomuk-webminerservice");
    }
    else {
        //shutdown the webminer
        m_serviceWebMiner->asyncCall("shutdown");
    }
    setResult(true);
}

#include "nepomukservicejob.moc"
