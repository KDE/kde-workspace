/*
 * Class to generate support information for plasma shells
 *
 * Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SUPPORTINFORMATION_H
#define SUPPORTINFORMATION_H

#include <QDebug>

namespace Plasma
{
    class Corona;
    class Containment;
    class Applet;
}

class SupportInformation
{
public:
    static QString generateSupportInformation(Plasma::Corona *corona);

private:
    SupportInformation(const QDebug &outputStream);

    void addHeader();

    void addInformationForCorona(Plasma::Corona *corona);
    void addInformationForContainment(Plasma::Containment *containment);
    void addInformationForApplet(Plasma::Applet *applet);

    void addSeperator();

    QDebug m_stream;
};

#endif // SUPPORTINFORMATION_H
