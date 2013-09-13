/*
 *  Copyright (C) 2011 Andriy Rysin (rysin@kde.org), Shivam Makkar (amourphious1992@gmail.com)
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

#include "symbol_parser.h"


class SymbolParserTest : public QObject
{
    Q_OBJECT

    KbLayout kblayout;

private Q_SLOTS:
    void initTestCase() {
    QString layout("us");
    QString variant("basic");
    kblayout = grammar::parseSymbols(layout, variant);
    }

    void cleanupTestCase() {
//    	delete kblayout;
    }

    void testSymbolParser() {
        QVERIFY( kblayout.getKeyCount() > 0 );
        QVERIFY( !kblayout.getLayoutName().isEmpty() );
        QCOMPARE( kblayout.country, QString("us") );
    }

};

//TODO: something lighter than KDEMAIN ?
QTEST_KDEMAIN( SymbolParserTest, NoGUI )

#include "symbol_parser_test.moc"

