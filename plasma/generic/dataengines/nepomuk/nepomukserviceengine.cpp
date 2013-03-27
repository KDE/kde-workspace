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

#include "nepomukserviceengine.h"
#include "nepomukserviceservice.h"

#include <Plasma/DataContainer>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusServiceWatcher>
#include <QtDBus/QDBusReply>

const QLatin1String SERVICE_NEPOMUK = QLatin1String("Nepomuk");           /**< Source for Nepomuk in general */
const QLatin1String SERVICE_FILEWATCH = QLatin1String("FileWatch");       /**< Service for the FileWatch */
const QLatin1String SERVICE_FILEINDEXER = QLatin1String("FileIndexer");   /**< Service for the FileIndexer */
const QLatin1String SERVICE_WEBMINER = QLatin1String("WebMiner");         /**< Service for the WebMiner */
const QLatin1String SERVICE_PIM = QLatin1String("PIM");                   /**< Service for the Akonadi_nepomuk feeder */

const QLatin1String DATA_I18NNAME = QLatin1String("i18nName");            /**< Translated name of the service */
const QLatin1String DATA_STATUSMSG = QLatin1String("statusMessage");      /**< Translated status string from the service */
const QLatin1String DATA_STATUSENUM = QLatin1String("status");            /**< Integer giving detailed status information @sse ServiceStatus*/

const QLatin1String DATA_CANBESUSPENDED= QLatin1String("canBeSuspended"); /**< If the service can be suspended (false for FileWatch only) */
const QLatin1String DATA_ISAVAILABLE = QLatin1String("isAvailable");      /**< Service loaded / dbus interface up and running */
const QLatin1String DATA_ISSUSPENDED = QLatin1String("isSuspended");      /**< Service is suspended */
const QLatin1String DATA_ISACTIVE = QLatin1String("isActive");            /**< Service does something */
const QLatin1String DATA_PERCENT = QLatin1String("percent");              /**< Percentage of the current task */

 /**
  * Activity status of all plugins will only change every 3 seconds
  *
  * This prevents the trayicon from show/hide for very short operations
  */
const int ACTIVITY_TIMEOUT = 1000 * 3;


NepomukServiceEngine::NepomukServiceEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)

    , m_nepomukServerWatcher(0)
    , m_dBusServer(0)
    , m_fileWatcherWatcher(0)
    , m_dBusFileWatcher(0)
    , m_serviceFileWatcher(0)
    , m_fileIndexerWatcher(0)
    , m_dBusFileIndexer(0)
    , m_serviceFileIndexer(0)
    , m_akonadiFeederWatcher(0)
    , m_dBusAkonadiFeeder(0)
    , m_serviceAkonadiFeeder(0)
    , m_webMinerWatcher(0)
    , m_dBusWebMiner(0)
    , m_serviceWebMiner(0)
    , m_fileWatcherActive(false)
    , m_fileIndexerActive(false)
    , m_akonadiFeederActive(false)
    , m_webMinerActive(false)
{
    // we've passed the constructor's args to our parent class
    // we're done for now!

    m_updateTimer.setSingleShot(true);

    connect( &m_updateTimer, SIGNAL( timeout() ), this, SLOT( checkIsActive()) );
}

Plasma::Service *NepomukServiceEngine::serviceForSource(const QString &source)
{
    NepomukServiceService *service = 0;
    if(source == SERVICE_NEPOMUK) {
        service = new NepomukServiceService(this, source, QLatin1String("nepomuk"));
    }
    else {
        service = new NepomukServiceService(this, source, QLatin1String("nepomukservice"));
    }

    return service;
}

void NepomukServiceEngine::init()
{
    // default initialization the Nepomuk Servce Source
    Plasma::DataEngine::Data serverdata;
    serverdata.insert(DATA_STATUSMSG, i18n("The Nepomuk Service is disabled"));
    serverdata.insert(DATA_ISAVAILABLE, false);
    serverdata.insert(DATA_ISACTIVE, false);
    setData(SERVICE_NEPOMUK, serverdata);

    //Default initialization of all other supported sources
    setData(SERVICE_FILEWATCH, DATA_I18NNAME, i18n("File Watcher"));
    setData(SERVICE_FILEWATCH, DATA_CANBESUSPENDED, false);
    setData(SERVICE_FILEWATCH, DATA_ISSUSPENDED, false);

    setData(SERVICE_FILEINDEXER, DATA_I18NNAME, i18n("File Indexer"));
    setData(SERVICE_FILEINDEXER, DATA_CANBESUSPENDED, true);
    setData(SERVICE_FILEINDEXER, DATA_ISSUSPENDED, false);

    setData(SERVICE_PIM, DATA_I18NNAME, i18n("PIM Indexer"));
    setData(SERVICE_PIM, DATA_CANBESUSPENDED, true);
    setData(SERVICE_PIM, DATA_ISSUSPENDED, false);

    setData(SERVICE_WEBMINER, DATA_I18NNAME, i18n("WebMiner"));
    setData(SERVICE_WEBMINER, DATA_CANBESUSPENDED, true);
    setData(SERVICE_WEBMINER, DATA_ISSUSPENDED, false);

    fileWatcherDisabled();
    fileIndexerDisabled();
    akonadiFeederDisabled();
    webMinerDisabled();

    // async call to check if the dbus service is available
    QDBusPendingCall async = QDBusConnection::sessionBus().interface()->asyncCall(QLatin1String("NameHasOwner"), QLatin1String("org.kde.NepomukServer"));
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(async, this);
    connect(callWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(nepomukDBusServiceCheck(QDBusPendingCallWatcher*)));
}

