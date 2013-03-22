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

#ifndef NEPOMUKSERVICE_SERVICE_H
#define NEPOMUKSERVICE_SERVICE_H

#include <Plasma/Service>
#include <Plasma/ServiceJob>

using namespace Plasma;

/**
 * @brief retrieves the right serviceJob for the Nepomuk DataEngine
 */
class NepomukServiceService : public Plasma::Service
{
    Q_OBJECT

public:
    /**
     * @brief NepomukServiceService
     *
     * @param parent some parent obejct
     * @param source the source for which the service will be requested
     * @param service what service.operation file should be used
     *        @arg @c nepomuk
     *        @arg @c nepomukservice
     */
    NepomukServiceService(QObject *parent, const QString &source, const QString &service);
    ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters);

private:
    QString m_id;
};

#endif // SEARCHLAUNCH_SERVICE_H
