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

#ifndef IP_H
#define IP_H

#include "geolocationprovider.h"

class KJob;
namespace KIO {
    class Job;
}

class Ip : public GeolocationProvider
{
    Q_OBJECT
public:
    explicit Ip(QObject *parent = 0, const QVariantList &args = QVariantList());
    ~Ip();

    virtual void update();

protected Q_SLOTS:
    void readData(KIO::Job *, const QByteArray& data);
    void result(KJob* job);

private:
    class Private;
    Private *const d;

};

#endif