void NepomukServiceEngine::nepomukDBusServiceCheck(QDBusPendingCallWatcher*call)
{
    QDBusPendingReply<bool> reply = *call;
    if ( !reply.isError() && reply.value() ) {
        nepomukEnabled();
    }
    call->deleteLater();

    // put the service watcher initialization here to avoid a race condition
    // watch if the nepomuk service is available in general
    m_nepomukServerWatcher = new QDBusServiceWatcher( QLatin1String("org.kde.NepomukServer"), QDBusConnection::sessionBus(),
                                                    QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                                    this );
    connect( m_nepomukServerWatcher, SIGNAL( serviceRegistered( QString ) ), this, SLOT( nepomukEnabled()) );
    connect( m_nepomukServerWatcher, SIGNAL( serviceUnregistered( QString ) ), this, SLOT( nepomukDisabled()) );

    checkIsActive();
}

void NepomukServiceEngine::nepomukEnabled()
{
    //#####################################
    // Server interface
    m_dBusServer = new QDBusInterface(QLatin1String("org.kde.NepomukServer"),
                                      QLatin1String("/nepomukserver"),
                                      QLatin1String("org.kde.NepomukServer"),
                                      QDBusConnection::sessionBus());

    setData(SERVICE_NEPOMUK, DATA_STATUSMSG, i18n("The Nepomuk Service is enabled"));
    setData(SERVICE_NEPOMUK, DATA_ISAVAILABLE, true);

    //#####################################
    // File Watcher
    // async call to check if the dbus service is available
    QDBusPendingCall asyncFW = QDBusConnection::sessionBus().interface()->asyncCall(QLatin1String("NameHasOwner"), QLatin1String("org.kde.nepomuk.services.nepomukfilewatch"));
    QDBusPendingCallWatcher *callWatcherFW = new QDBusPendingCallWatcher(asyncFW, this);
    connect(callWatcherFW, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(fileWatcherDBusServiceCheck(QDBusPendingCallWatcher*)));

    //#####################################
    // File indexer
    // async call to check if the dbus service is available
    QDBusPendingCall asyncFI = QDBusConnection::sessionBus().interface()->asyncCall(QLatin1String("NameHasOwner"), QLatin1String("org.kde.nepomuk.services.nepomukfileindexer"));
    QDBusPendingCallWatcher *callWatcherFI = new QDBusPendingCallWatcher(asyncFI, this);
    connect(callWatcherFI, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(fileIndexerDBusServiceCheck(QDBusPendingCallWatcher*)));

     //#####################################
     // Akonadi Feeder
    // async call to check if the dbus service is available
    QDBusPendingCall asyncAF = QDBusConnection::sessionBus().interface()->asyncCall(QLatin1String("NameHasOwner"), QLatin1String("org.freedesktop.Akonadi.Agent.akonadi_nepomuk_feeder"));
    QDBusPendingCallWatcher *callWatcherAF = new QDBusPendingCallWatcher(asyncAF, this);
    connect(callWatcherAF, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(akonadiFeederDBusServiceCheck(QDBusPendingCallWatcher*)));

     //#####################################
     // WebMiner
    // async call to check if the dbus service is available
    QDBusPendingCall asyncWM = QDBusConnection::sessionBus().interface()->asyncCall(QLatin1String("NameHasOwner"), QLatin1String("org.freedesktop.Akonadi.Agent.akonadi_nepomuk_feeder"));
    QDBusPendingCallWatcher *callWatcherWM = new QDBusPendingCallWatcher(asyncWM, this);
    connect(callWatcherWM, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(webMinerDBusServiceCheck(QDBusPendingCallWatcher*)));
}

