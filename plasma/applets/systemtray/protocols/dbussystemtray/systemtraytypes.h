/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef SYSTEMTRAYTYPES_H
#define SYSTEMTRAYTYPES_H

#include <QDBusArgument>
#include <QVector>

struct Icon {
    int width;
    int height;
    QByteArray data;
};

const QDBusArgument &operator<<(QDBusArgument &argument, const Icon &icon);
const QDBusArgument &operator>>(const QDBusArgument &argument, Icon &icon);

Q_DECLARE_METATYPE(Icon)

typedef QVector<Icon> IconVector;
const QDBusArgument &operator<<(QDBusArgument &argument, const IconVector &iconVector);
const QDBusArgument &operator>>(const QDBusArgument &argument, IconVector &iconVector);

Q_DECLARE_METATYPE(IconVector)

#endif
