/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GEOLOCATION_DATAENGINE_H
#define GEOLOCATION_DATAENGINE_H

#include <QTimer>

#include <Plasma/DataEngine>
#include <Solid/Networking>

#include "geolocationprovider.h"

class GeolocationProvider;

class Geolocation : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        Geolocation(QObject* parent, const QVariantList& args);
        virtual ~Geolocation();
        virtual void init();
        virtual QStringList sources() const;

    protected:
        bool sourceRequestEvent(const QString &name);
        bool updateSourceEvent(const QString& name);
        bool updatePlugins(GeolocationProvider::UpdateTriggers triggers);

    protected slots:
        void networkStatusChanged();
        void pluginAvailabilityChanged(GeolocationProvider *provider);
        void pluginUpdated();
        void actuallySetData();

    private:
        Data m_data;
        EntryAccuracy m_accuracy;
        QList<GeolocationProvider *> m_plugins;
        QTimer m_updateTimer;
};

K_EXPORT_PLASMA_DATAENGINE(geolocation, Geolocation)

#endif