void NepomukServiceEngine::fileWatcherDBusServiceCheck(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<bool> reply = *call;
    if ( !reply.isError() && reply.value() ) {
        fileWatcherEnabled();
    }
    else {
        fileWatcherDisabled();
    }
    call->deleteLater();

    // put the watcher initialization here to avoid a race condition
    m_serviceFileWatcher = new QDBusInterface(QLatin1String("org.kde.nepomuk.services.nepomukfilewatch"),
                                              QLatin1String("/servicecontrol"),
                                              QLatin1String("org.kde.nepomuk.ServiceControl"),
                                              QDBusConnection::sessionBus(), this);

    // watch for the file indexer service to come up and go down
    m_fileWatcherWatcher = new QDBusServiceWatcher( m_serviceFileWatcher->service(), QDBusConnection::sessionBus(),
                                                  QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                                  this );
    connect( m_fileWatcherWatcher, SIGNAL( serviceRegistered( QString ) ), this, SLOT( fileWatcherEnabled()) );
    connect( m_fileWatcherWatcher, SIGNAL( serviceUnregistered( QString ) ), this, SLOT( fileWatcherDisabled()) );
}

void NepomukServiceEngine::fileIndexerDBusServiceCheck(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<bool> reply = *call;
    if ( !reply.isError() && reply.value() ) {
        fileIndexerEnabled();
    }
    else {
        fileIndexerDisabled();
    }
    call->deleteLater();

    // put the watcher initialization here to avoid a race condition
    m_serviceFileIndexer = new QDBusInterface(QLatin1String("org.kde.nepomuk.services.nepomukfileindexer"),
                                              QLatin1String("/servicecontrol"),
                                              QLatin1String("org.kde.nepomuk.ServiceControl"),
                                              QDBusConnection::sessionBus(), this);

    // watch for the file indexer service to come up and go down
    m_fileIndexerWatcher = new QDBusServiceWatcher( m_serviceFileIndexer->service(), QDBusConnection::sessionBus(),
                                                  QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                                  this );

    connect( m_fileIndexerWatcher, SIGNAL( serviceRegistered( QString ) ), this, SLOT( fileIndexerEnabled()) );
    connect( m_fileIndexerWatcher, SIGNAL( serviceUnregistered( QString ) ), this, SLOT( fileIndexerDisabled()) );
}

void NepomukServiceEngine::akonadiFeederDBusServiceCheck(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<bool> reply = *call;
    if ( !reply.isError() && reply.value() ) {
        akonadiFeederEnabled();
    }
    else {
        akonadiFeederDisabled();
    }
    call->deleteLater();

    // put the watcher initialization here to avoid a race condition
    m_serviceAkonadiFeeder = new QDBusInterface(QLatin1String("org.freedesktop.Akonadi.Agent.akonadi_nepomuk_feeder"),
                                              QLatin1String("/"),
                                              QLatin1String("org.freedesktop.Akonadi.Agent.Control"),
                                              QDBusConnection::sessionBus(), this);

    // watch for the akonadi feeder agent to come up and go down
    m_akonadiFeederWatcher = new QDBusServiceWatcher( m_serviceAkonadiFeeder->service(),
                                                    QDBusConnection::sessionBus(),
                                                    QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                                    this );
    connect( m_akonadiFeederWatcher, SIGNAL( serviceRegistered( QString ) ), this, SLOT( akonadiFeederEnabled()) );
    connect( m_akonadiFeederWatcher, SIGNAL( serviceUnregistered( QString ) ), this, SLOT( akonadiFeederDisabled()) );
}

void NepomukServiceEngine::webMinerDBusServiceCheck(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<bool> reply = *call;
    if ( !reply.isError() && reply.value() ) {
        webMinerEnabled();
    }
    else {
        webMinerDisabled();
    }
    call->deleteLater();

    // put the watcher initialization here to avoid a race condition
    m_serviceWebMiner = new QDBusInterface(QLatin1String("org.kde.nepomuk.services.nepomuk-webminerservice"),
                                           QLatin1String("/servicecontrol"),
                                           QLatin1String("org.kde.nepomuk.ServiceControl"),
                                           QDBusConnection::sessionBus(), this);

    // watch for the akonadi feeder agent to come up and go down
    m_webMinerWatcher = new QDBusServiceWatcher( m_serviceWebMiner->service(),
                                                    QDBusConnection::sessionBus(),
                                                    QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                                    this );
    connect( m_webMinerWatcher, SIGNAL( serviceRegistered( QString ) ), this, SLOT( webMinerEnabled()) );
    connect( m_webMinerWatcher, SIGNAL( serviceUnregistered( QString ) ), this, SLOT( webMinerDisabled()) );
}

