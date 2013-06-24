/********************************************************************
KWin - the KDE window manager
This file is part of the KDE project.

Copyright (C) 2012 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "../virtualdesktops.h"
// KDE
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KConfigGroup>

#include <QtTest/QtTest>

namespace KWin {

int screen_number = 0;

}

Q_DECLARE_METATYPE(Qt::Orientation)

using namespace KWin;

class TestVirtualDesktops : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void count_data();
    void count();
    void navigationWrapsAround_data();
    void navigationWrapsAround();
    void current_data();
    void current();
    void currentChangeOnCountChange_data();
    void currentChangeOnCountChange();
    void next_data();
    void next();
    void previous_data();
    void previous();
    void left_data();
    void left();
    void right_data();
    void right();
    void above_data();
    void above();
    void below_data();
    void below();
    void updateGrid_data();
    void updateGrid();
    void updateLayout_data();
    void updateLayout();
    void name_data();
    void name();
    void switchToShortcuts();
    void load();
    void save();

private:
    void addDirectionColumns();
    template<typename T>
    void testDirection(const QString &actionName);
};

void TestVirtualDesktops::init()
{
    VirtualDesktopManager::create();
    screen_number = 0;
}

void TestVirtualDesktops::cleanup()
{
    delete VirtualDesktopManager::self();
}

static const uint s_countInitValue = 2;

void TestVirtualDesktops::count_data()
{
    QTest::addColumn<uint>("request");
    QTest::addColumn<uint>("result");
    QTest::addColumn<bool>("signal");
    QTest::addColumn<bool>("removedSignal");

    QTest::newRow("Minimum")       << (uint)1 << (uint)1 << true << true;
    QTest::newRow("Below Minimum") << (uint)0 << (uint)1 << true << true;
    QTest::newRow("Normal Value")  << (uint)10 << (uint)10 << true << false;
    QTest::newRow("Maximum")       << VirtualDesktopManager::maximum() << VirtualDesktopManager::maximum() << true << false;
    QTest::newRow("Above Maximum") << VirtualDesktopManager::maximum() + 1 << VirtualDesktopManager::maximum() << true << false;
    QTest::newRow("Unchanged")     << s_countInitValue << s_countInitValue << false << false;
}

void TestVirtualDesktops::count()
{
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    QCOMPARE(vds->count(), (uint)0);
    // start with a useful desktop count
    vds->setCount(s_countInitValue);

    QSignalSpy spy(vds, SIGNAL(countChanged(uint,uint)));
    QSignalSpy desktopsRemoved(vds, SIGNAL(desktopsRemoved(uint)));

    QFETCH(uint, request);
    QFETCH(uint, result);
    QFETCH(bool, signal);
    QFETCH(bool, removedSignal);
    vds->setCount(request);
    QCOMPARE(vds->count(), result);
    QCOMPARE(spy.isEmpty(), !signal);
    if (!spy.isEmpty()) {
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.count(), 2);
        QCOMPARE(arguments.at(0).type(), QVariant::UInt);
        QCOMPARE(arguments.at(1).type(), QVariant::UInt);
        QCOMPARE(arguments.at(0).toUInt(), s_countInitValue);
        QCOMPARE(arguments.at(1).toUInt(), result);
    }
    QCOMPARE(desktopsRemoved.isEmpty(), !removedSignal);
    if (!desktopsRemoved.isEmpty()) {
        QList<QVariant> arguments = desktopsRemoved.takeFirst();
        QCOMPARE(arguments.count(), 1);
        QCOMPARE(arguments.at(0).type(), QVariant::UInt);
        QCOMPARE(arguments.at(0).toUInt(), s_countInitValue);
    }
}

void TestVirtualDesktops::navigationWrapsAround_data()
{
    QTest::addColumn<bool>("init");
    QTest::addColumn<bool>("request");
    QTest::addColumn<bool>("result");
    QTest::addColumn<bool>("signal");

    QTest::newRow("enable")        << false << true  << true  << true;
    QTest::newRow("disable")       << true  << false << false << true;
    QTest::newRow("keep enabled")  << true  << true  << true  << false;
    QTest::newRow("keep disabled") << false << false << false << false;
}

void TestVirtualDesktops::navigationWrapsAround()
{
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    QCOMPARE(vds->isNavigationWrappingAround(), false);
    QFETCH(bool, init);
    QFETCH(bool, request);
    QFETCH(bool, result);
    QFETCH(bool, signal);

    // set to init value
    vds->setNavigationWrappingAround(init);
    QCOMPARE(vds->isNavigationWrappingAround(), init);

    QSignalSpy spy(vds, SIGNAL(navigationWrappingAroundChanged()));
    vds->setNavigationWrappingAround(request);
    QCOMPARE(vds->isNavigationWrappingAround(), result);
    QCOMPARE(spy.isEmpty(), !signal);
}

void TestVirtualDesktops::current_data()
{
    QTest::addColumn<uint>("count");
    QTest::addColumn<uint>("init");
    QTest::addColumn<uint>("request");
    QTest::addColumn<uint>("result");
    QTest::addColumn<bool>("signal");

    QTest::newRow("lower")         << (uint)4 << (uint)3 << (uint)2 << (uint)2 << true;
    QTest::newRow("higher")        << (uint)4 << (uint)1 << (uint)2 << (uint)2 << true;
    QTest::newRow("maximum")       << (uint)4 << (uint)1 << (uint)4 << (uint)4 << true;
    QTest::newRow("above maximum") << (uint)4 << (uint)1 << (uint)5 << (uint)1 << false;
    QTest::newRow("minimum")       << (uint)4 << (uint)2 << (uint)1 << (uint)1 << true;
    QTest::newRow("below minimum") << (uint)4 << (uint)2 << (uint)0 << (uint)2 << false;
    QTest::newRow("unchanged")     << (uint)4 << (uint)2 << (uint)2 << (uint)2 << false;
}

void TestVirtualDesktops::current()
{
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    QCOMPARE(vds->current(), (uint)0);
    QFETCH(uint, count);
    vds->setCount(count);
    QFETCH(uint, init);
    QVERIFY(vds->setCurrent(init));
    QCOMPARE(vds->current(), init);

    QSignalSpy spy(vds, SIGNAL(currentChanged(uint,uint)));

    QFETCH(uint, request);
    QFETCH(uint, result);
    QFETCH(bool, signal);
    QCOMPARE(vds->setCurrent(request), signal);
    QCOMPARE(vds->current(), result);
    QCOMPARE(spy.isEmpty(), !signal);
    if (!spy.isEmpty()) {
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.count(), 2);
        QCOMPARE(arguments.at(0).type(), QVariant::UInt);
        QCOMPARE(arguments.at(1).type(), QVariant::UInt);
        QCOMPARE(arguments.at(0).toUInt(), init);
        QCOMPARE(arguments.at(1).toUInt(), result);
    }
}

void TestVirtualDesktops::currentChangeOnCountChange_data()
{
    QTest::addColumn<uint>("initCount");
    QTest::addColumn<uint>("initCurrent");
    QTest::addColumn<uint>("request");
    QTest::addColumn<uint>("current");
    QTest::addColumn<bool>("signal");

    QTest::newRow("increment")                << (uint)4 << (uint)2 << (uint)5 << (uint)2 << false;
    QTest::newRow("increment on last")        << (uint)4 << (uint)4 << (uint)5 << (uint)4 << false;
    QTest::newRow("decrement")                << (uint)4 << (uint)2 << (uint)3 << (uint)2 << false;
    QTest::newRow("decrement on second last") << (uint)4 << (uint)3 << (uint)3 << (uint)3 << false;
    QTest::newRow("decrement on last")        << (uint)4 << (uint)4 << (uint)3 << (uint)3 << true;
    QTest::newRow("multiple decrement")       << (uint)4 << (uint)2 << (uint)1 << (uint)1 << true;
}

void TestVirtualDesktops::currentChangeOnCountChange()
{
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    QFETCH(uint, initCount);
    QFETCH(uint, initCurrent);
    vds->setCount(initCount);
    vds->setCurrent(initCurrent);

    QSignalSpy spy(vds,  SIGNAL(currentChanged(uint,uint)));

    QFETCH(uint, request);
    QFETCH(uint, current);
    QFETCH(bool, signal);

    vds->setCount(request);
    QCOMPARE(vds->current(), current);
    QCOMPARE(spy.isEmpty(), !signal);
}

void TestVirtualDesktops::addDirectionColumns()
{
    QTest::addColumn<uint>("initCount");
    QTest::addColumn<uint>("initCurrent");
    QTest::addColumn<bool>("wrap");
    QTest::addColumn<uint>("result");
}

template <typename T>
void TestVirtualDesktops::testDirection(const QString &actionName)
{
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    QFETCH(uint, initCount);
    QFETCH(uint, initCurrent);
    vds->setCount(initCount);
    vds->setCurrent(initCurrent);

    QFETCH(bool, wrap);
    QFETCH(uint, result);
    T functor;
    QCOMPARE(functor(0, wrap), result);

    vds->setNavigationWrappingAround(wrap);
    QScopedPointer<KActionCollection> keys(new KActionCollection(this));
    vds->initShortcuts(keys.data());
    QAction *action = keys->action(actionName);
    QVERIFY(action);
    QVERIFY(static_cast<KAction*>(action)->isGlobalShortcutEnabled());
    action->trigger();
    QCOMPARE(vds->current(), result);
    QCOMPARE(functor(initCurrent, wrap), result);
}

void TestVirtualDesktops::next_data()
{
    addDirectionColumns();

    QTest::newRow("one desktop, wrap")        << (uint)1 << (uint)1 << true  << (uint)1;
    QTest::newRow("one desktop, no wrap")     << (uint)1 << (uint)1 << false << (uint)1;
    QTest::newRow("desktops, wrap")           << (uint)4 << (uint)1 << true  << (uint)2;
    QTest::newRow("desktops, no wrap")        << (uint)4 << (uint)1 << false << (uint)2;
    QTest::newRow("desktops at end, wrap")    << (uint)4 << (uint)4 << true  << (uint)1;
    QTest::newRow("desktops at end, no wrap") << (uint)4 << (uint)4 << false << (uint)4;
}

void TestVirtualDesktops::next()
{
    testDirection<DesktopNext>("Switch to Next Desktop");
}

void TestVirtualDesktops::previous_data()
{
    addDirectionColumns();

    QTest::newRow("one desktop, wrap")          << (uint)1 << (uint)1 << true  << (uint)1;
    QTest::newRow("one desktop, no wrap")       << (uint)1 << (uint)1 << false << (uint)1;
    QTest::newRow("desktops, wrap")             << (uint)4 << (uint)3 << true  << (uint)2;
    QTest::newRow("desktops, no wrap")          << (uint)4 << (uint)3 << false << (uint)2;
    QTest::newRow("desktops at start, wrap")    << (uint)4 << (uint)1 << true  << (uint)4;
    QTest::newRow("desktops at start, no wrap") << (uint)4 << (uint)1 << false << (uint)1;
}

void TestVirtualDesktops::previous()
{
    testDirection<DesktopPrevious>("Switch to Previous Desktop");
}

void TestVirtualDesktops::left_data()
{
    addDirectionColumns();
    QTest::newRow("one desktop, wrap")          << (uint)1 << (uint)1 << true  << (uint)1;
    QTest::newRow("one desktop, no wrap")       << (uint)1 << (uint)1 << false << (uint)1;
    QTest::newRow("desktops, wrap, 1st row")    << (uint)4 << (uint)2 << true  << (uint)1;
    QTest::newRow("desktops, no wrap, 1st row") << (uint)4 << (uint)2 << false << (uint)1;
    QTest::newRow("desktops, wrap, 2nd row")    << (uint)4 << (uint)4 << true  << (uint)3;
    QTest::newRow("desktops, no wrap, 2nd row") << (uint)4 << (uint)4 << false << (uint)3;

    QTest::newRow("desktops at start, wrap, 1st row")    << (uint)4 << (uint)1 << true  << (uint)2;
    QTest::newRow("desktops at start, no wrap, 1st row") << (uint)4 << (uint)1 << false << (uint)1;
    QTest::newRow("desktops at start, wrap, 2nd row")    << (uint)4 << (uint)3 << true  << (uint)4;
    QTest::newRow("desktops at start, no wrap, 2nd row") << (uint)4 << (uint)3 << false << (uint)3;

    QTest::newRow("non symmetric, start") << (uint)5 << (uint)5 << false << (uint)4;
    QTest::newRow("non symmetric, end, no wrap") << (uint)5 << (uint)4 << false << (uint)4;
    QTest::newRow("non symmetric, end, wrap") << (uint)5 << (uint)4 << true << (uint)5;
}

void TestVirtualDesktops::left()
{
    testDirection<DesktopLeft>("Switch One Desktop to the Left");
}

void TestVirtualDesktops::right_data()
{
    addDirectionColumns();
    QTest::newRow("one desktop, wrap")          << (uint)1 << (uint)1 << true  << (uint)1;
    QTest::newRow("one desktop, no wrap")       << (uint)1 << (uint)1 << false << (uint)1;
    QTest::newRow("desktops, wrap, 1st row")    << (uint)4 << (uint)1 << true  << (uint)2;
    QTest::newRow("desktops, no wrap, 1st row") << (uint)4 << (uint)1 << false << (uint)2;
    QTest::newRow("desktops, wrap, 2nd row")    << (uint)4 << (uint)3 << true  << (uint)4;
    QTest::newRow("desktops, no wrap, 2nd row") << (uint)4 << (uint)3 << false << (uint)4;

    QTest::newRow("desktops at start, wrap, 1st row")    << (uint)4 << (uint)2 << true  << (uint)1;
    QTest::newRow("desktops at start, no wrap, 1st row") << (uint)4 << (uint)2 << false << (uint)2;
    QTest::newRow("desktops at start, wrap, 2nd row")    << (uint)4 << (uint)4 << true  << (uint)3;
    QTest::newRow("desktops at start, no wrap, 2nd row") << (uint)4 << (uint)4 << false << (uint)4;

    QTest::newRow("non symmetric, start") << (uint)5 << (uint)4 << false << (uint)5;
    QTest::newRow("non symmetric, end, no wrap") << (uint)5 << (uint)5 << false << (uint)5;
    QTest::newRow("non symmetric, end, wrap") << (uint)5 << (uint)5 << true << (uint)4;
}

void TestVirtualDesktops::right()
{
    testDirection<DesktopRight>("Switch One Desktop to the Right");
}

void TestVirtualDesktops::above_data()
{
    addDirectionColumns();
    QTest::newRow("one desktop, wrap")             << (uint)1 << (uint)1 << true  << (uint)1;
    QTest::newRow("one desktop, no wrap")          << (uint)1 << (uint)1 << false << (uint)1;
    QTest::newRow("desktops, wrap, 1st column")    << (uint)4 << (uint)3 << true  << (uint)1;
    QTest::newRow("desktops, no wrap, 1st column") << (uint)4 << (uint)3 << false << (uint)1;
    QTest::newRow("desktops, wrap, 2nd column")    << (uint)4 << (uint)4 << true  << (uint)2;
    QTest::newRow("desktops, no wrap, 2nd column") << (uint)4 << (uint)4 << false << (uint)2;

    QTest::newRow("desktops at start, wrap, 1st column")    << (uint)4 << (uint)1 << true  << (uint)3;
    QTest::newRow("desktops at start, no wrap, 1st column") << (uint)4 << (uint)1 << false << (uint)1;
    QTest::newRow("desktops at start, wrap, 2nd column")    << (uint)4 << (uint)2 << true  << (uint)4;
    QTest::newRow("desktops at start, no wrap, 2nd column") << (uint)4 << (uint)2 << false << (uint)2;
}

void TestVirtualDesktops::above()
{
    testDirection<DesktopAbove>("Switch One Desktop Up");
}

void TestVirtualDesktops::below_data()
{
    addDirectionColumns();
    QTest::newRow("one desktop, wrap")             << (uint)1 << (uint)1 << true  << (uint)1;
    QTest::newRow("one desktop, no wrap")          << (uint)1 << (uint)1 << false << (uint)1;
    QTest::newRow("desktops, wrap, 1st column")    << (uint)4 << (uint)1 << true  << (uint)3;
    QTest::newRow("desktops, no wrap, 1st column") << (uint)4 << (uint)1 << false << (uint)3;
    QTest::newRow("desktops, wrap, 2nd column")    << (uint)4 << (uint)2 << true  << (uint)4;
    QTest::newRow("desktops, no wrap, 2nd column") << (uint)4 << (uint)2 << false << (uint)4;

    QTest::newRow("desktops at start, wrap, 1st column")    << (uint)4 << (uint)3 << true  << (uint)1;
    QTest::newRow("desktops at start, no wrap, 1st column") << (uint)4 << (uint)3 << false << (uint)3;
    QTest::newRow("desktops at start, wrap, 2nd column")    << (uint)4 << (uint)4 << true  << (uint)2;
    QTest::newRow("desktops at start, no wrap, 2nd column") << (uint)4 << (uint)4 << false << (uint)4;
}

void TestVirtualDesktops::below()
{
    testDirection<DesktopBelow>("Switch One Desktop Down");
}

void TestVirtualDesktops::updateGrid_data()
{
    QTest::addColumn<uint>("initCount");
    QTest::addColumn<QSize>("size");
    QTest::addColumn<Qt::Orientation>("orientation");
    QTest::addColumn<QPoint>("coords");
    QTest::addColumn<uint>("desktop");
    const Qt::Orientation h = Qt::Horizontal;
    const Qt::Orientation v = Qt::Vertical;

    QTest::newRow("one desktop, h")    << (uint)1 << QSize(1, 1) << h << QPoint(0, 0) << (uint)1;
    QTest::newRow("one desktop, v")    << (uint)1 << QSize(1, 1) << v << QPoint(0, 0) << (uint)1;
    QTest::newRow("one desktop, h, 0") << (uint)1 << QSize(1, 1) << h << QPoint(1, 0) << (uint)0;
    QTest::newRow("one desktop, v, 0") << (uint)1 << QSize(1, 1) << v << QPoint(0, 1) << (uint)0;

    QTest::newRow("two desktops, h, 1") << (uint)2 << QSize(2, 1) << h << QPoint(0, 0) << (uint)1;
    QTest::newRow("two desktops, h, 2") << (uint)2 << QSize(2, 1) << h << QPoint(1, 0) << (uint)2;
    QTest::newRow("two desktops, h, 3") << (uint)2 << QSize(2, 1) << h << QPoint(0, 1) << (uint)0;
    QTest::newRow("two desktops, h, 4") << (uint)2 << QSize(2, 1) << h << QPoint(2, 0) << (uint)0;

    QTest::newRow("two desktops, v, 1") << (uint)2 << QSize(2, 1) << v << QPoint(0, 0) << (uint)1;
    QTest::newRow("two desktops, v, 2") << (uint)2 << QSize(2, 1) << v << QPoint(1, 0) << (uint)2;
    QTest::newRow("two desktops, v, 3") << (uint)2 << QSize(2, 1) << v << QPoint(0, 1) << (uint)0;
    QTest::newRow("two desktops, v, 4") << (uint)2 << QSize(2, 1) << v << QPoint(2, 0) << (uint)0;

    QTest::newRow("four desktops, h, one row, 1") << (uint)4 << QSize(4, 1) << h << QPoint(0, 0) << (uint)1;
    QTest::newRow("four desktops, h, one row, 2") << (uint)4 << QSize(4, 1) << h << QPoint(1, 0) << (uint)2;
    QTest::newRow("four desktops, h, one row, 3") << (uint)4 << QSize(4, 1) << h << QPoint(2, 0) << (uint)3;
    QTest::newRow("four desktops, h, one row, 4") << (uint)4 << QSize(4, 1) << h << QPoint(3, 0) << (uint)4;

    QTest::newRow("four desktops, v, one column, 1") << (uint)4 << QSize(1, 4) << v << QPoint(0, 0) << (uint)1;
    QTest::newRow("four desktops, v, one column, 2") << (uint)4 << QSize(1, 4) << v << QPoint(0, 1) << (uint)2;
    QTest::newRow("four desktops, v, one column, 3") << (uint)4 << QSize(1, 4) << v << QPoint(0, 2) << (uint)3;
    QTest::newRow("four desktops, v, one column, 4") << (uint)4 << QSize(1, 4) << v << QPoint(0, 3) << (uint)4;

    QTest::newRow("four desktops, h, grid, 1") << (uint)4 << QSize(2, 2) << h << QPoint(0, 0) << (uint)1;
    QTest::newRow("four desktops, h, grid, 2") << (uint)4 << QSize(2, 2) << h << QPoint(1, 0) << (uint)2;
    QTest::newRow("four desktops, h, grid, 3") << (uint)4 << QSize(2, 2) << h << QPoint(0, 1) << (uint)3;
    QTest::newRow("four desktops, h, grid, 4") << (uint)4 << QSize(2, 2) << h << QPoint(1, 1) << (uint)4;
    QTest::newRow("four desktops, h, grid, 0/3") << (uint)4 << QSize(2, 2) << h << QPoint(0, 3) << (uint)0;

    QTest::newRow("three desktops, h, grid, 1") << (uint)3 << QSize(2, 2) << h << QPoint(0, 0) << (uint)1;
    QTest::newRow("three desktops, h, grid, 2") << (uint)3 << QSize(2, 2) << h << QPoint(1, 0) << (uint)2;
    QTest::newRow("three desktops, h, grid, 3") << (uint)3 << QSize(2, 2) << h << QPoint(0, 1) << (uint)3;
    QTest::newRow("three desktops, h, grid, 4") << (uint)3 << QSize(2, 2) << h << QPoint(1, 1) << (uint)0;
}

void TestVirtualDesktops::updateGrid()
{
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    QFETCH(uint, initCount);
    vds->setCount(initCount);
    VirtualDesktopGrid grid;

    QFETCH(QSize, size);
    QFETCH(Qt::Orientation, orientation);
    grid.update(size, orientation);
    QCOMPARE(grid.size(), size);
    QCOMPARE(grid.width(), size.width());
    QCOMPARE(grid.height(), size.height());
    QFETCH(QPoint, coords);
    QFETCH(uint, desktop);
    QCOMPARE(grid.at(coords), desktop);
    if (desktop != 0) {
        QCOMPARE(grid.gridCoords(desktop), coords);
    }
}

void TestVirtualDesktops::updateLayout_data()
{
    QTest::addColumn<uint>("desktop");
    QTest::addColumn<QSize>("result");

    QTest::newRow("01") << (uint)1  << QSize(1, 2);
    QTest::newRow("02") << (uint)2  << QSize(1, 2);
    QTest::newRow("03") << (uint)3  << QSize(2, 2);
    QTest::newRow("04") << (uint)4  << QSize(2, 2);
    QTest::newRow("05") << (uint)5  << QSize(3, 2);
    QTest::newRow("06") << (uint)6  << QSize(3, 2);
    QTest::newRow("07") << (uint)7  << QSize(4, 2);
    QTest::newRow("08") << (uint)8  << QSize(4, 2);
    QTest::newRow("09") << (uint)9  << QSize(5, 2);
    QTest::newRow("10") << (uint)10 << QSize(5, 2);
    QTest::newRow("11") << (uint)11 << QSize(6, 2);
    QTest::newRow("12") << (uint)12 << QSize(6, 2);
    QTest::newRow("13") << (uint)13 << QSize(7, 2);
    QTest::newRow("14") << (uint)14 << QSize(7, 2);
    QTest::newRow("15") << (uint)15 << QSize(8, 2);
    QTest::newRow("16") << (uint)16 << QSize(8, 2);
    QTest::newRow("17") << (uint)17 << QSize(9, 2);
    QTest::newRow("18") << (uint)18 << QSize(9, 2);
    QTest::newRow("19") << (uint)19 << QSize(10, 2);
    QTest::newRow("20") << (uint)20 << QSize(10, 2);
}

void TestVirtualDesktops::updateLayout()
{
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    QSignalSpy spy(vds, SIGNAL(layoutChanged(int,int)));
    // call update layout - implicitly through setCount
    QFETCH(uint, desktop);
    QFETCH(QSize, result);
    vds->setCount(desktop);
    QCOMPARE(vds->grid().size(), result);
    QCOMPARE(spy.count(), 1);
    const QVariantList &arguments = spy.at(0);
    QCOMPARE(arguments.at(0).toInt(), result.width());
    QCOMPARE(arguments.at(1).toInt(), result.height());
    // calling update layout again should not change anything
    vds->updateLayout();
    QCOMPARE(vds->grid().size(), result);
    QCOMPARE(spy.count(), 2);
    const QVariantList &arguments2 = spy.at(1);
    QCOMPARE(arguments2.at(0).toInt(), result.width());
    QCOMPARE(arguments2.at(1).toInt(), result.height());
}

void TestVirtualDesktops::name_data()
{
    QTest::addColumn<uint>("initCount");
    QTest::addColumn<uint>("desktop");
    QTest::addColumn<QString>("desktopName");

    QTest::newRow("desktop 1") << (uint)4 << (uint)1 << "Desktop 1";
    QTest::newRow("desktop 2") << (uint)4 << (uint)2 << "Desktop 2";
    QTest::newRow("desktop 3") << (uint)4 << (uint)3 << "Desktop 3";
    QTest::newRow("desktop 4") << (uint)4 << (uint)4 << "Desktop 4";
    QTest::newRow("desktop 5") << (uint)4 << (uint)5 << "Desktop 5";
}

void TestVirtualDesktops::name()
{
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    QFETCH(uint, initCount);
    vds->setCount(initCount);
    QFETCH(uint, desktop);

    QTEST(vds->name(desktop), "desktopName");
}

void TestVirtualDesktops::switchToShortcuts()
{
    QScopedPointer<KActionCollection> keys(new KActionCollection(this));
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    vds->setCount(vds->maximum());
    vds->setCurrent(vds->maximum());
    QCOMPARE(vds->current(), vds->maximum());
    vds->initShortcuts(keys.data());
    const QString toDesktop = "Switch to Desktop %1";
    for (uint i=1; i<=vds->maximum(); ++i) {
        const QString desktop(toDesktop.arg(i));
        QAction *action = keys->action(desktop);
        QVERIFY2(action, desktop.toUtf8().constData());
        QVERIFY(static_cast<KAction*>(action)->isGlobalShortcutEnabled());
        action->trigger();
        QCOMPARE(vds->current(), i);
    }
    // test switchTo with incorrect data in QAction
    KAction *action = keys->addAction("wrong", vds, SLOT(slotSwitchTo()));
    action->trigger();
    // should still be on max
    QCOMPARE(vds->current(), vds->maximum());
    // invoke switchTo not from a QAction
    QMetaObject::invokeMethod(vds, "slotSwitchTo");
    // should still be on max
    QCOMPARE(vds->current(), vds->maximum());
}

void TestVirtualDesktops::load()
{
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    // no config yet, load should not change anything
    vds->load();
    QCOMPARE(vds->count(), (uint)0);
    // empty config should create one desktop
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    vds->setConfig(config);
    vds->load();
    QCOMPARE(vds->count(), (uint)1);
    // setting a sensible number
    config->group("Desktops").writeEntry("Number", 4);
    vds->load();
    QCOMPARE(vds->count(), (uint)4);
    // setting the screen number should reset to one desktop as config value is missing
    screen_number = 2;
    vds->load();
    QCOMPARE(vds->count(), (uint)1);
    // creating the respective group should properly load
    config->group("Desktops-screen-2").writeEntry("Number", 5);
    vds->load();
    QCOMPARE(vds->count(), (uint)5);
}

void TestVirtualDesktops::save()
{
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    vds->setCount(4);
    // no config yet, just to ensure it actually works
    vds->save();
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    vds->setConfig(config);

    // now save should create the group "Desktops"
    QCOMPARE(config->hasGroup("Desktops"), false);
    vds->save();
    QCOMPARE(config->hasGroup("Desktops"), true);
    KConfigGroup desktops = config->group("Desktops");
    QCOMPARE(desktops.readEntry<int>("Number", 1), 4);
    QCOMPARE(desktops.hasKey("Name_1"), false);
    QCOMPARE(desktops.hasKey("Name_2"), false);
    QCOMPARE(desktops.hasKey("Name_3"), false);
    QCOMPARE(desktops.hasKey("Name_4"), false);

    // change screen number
    screen_number = 3;
    QCOMPARE(config->hasGroup("Desktops-screen-3"), false);
    vds->setCount(3);
    vds->save();
    QCOMPARE(config->hasGroup("Desktops-screen-3"), true);
    // old one should be unchanged
    desktops = config->group("Desktops");
    QCOMPARE(desktops.readEntry<int>("Number", 1), 4);
    desktops = config->group("Desktops-screen-3");
    QCOMPARE(desktops.readEntry<int>("Number", 1), 3);
    QCOMPARE(desktops.hasKey("Name_1"), false);
    QCOMPARE(desktops.hasKey("Name_2"), false);
    QCOMPARE(desktops.hasKey("Name_3"), false);
    QCOMPARE(desktops.hasKey("Name_4"), false);

}

QTEST_MAIN(TestVirtualDesktops)
#include "test_virtual_desktops.moc"
