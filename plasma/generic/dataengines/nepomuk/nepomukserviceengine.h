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

#ifndef NEPOMUKSERVICEENGINE_H
#define NEPOMUKSERVICEENGINE_H

#include <Plasma/DataEngine>
#include <QtCore/QTimer>

class QDBusInterface;
class QDBusServiceWatcher;
class QDBusPendingCallWatcher;

/**
 * @brief Nepomuk DataEngine to expose runtime information about the Nepomuk indexer
 *
 * Delivers current state and an translated status string.
 * Also allows via the NepomukServiceServcie to suspend/resume/start/stop the indexer
 *
 * usually the user should only suspend/resume any indexer though.
 *
 * Available Sources:
 * - "Nepomuk" returns some general information about the indexer and Nepomuk
 *     - statusMessage : tells if the data storage is enabled or not
 *     - isAvailable: to to tell if the storage is enabled or not
 *     - isActive: true when at least one indexer plugin is working
 * - "FileWatch"
 *     - statusMessage : translated string of the current FileWatch status
 *     - status: integer of the state @see NepomukServiceEngine::ServiceStatus
 *     - isAvailable: if FileWatch is available on dbus
 *     - isActive: true when the FileWatch is working
 * - "FileIndexer"
 *     - statusMessage : translated string of the current FileIndexer status
 *     - status: integer of the state @see NepomukServiceEngine::ServiceStatus
 *     - isAvailable: if FileIndexer is available on dbus
 *     - isSuspended: if FileIndexer is suspended
 *     - isActive: true when the FileIndexer is working
 * - "WebMiner"
 *     - statusMessage : translated string of the current WebMiner status
 *     - status: integer of the state @see NepomukServiceEngine::ServiceStatus
 *     - isAvailable: if WebMiner is available on dbus
 *     - isSuspended: if WebMiner is suspended
 *     - isActive: true when the WebMiner is working
 * - "PIM"
 *     - statusMessage : translated string of the current Akonadi_Feeder status
 *     - status: integer of the state @see NepomukServiceEngine::ServiceStatus
 *     - isAvailable: if Akonadi_Feeder is available on dbus
 *     - isSuspended: if Akonadi_Feeder is suspended
 *     - isActive: true when the Akonadi_Feeder is working
 *
 * @todo Add status progress (0-100%) output to all services
 * @todo Add timer for statusMessage updates so they do not change to quickly (otherwise FileWatch changes can't be read)
 */
class NepomukServiceEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    NepomukServiceEngine(QObject *parent, const QVariantList &args);
    Plasma::Service *serviceForSource(const QString &source);

    void init();

private slots:
    void nepomukEnabled();
    void nepomukDisabled();

    void fileWatcherEnabled();
    void fileWatcherDisabled();
    void updateFileWatcherStatus();
    void updateFileWatcherStatus(int status, QString msg);
    void asyncFileWatcherStatusInt(QDBusPendingCallWatcher *call);
    void updateFileWatcherStatus(int status);
    void asyncFileWatcherStatusMsg(QDBusPendingCallWatcher *call);
    void updateFileWatcherStatus(QString msg);
    void fileWatcherStarted();
    void fileWatcherStopped();

    void fileIndexerEnabled();
    void fileIndexerDisabled();
    void updateFileIndexerStatus();
    void updateFileIndexerStatus(int status, QString msg);
    void asyncFileIndexerStatusInt(QDBusPendingCallWatcher *call);
    void updateFileIndexerStatus(int status);
    void asyncFileIndexerStatusMsg(QDBusPendingCallWatcher *call);
    void updateFileIndexerStatus(QString msg);
    void asyncFileIndexerIsIndexing(QDBusPendingCallWatcher *call);
    void fileIndexerStarted();
    void fileIndexerStopped();

    void akonadiFeederEnabled();
    void akonadiFeederDisabled();
    void asyncFeederOnlineChanged(QDBusPendingCallWatcher *call);
    void akonadiFeederOnlineChanged(bool enable);
    void updateAkonadiFeederStatus();
    void updateAkonadiFeederStatus(int status, QString msg);
    void asyncAkonadiFeederStatusInt(QDBusPendingCallWatcher *call);
    void updateAkonadiFeederStatus(int status);
    void asyncAkonadiFeederStatusMsg(QDBusPendingCallWatcher *call);
    void updateAkonadiFeederStatus(QString msg);
    void updateAkonadiProgress(int progress);
    void akonadiFeederStarted();
    void akonadiFeederStopped();

    void webMinerEnabled();
    void webMinerDisabled();
    void updateWebMinerStatus();
    void updateWebMinerStatus(int status, QString msg);
    void asyncWebMinerStatusInt(QDBusPendingCallWatcher *call);
    void updateWebMinerStatus(int status);
    void asyncWebMinerStatusMsg(QDBusPendingCallWatcher *call);
    void updateWebMinerStatus(QString msg);
    void webMinerStarted();
    void webMinerStopped();

    /**
     * @brief called whenever one of the plugins start/stop indexing
     *
     * The start/stop indexing of the plugins start the @c m_updateTimer for
     * @c ACTIVITY_TIMEOUT msec. Afterwards this methods checks the overall
     * activity and set the @c Nepomuk source accordingly
     *
     * Helps to avoid fast show/hide transitions in the systray
     */
    void checkIsActive();

private:
    enum ServiceStatus {
        STATUS_NORMAL = 0,          /**< Normal mode, mostly means it is doing something */
        STATUS_IDLE = 1,            /**< Idle, does not do anything to save cpu cycles */
        STATUS_ONBATTERY = 2,       /**< suspended due to battery mode */
        STATUS_LOWDISKSPACE = 3,    /**< suspended due to low diskspace */
        STATUS_SUSPENDED = 4,       /**< suspended by the user */
        STATUS_CLEANING = 5,        /**< FileIndexer is cleaning data */
        STATUS_NOTNETWORK = 6,      /**< suspended due to missing network */
        STATUS_DISABLED = 50        /**< no dbus service available/service disabled also has ISAVAILABLE=false*/
    };

    QDBusServiceWatcher* nepomukServerWatcher;
    QDBusInterface *m_dBusServer;

    QDBusServiceWatcher* fileWatcherWatcher;
    QDBusInterface *m_dBusFileWatcher;
    QDBusInterface *m_serviceFileWatcher;

    QDBusServiceWatcher* fileIndexerWatcher;
    QDBusInterface *m_dBusFileIndexer;
    QDBusInterface *m_serviceFileIndexer;

    QDBusServiceWatcher* akonadiFeederWatcher;
    QDBusInterface *m_dBusAkonadiFeeder;
    QDBusInterface *m_serviceAkonadiFeeder;

    QDBusServiceWatcher* webMinerWatcher;
    QDBusInterface *m_dBusWebMiner;
    QDBusInterface *m_serviceWebMiner;


    // Delay the status change
    // If the status changed from pasive to active back to passive in less than 3000ms the
    // server status is not changed.
    // also it stays active for 3000ms after the last indexer went passive
    // this delays the hide/show in the systray
    QTimer m_updateTimer;

    //Saved here, as data pushed into the data storage is sent away and can't be retrieved here
    bool m_fileWatcherActive;
    bool m_fileIndexerActive;
    bool m_akonadiFeederActive;
    bool m_webMinerActive;
};

#endif