void NepomukServiceEngine::nepomukDisabled()
{
    setData(SERVICE_NEPOMUK, DATA_STATUSMSG, i18n("The Nepomuk Service is disabled"));
    setData(SERVICE_NEPOMUK, DATA_ISAVAILABLE, false);

    delete m_fileWatcherWatcher;
    m_fileWatcherWatcher = 0;
    delete m_dBusFileWatcher;
    m_dBusFileWatcher = 0;
    delete m_serviceFileWatcher;
    m_serviceFileWatcher = 0;

    delete m_fileIndexerWatcher;
    m_fileIndexerWatcher = 0;
    delete m_dBusFileIndexer;
    m_dBusFileIndexer = 0;
    delete m_serviceFileIndexer;
    m_serviceFileIndexer = 0;

    delete m_akonadiFeederWatcher;
    m_akonadiFeederWatcher = 0;
    delete m_dBusAkonadiFeeder;
    m_dBusAkonadiFeeder= 0;
    delete m_serviceAkonadiFeeder;
    m_serviceAkonadiFeeder = 0;

    delete m_webMinerWatcher;
    m_webMinerWatcher = 0;
    delete m_dBusWebMiner;
    m_dBusWebMiner= 0;
    delete m_serviceWebMiner;
    m_serviceWebMiner = 0;
}


void NepomukServiceEngine::fileWatcherEnabled()
{
    m_dBusFileWatcher = new QDBusInterface(QLatin1String("org.kde.nepomuk.services.nepomukfilewatch"),
                                           QLatin1String("/nepomukfilewatch"),
                                           QLatin1String("org.kde.nepomuk.FileWatch"),
                                           QDBusConnection::sessionBus(), this);

    connect( m_dBusFileWatcher, SIGNAL( status(int,QString) ), this, SLOT( updateFileWatcherStatus(int,QString) ) );
    connect( m_dBusFileWatcher, SIGNAL( metadataUpdateStarted() ), this, SLOT( fileWatcherStarted() ) );
    connect( m_dBusFileWatcher, SIGNAL( metadataUpdateStopped() ), this, SLOT( fileWatcherStopped() ) );

    setData(SERVICE_FILEWATCH, DATA_ISAVAILABLE, true);
    setData(SERVICE_FILEWATCH, DATA_ISACTIVE, false);
    setData(SERVICE_FILEWATCH, DATA_PERCENT, 0);

    updateFileWatcherStatus();
}

void NepomukServiceEngine::fileWatcherDisabled()
{
    delete m_dBusFileWatcher;
    m_dBusFileWatcher = 0;

    setData(SERVICE_FILEWATCH, DATA_STATUSMSG, i18n("The indexer is not started"));
    setData(SERVICE_FILEWATCH, DATA_STATUSENUM, STATUS_DISABLED);

    setData(SERVICE_FILEWATCH, DATA_ISAVAILABLE, false);
    setData(SERVICE_FILEWATCH, DATA_ISACTIVE, false);
    setData(SERVICE_FILEWATCH, DATA_PERCENT, 0);

    m_updateTimer.start(ACTIVITY_TIMEOUT);
}

void NepomukServiceEngine::updateFileWatcherStatus()
{
    if(!m_dBusFileWatcher) {
        return;
    }

    // asynchrone dbus call, watcher gets deleted again in finished slot via deleteLater
    QDBusPendingCall async = m_dBusFileWatcher->asyncCall("statusMessage");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(asyncFileWatcherStatusMsg(QDBusPendingCallWatcher*)));

    QDBusPendingCall async2 = m_dBusFileWatcher->asyncCall("statusMessage");
    QDBusPendingCallWatcher *watcher2 = new QDBusPendingCallWatcher(async2, this);

    QObject::connect(watcher2, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(asyncFileWatcherStatusInt(QDBusPendingCallWatcher*)));
}


void NepomukServiceEngine::updateFileWatcherStatus(int status, QString msg)
{
    updateFileWatcherStatus(status);
    updateFileWatcherStatus(msg);
}

void NepomukServiceEngine::asyncFileWatcherStatusInt(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<int> reply = *call;
    if ( !reply.isError()) {
        int status= reply.value();
        updateFileWatcherStatus(status);
    }
    call->deleteLater();
}

void NepomukServiceEngine::updateFileWatcherStatus(int status)
{
    if(status == 0) {
        fileWatcherStopped();
    }
    else if(status == 1) {
        fileWatcherStarted();
    }
}

void NepomukServiceEngine::asyncFileWatcherStatusMsg(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QString> reply = *call;
    if ( !reply.isError()) {
        QString status= reply.value();
        updateFileWatcherStatus(status);
    }
    call->deleteLater();
}

