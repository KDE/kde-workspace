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

#include "geometry_parser.h"


class GeometryParserTest : public QObject
{
    Q_OBJECT

    Geometry geometry;

private Q_SLOTS:
    void initTestCase() {
	QString model("pc104");
	geometry = grammar::parseGeometry(model);
    }

    void cleanupTestCase() {
//    	delete geometry;
    }

    void testGeometryParser() {
        QVERIFY( geometry.getWidth() > 0 );
        QVERIFY( geometry.getShapeCount() > 0 );
//        QVERIFY( ! geometry.getName().isEmpty() );
        QVERIFY( !geometry.getDescription().isEmpty() );

        QCOMPARE( geometry.getName(), QString("pc104") );
    }

};

//TODO: something lighter than KDEMAIN ?
QTEST_KDEMAIN( GeometryParserTest, NoGUI )

#include "geometry_parser_test.moc"
