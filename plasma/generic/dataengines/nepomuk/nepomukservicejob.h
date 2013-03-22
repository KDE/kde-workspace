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

#ifndef NEPOMUKSERVICEJOB_H
#define NEPOMUKSERVICEJOB_H

// plasma
#include <Plasma/ServiceJob>

class QDBusInterface;

/**
 * @brief Service operations to manipulate the Nepomuk DataStorage and the indexing plugins
 *
 * Allows to start/stop/resume/suspend all plugins at once via Nepomuk DataStorage
 * Allows to start/stop/suspend/resume and show settings for each individual indexer service
 */
class NepomukServiceJob : public Plasma::ServiceJob
{

    Q_OBJECT

    public:
        NepomukServiceJob(const QString &id, const QString &operation, QMap<QString, QVariant> &parameters, QObject *parent = 0);
        ~NepomukServiceJob();

    protected:
        void start();

    private:
        void openFileWatcherSettings();
        void openFileIndexerSettings();
        void openAkonadiFeederSettings();
        void openWebMinerSettings();

        void suspendFileIndexer(bool suspend);
        void enableFileIndexer(bool enable);

        void suspendAkonadiFeeder(bool suspend);
        void enableAkonadiFeeder(bool enable);

        void suspendWebMiner(bool suspend);
        void enableWebMiner(bool enable);

        QDBusInterface *m_dBusServer;
        QDBusInterface *m_serviceFileWatcher;
        QDBusInterface *m_dbusFileIndexer;
        QDBusInterface *m_serviceFileIndexer;
        QDBusInterface *m_dBusAkonadiFeeder;
        QDBusInterface *m_dbusWebMiner;
        QDBusInterface *m_serviceWebMiner;

        QString m_id;
};

#endif // TASKJOB_H