void NepomukServiceEngine::updateFileWatcherStatus(QString msg)
{
    setData(SERVICE_FILEWATCH, DATA_STATUSMSG, msg);
}

void NepomukServiceEngine::fileWatcherStarted()
{
    setData(SERVICE_FILEWATCH, DATA_ISACTIVE, true);
    setData(SERVICE_FILEWATCH, DATA_STATUSENUM, STATUS_NORMAL);
    m_fileWatcherActive = true;

    if(!m_updateTimer.isActive()) {
        m_updateTimer.start(ACTIVITY_TIMEOUT);
    }
}

void NepomukServiceEngine::fileWatcherStopped()
{
    setData(SERVICE_FILEWATCH, DATA_ISACTIVE, false);
    setData(SERVICE_FILEWATCH, DATA_STATUSENUM, STATUS_IDLE);
    m_fileWatcherActive = false;

    m_updateTimer.start(ACTIVITY_TIMEOUT);
}

void NepomukServiceEngine::fileIndexerEnabled()
{
    m_dBusFileIndexer = new QDBusInterface(QLatin1String("org.kde.nepomuk.services.nepomukfileindexer"),
                                           QLatin1String("/nepomukfileindexer"),
                                           QLatin1String("org.kde.nepomuk.FileIndexer"),
                                           QDBusConnection::sessionBus(), this);

    connect( m_dBusFileIndexer, SIGNAL( status(int,QString) ), this, SLOT( updateFileIndexerStatus(int,QString) ) );
    connect( m_dBusFileIndexer, SIGNAL( indexingStarted() ), this, SLOT( fileIndexerStarted() ) );
    connect( m_dBusFileIndexer, SIGNAL( indexingStopped() ), this, SLOT( fileIndexerStopped() ) );

    setData(SERVICE_FILEINDEXER, DATA_ISAVAILABLE, true);
    setData(SERVICE_FILEINDEXER, DATA_ISSUSPENDED, false);
    setData(SERVICE_FILEINDEXER, DATA_ISACTIVE, false);
    setData(SERVICE_FILEINDEXER, DATA_PERCENT, 0);

    updateFileIndexerStatus();
}

void NepomukServiceEngine::fileIndexerDisabled()
{
    delete m_dBusFileIndexer;
    m_dBusFileIndexer = 0;

    setData(SERVICE_FILEINDEXER, DATA_STATUSMSG, i18n("The indexer is not started"));
    setData(SERVICE_FILEINDEXER, DATA_STATUSENUM, STATUS_DISABLED);

    setData(SERVICE_FILEINDEXER, DATA_ISAVAILABLE, false);
    setData(SERVICE_FILEINDEXER, DATA_ISSUSPENDED, false);
    setData(SERVICE_FILEINDEXER, DATA_ISACTIVE, false);
    setData(SERVICE_FILEINDEXER, DATA_PERCENT, 0);

    m_updateTimer.start(ACTIVITY_TIMEOUT);
}

void NepomukServiceEngine::updateFileIndexerStatus()
{
    if(!m_dBusFileIndexer) {
        return;
    }

    // asynchrone dbus call, watcher gets deleted again in finished slot via deleteLater
    QDBusPendingCall async = m_dBusFileIndexer->asyncCall("statusMessage");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(asyncFileIndexerStatusMsg(QDBusPendingCallWatcher*)));

    QDBusPendingCall async2 = m_dBusFileIndexer->asyncCall("currentStatus");
    QDBusPendingCallWatcher *watcher2 = new QDBusPendingCallWatcher(async2, this);

    QObject::connect(watcher2, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(asyncFileIndexerStatusInt(QDBusPendingCallWatcher*)));
}

void NepomukServiceEngine::updateFileIndexerStatus(int status, QString msg)
{
    updateFileIndexerStatus(status);
    updateFileIndexerStatus(msg);
}

void NepomukServiceEngine::asyncFileIndexerStatusInt(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<int> reply = *call;
    if ( !reply.isError()) {
        int status= reply.value();
        updateFileIndexerStatus(status);
    }
    call->deleteLater();
}

