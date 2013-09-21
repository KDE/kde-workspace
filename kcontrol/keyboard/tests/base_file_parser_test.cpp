/*
 *  Copyright (C) 2011 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <kdebug.h>
#include <QtGui/QApplication>
#include <qtest_kde.h>

#include "model_to_geometry.h"


class BaseParserTest : public QObject
{
    Q_OBJECT

    ModelToGeometryTable modelToGeom;

private Q_SLOTS:
    void initTestCase() {
        modelToGeom.createTable();
    }

    void cleanupTestCase() {
    }

    void testBaseParser() {
        QCOMPARE( modelToGeom.getGeometryName("latitude"), QString("latitude") );
        QCOMPARE( modelToGeom.getGeometryFile("macbook78"), QString("macintosh") );
        QCOMPARE( modelToGeom.getGeometryName("pc101"), QString("pc101") );
        QCOMPARE( modelToGeom.getGeometryFile("pc101"), QString("pc") );
        QCOMPARE( modelToGeom.getGeometryName("random"), QString("pc104") );
    }

};

//TODO: something lighter than KDEMAIN ?
QTEST_KDEMAIN( BaseParserTest, NoGUI )

#include "base_file_parser_test.moc"

