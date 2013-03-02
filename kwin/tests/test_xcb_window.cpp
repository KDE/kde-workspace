/********************************************************************
KWin - the KDE window manager
This file is part of the KDE project.

Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>

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
#include "testutils.h"
// KWin
#include "../xcbutils.h"
// Qt
#include <QApplication>
#include <QtTest/QtTest>
// xcb
#include <xcb/xcb.h>

using namespace KWin;

class TestXcbWindow : public QObject
{
    Q_OBJECT
private slots:
    void defaultCtor();
    void ctor();
    void classCtor();
    void create();
    void mapUnmap();
    void geometry();
    void destroy();
};

void TestXcbWindow::defaultCtor()
{
    Xcb::Window window;
    QCOMPARE(window.isValid(), false);
    xcb_window_t wId = window;
    QCOMPARE(wId, noneWindow());

    xcb_window_t nativeWindow = createWindow();
    Xcb::Window window2(nativeWindow);
    QCOMPARE(window2.isValid(), true);
    wId = window2;
    QCOMPARE(wId, nativeWindow);
}

void TestXcbWindow::ctor()
{
    const QRect geometry(0, 0, 10, 10);
    const uint32_t values[] = {true};
    Xcb::Window window(geometry, XCB_CW_OVERRIDE_REDIRECT, values);
    QCOMPARE(window.isValid(), true);
    QVERIFY(window != XCB_WINDOW_NONE);
    Xcb::WindowGeometry windowGeometry(window);
    QCOMPARE(windowGeometry.isNull(), false);
    QCOMPARE(windowGeometry.rect(), geometry);
}

void TestXcbWindow::classCtor()
{
    const QRect geometry(0, 0, 10, 10);
    const uint32_t values[] = {true};
    Xcb::Window window(geometry, XCB_WINDOW_CLASS_INPUT_ONLY, XCB_CW_OVERRIDE_REDIRECT, values);
    QCOMPARE(window.isValid(), true);
    QVERIFY(window != XCB_WINDOW_NONE);
    Xcb::WindowGeometry windowGeometry(window);
    QCOMPARE(windowGeometry.isNull(), false);
    QCOMPARE(windowGeometry.rect(), geometry);

    Xcb::WindowAttributes attribs(window);
    QCOMPARE(attribs.isNull(), false);
    QVERIFY(attribs->_class == XCB_WINDOW_CLASS_INPUT_ONLY);
}

void TestXcbWindow::create()
{
    Xcb::Window window;
    QCOMPARE(window.isValid(), false);
    xcb_window_t wId = window;
    QCOMPARE(wId, noneWindow());

    const QRect geometry(0, 0, 10, 10);
    const uint32_t values[] = {true};
    window.create(geometry, XCB_CW_OVERRIDE_REDIRECT, values);
    QCOMPARE(window.isValid(), true);
    QVERIFY(window != XCB_WINDOW_NONE);
}

void TestXcbWindow::mapUnmap()
{
    const QRect geometry(0, 0, 10, 10);
    const uint32_t values[] = {true};
    Xcb::Window window(geometry, XCB_WINDOW_CLASS_INPUT_ONLY, XCB_CW_OVERRIDE_REDIRECT, values);
    Xcb::WindowAttributes attribs(window);
    QCOMPARE(attribs.isNull(), false);
    QVERIFY(attribs->map_state == XCB_MAP_STATE_UNMAPPED);

    window.map();
    Xcb::WindowAttributes attribs2(window);
    QCOMPARE(attribs2.isNull(), false);
    QVERIFY(attribs2->map_state != XCB_MAP_STATE_UNMAPPED);

    window.unmap();
    Xcb::WindowAttributes attribs3(window);
    QCOMPARE(attribs3.isNull(), false);
    QVERIFY(attribs3->map_state == XCB_MAP_STATE_UNMAPPED);
}

void TestXcbWindow::geometry()
{
    const QRect geometry(0, 0, 10, 10);
    const uint32_t values[] = {true};
    Xcb::Window window(geometry, XCB_WINDOW_CLASS_INPUT_ONLY, XCB_CW_OVERRIDE_REDIRECT, values);
    Xcb::WindowGeometry windowGeometry(window);
    QCOMPARE(windowGeometry.isNull(), false);
    QCOMPARE(windowGeometry.rect(), geometry);

    const QRect geometry2(10, 20, 100, 200);
    window.setGeometry(geometry2);
    Xcb::WindowGeometry windowGeometry2(window);
    QCOMPARE(windowGeometry2.isNull(), false);
    QCOMPARE(windowGeometry2.rect(), geometry2);
}

void TestXcbWindow::destroy()
{
    const QRect geometry(0, 0, 10, 10);
    const uint32_t values[] = {true};
    Xcb::Window window(geometry, XCB_CW_OVERRIDE_REDIRECT, values);
    QCOMPARE(window.isValid(), true);
    xcb_window_t wId = window;

    window.create(geometry, XCB_CW_OVERRIDE_REDIRECT, values);
    // wId should now be invalid
    xcb_generic_error_t *error = NULL;
    ScopedCPointer<xcb_get_window_attributes_reply_t> attribs(xcb_get_window_attributes_reply(
        connection(),
        xcb_get_window_attributes(connection(), wId),
        &error));
    QVERIFY(attribs.isNull());
    QCOMPARE(error->error_code, uint8_t(3));
    QCOMPARE(error->resource_id, wId);
    free(error);

    // test the same for the dtor
    {
        Xcb::Window scopedWindow(geometry, XCB_CW_OVERRIDE_REDIRECT, values);
        QVERIFY(scopedWindow.isValid());
        wId = scopedWindow;
    }
    error = NULL;
    ScopedCPointer<xcb_get_window_attributes_reply_t> attribs2(xcb_get_window_attributes_reply(
        connection(),
        xcb_get_window_attributes(connection(), wId),
        &error));
    QVERIFY(attribs2.isNull());
    QCOMPARE(error->error_code, uint8_t(3));
    QCOMPARE(error->resource_id, wId);
    free(error);
}

KWIN_TEST_MAIN(TestXcbWindow)
#include "test_xcb_window.moc"