void NepomukServiceEngine::updateFileIndexerStatus(int status)
{
    setData(SERVICE_FILEINDEXER, DATA_STATUSENUM, status);

    if(status == 4) { // suspended
        setData(SERVICE_FILEINDEXER, DATA_ISSUSPENDED, true);
        fileIndexerStopped();
    }
    else if(status == 1) { // idle
        setData(SERVICE_FILEINDEXER, DATA_ISSUSPENDED, false);
        fileIndexerStopped();
    }
    else if(status == 5) { //cleaning
        setData(SERVICE_FILEINDEXER, DATA_ISSUSPENDED, false);
        fileIndexerStarted();
    }
    else if(status == 0) { // normal (might be indexing or waiting to do something
                           // not idle state though
        setData(SERVICE_FILEINDEXER, DATA_ISSUSPENDED, false);

        if(!m_dBusFileIndexer) {
            return;
        }

        // asynchrone dbus call, watcher gets deleted again in finished slot via deleteLater
        QDBusPendingCall async = m_dBusFileIndexer->asyncCall("isIndexing");
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(asyncFileIndexerIsIndexing(QDBusPendingCallWatcher*)));
    }
    else { // suspended due to low battery etc
        setData(SERVICE_FILEINDEXER, DATA_ISSUSPENDED, true);
        fileIndexerStopped();
    }
}

void NepomukServiceEngine::asyncFileIndexerStatusMsg(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QString> reply = *call;
    if ( !reply.isError()) {
        QString status= reply.value();
        updateFileIndexerStatus(status);
    }
    call->deleteLater();
}

void NepomukServiceEngine::updateFileIndexerStatus(QString msg)
{
    setData(SERVICE_FILEINDEXER, DATA_STATUSMSG, msg);
}

void NepomukServiceEngine::asyncFileIndexerIsIndexing(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<bool> reply = *call;
    if ( !reply.isError() && reply.value()) {
        fileIndexerStarted();
    }
    else {
        fileIndexerStopped();
    }
    call->deleteLater();
}

void NepomukServiceEngine::fileIndexerStarted()
{
    setData(SERVICE_FILEINDEXER, DATA_ISACTIVE, true);
    m_fileIndexerActive = true;

    if(!m_updateTimer.isActive()) {
        m_updateTimer.start(ACTIVITY_TIMEOUT);
    }
}

void NepomukServiceEngine::fileIndexerStopped()
{
    setData(SERVICE_FILEINDEXER, DATA_ISACTIVE, false);
    m_fileIndexerActive = false;

    // the fileindexer sends indexingStopped/started signals after each batch of 10 files
    // that are indexed, this causes the restart the timer all the time
    // and never set isActive for the server if we do not check for active time too.

    // on the downside, the icon in the systray might get hidden right after the last file was processed
    // and thus not stay for a longer time
    if( !m_updateTimer.isActive()) {
        m_updateTimer.start(ACTIVITY_TIMEOUT);
    }
}

void NepomukServiceEngine::akonadiFeederEnabled()
{
    m_dBusAkonadiFeeder = new QDBusInterface(QLatin1String("org.freedesktop.Akonadi.Agent.akonadi_nepomuk_feeder"),
                                           QLatin1String("/"),
                                           QLatin1String("org.freedesktop.Akonadi.Agent.Status"),
                                           QDBusConnection::sessionBus(), this);

    connect( m_dBusAkonadiFeeder, SIGNAL( status(int, QString) ), this, SLOT( updateAkonadiFeederStatus(int,QString) ) );
    connect( m_dBusAkonadiFeeder, SIGNAL( onlineChanged(bool) ), this, SLOT( akonadiFeederOnlineChanged(bool) ) );
    connect( m_dBusAkonadiFeeder, SIGNAL( percent(int) ), this, SLOT( updateAkonadiProgress(int) ) );

    setData(SERVICE_PIM, DATA_ISAVAILABLE, true);
    setData(SERVICE_PIM, DATA_ISSUSPENDED, false);
    setData(SERVICE_PIM, DATA_ISACTIVE, false);
    setData(SERVICE_PIM, DATA_STATUSENUM, STATUS_NORMAL);
    setData(SERVICE_PIM, DATA_PERCENT, 0);

    QDBusPendingCall async = m_dBusAkonadiFeeder->asyncCall("isOnline");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(asyncFeederOnlineChanged(QDBusPendingCallWatcher*)));
}

void NepomukServiceEngine::asyncFeederOnlineChanged(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<bool> reply = *call;
    if ( !reply.isError() && reply.value()) {
        akonadiFeederOnlineChanged(true);
    }
    else {
        akonadiFeederOnlineChanged(false);
    }
    call->deleteLater();
}

void NepomukServiceEngine::akonadiFeederDisabled()
{
    delete m_dBusAkonadiFeeder;
    m_dBusAkonadiFeeder = 0;

    setData(SERVICE_PIM, DATA_STATUSMSG, i18n("The indexer is not started"));
    setData(SERVICE_PIM, DATA_STATUSENUM, STATUS_DISABLED);

    setData(SERVICE_PIM, DATA_ISAVAILABLE, false);
    setData(SERVICE_PIM, DATA_ISSUSPENDED, false);
    setData(SERVICE_PIM, DATA_ISACTIVE, false);
    setData(SERVICE_PIM, DATA_PERCENT, 0);

    m_updateTimer.start(ACTIVITY_TIMEOUT);
}

void NepomukServiceEngine::akonadiFeederOnlineChanged(bool enable)
{
    updateAkonadiFeederStatus();

    if(enable) {
        setData(SERVICE_PIM, DATA_ISSUSPENDED, false);
        setData(SERVICE_PIM, DATA_ISACTIVE, false);
    }
    else {
        akonadiFeederStopped();

        setData(SERVICE_PIM, DATA_ISSUSPENDED, true);
        setData(SERVICE_PIM, DATA_STATUSENUM, STATUS_DISABLED);
    }
}

void NepomukServiceEngine::updateAkonadiFeederStatus()
{
    if(!m_dBusAkonadiFeeder)
        return;

    // asynchrone dbus call, watcher gets deleted again in finished slot via deleteLater
    QDBusPendingCall async = m_dBusAkonadiFeeder->asyncCall("statusMessage");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(asyncAkonadiFeederStatusMsg(QDBusPendingCallWatcher*)));

    QDBusPendingCall async2 = m_dBusAkonadiFeeder->asyncCall("status");
    QDBusPendingCallWatcher *watcher2 = new QDBusPendingCallWatcher(async2, this);

    QObject::connect(watcher2, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(asyncAkonadiFeederStatusInt(QDBusPendingCallWatcher*)));

    /*
    //TODO: implement propper progress information for the akonadi feeder
    QDBusReply<int> progressReply = m_dBusAkonadiFeeder->call( "progress" );

    if ( progressReply.isValid() ) {
        setData(SERVICE_PIM, DATA_PERCENT, progressReply.value());
    }
    else {
        setData(SERVICE_PIM, DATA_PERCENT, 0);
    }
    */
}

void NepomukServiceEngine::updateAkonadiFeederStatus(int status, QString msg)
{
    updateAkonadiFeederStatus(status);
    updateAkonadiFeederStatus(msg);
}

void NepomukServiceEngine::asyncAkonadiFeederStatusInt(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<int> reply = *call;
    if ( !reply.isError()) {
        int status= reply.value();
        updateAkonadiFeederStatus(status);
    }
    call->deleteLater();

}

void NepomukServiceEngine::updateAkonadiFeederStatus(int status)
{
    if(status == 1) {
        akonadiFeederStarted();
    }
    else {
        akonadiFeederStopped();
    }
}

void NepomukServiceEngine::asyncAkonadiFeederStatusMsg(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QString> reply = *call;
    if ( !reply.isError()) {
        QString status= reply.value();
        updateAkonadiFeederStatus(status);
    }
    call->deleteLater();
}

void NepomukServiceEngine::updateAkonadiFeederStatus(QString msg)
{
    setData(SERVICE_PIM, DATA_STATUSMSG, msg);
}

void NepomukServiceEngine::updateAkonadiProgress(int progress)
{
    setData(SERVICE_PIM, DATA_PERCENT, progress);
}

void NepomukServiceEngine::akonadiFeederStarted()
{
    setData(SERVICE_PIM, DATA_ISSUSPENDED, false);
    setData(SERVICE_PIM, DATA_ISACTIVE, true);
    setData(SERVICE_PIM, DATA_STATUSENUM, STATUS_NORMAL);
    m_akonadiFeederActive = true;

    if(!m_updateTimer.isActive()) {
        m_updateTimer.start(ACTIVITY_TIMEOUT);
    }
}

void NepomukServiceEngine::akonadiFeederStopped()
{
    setData(SERVICE_PIM, DATA_ISACTIVE, false);
    setData(SERVICE_PIM, DATA_STATUSENUM, STATUS_IDLE);
    m_akonadiFeederActive = false;

    m_updateTimer.start(ACTIVITY_TIMEOUT);
}

void NepomukServiceEngine::webMinerEnabled()
{
    m_dBusWebMiner = new QDBusInterface(QLatin1String("org.kde.nepomuk.services.nepomuk-webminerservice"),
                                           QLatin1String("/WebMiner"),
                                           QLatin1String("org.kde.nepomuk.WebMiner"),
                                           QDBusConnection::sessionBus(), this);

    connect( m_dBusWebMiner, SIGNAL( status(int,QString) ), this, SLOT( updateWebMinerStatus(int,QString) ) );

    setData(SERVICE_WEBMINER, DATA_ISAVAILABLE, true);
    setData(SERVICE_WEBMINER, DATA_ISSUSPENDED, false);
    setData(SERVICE_WEBMINER, DATA_ISACTIVE, false);
    setData(SERVICE_WEBMINER, DATA_PERCENT, 0);

    updateWebMinerStatus();
}

void NepomukServiceEngine::webMinerDisabled()
{
    delete m_dBusWebMiner;
    m_dBusWebMiner = 0;

    setData(SERVICE_WEBMINER, DATA_STATUSMSG, i18n("The indexer is not started"));
    setData(SERVICE_WEBMINER, DATA_STATUSENUM, STATUS_DISABLED);

    setData(SERVICE_WEBMINER, DATA_ISAVAILABLE, false);
    setData(SERVICE_WEBMINER, DATA_ISSUSPENDED, false);
    setData(SERVICE_WEBMINER, DATA_ISACTIVE, false);
    setData(SERVICE_WEBMINER, DATA_PERCENT, 0);

    m_updateTimer.start(ACTIVITY_TIMEOUT);
}

void NepomukServiceEngine::updateWebMinerStatus()
{
    if(!m_dBusWebMiner) {
        return;
    }

    // asynchrone dbus call, watcher gets deleted again in finished slot via deleteLater
    QDBusPendingCall async = m_dBusWebMiner->asyncCall("statusMessage");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(asyncWebMinerStatusMsg(QDBusPendingCallWatcher*)));

    QDBusPendingCall async2 = m_dBusWebMiner->asyncCall("status");
    QDBusPendingCallWatcher *watcher2 = new QDBusPendingCallWatcher(async2, this);

    QObject::connect(watcher2, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(asyncWebMinerStatusInt(QDBusPendingCallWatcher*)));
}

void NepomukServiceEngine::updateWebMinerStatus(int status, QString msg)
{
    updateWebMinerStatus(status);
    updateWebMinerStatus(msg);
}

void NepomukServiceEngine::asyncWebMinerStatusInt(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<int> reply = *call;
    if ( !reply.isError()) {
        int status= reply.value();
        updateWebMinerStatus(status);
    }
    call->deleteLater();

}

void NepomukServiceEngine::updateWebMinerStatus(int status)
{
    //FIXME: expose more detailed status integer (onBattery/noNetwork etc)
    switch(status) {
    case 0: //idle
        setData(SERVICE_WEBMINER, DATA_STATUSENUM, STATUS_IDLE);
        setData(SERVICE_WEBMINER, DATA_ISSUSPENDED, false);
        webMinerStopped();
        break;
    case 1: //indexing
        setData(SERVICE_WEBMINER, DATA_ISSUSPENDED, false);
        webMinerStarted();
        break;
    case 2: //suspended
        setData(SERVICE_WEBMINER, DATA_STATUSENUM, STATUS_SUSPENDED);
        setData(SERVICE_WEBMINER, DATA_ISSUSPENDED, true);
        webMinerStopped();
        break;
    }
}

void NepomukServiceEngine::asyncWebMinerStatusMsg(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QString> reply = *call;
    if ( !reply.isError()) {
        QString status= reply.value();
        updateWebMinerStatus(status);
    }
    call->deleteLater();
}

void NepomukServiceEngine::updateWebMinerStatus(QString msg)
{
    setData(SERVICE_WEBMINER, DATA_STATUSMSG, msg);
}

void NepomukServiceEngine::webMinerStarted()
{
    setData(SERVICE_WEBMINER, DATA_STATUSENUM, STATUS_NORMAL);
    setData(SERVICE_WEBMINER, DATA_ISACTIVE, true);

    m_webMinerActive = true;

    if( !m_updateTimer.isActive()) {
        m_updateTimer.start(ACTIVITY_TIMEOUT);
    }
}

void NepomukServiceEngine::webMinerStopped()
{
    setData(SERVICE_WEBMINER, DATA_ISACTIVE, false);

    m_webMinerActive = false;

    if( !m_updateTimer.isActive()) {
        m_updateTimer.start(ACTIVITY_TIMEOUT);
    }
}

void NepomukServiceEngine::checkIsActive()
{
    // sets the active/not active state if at least one of the services is working
    // used to show/hide the icon in the systray
    if(m_fileWatcherActive || m_fileIndexerActive || m_akonadiFeederActive || m_webMinerActive) {
        setData(SERVICE_NEPOMUK, DATA_ISACTIVE, true);
    }
    else if(!m_fileWatcherActive && !m_fileIndexerActive && !m_akonadiFeederActive && !m_webMinerActive) {
        setData(SERVICE_NEPOMUK, DATA_ISACTIVE, false);
    }
}










// export the plugin; use the plugin name and the class name
K_EXPORT_PLASMA_DATAENGINE(nepomuk-serviceengine, NepomukServiceEngine)

// include the moc file so the build system makes it for us
#include "nepomukserviceengine.moc"

