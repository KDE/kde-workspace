/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2011 Arthur Arlt <a.arlt@stud.uni-heidelberg.de>
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
// own
#include "outline.h"
// KWin
#include "composite.h"
// KWin libs
#include <kwinxrenderutils.h>
// Plasma
#include <Plasma/FrameSvg>
// Qt
#include <QPainter>
// xcb
#include <xcb/render.h>

namespace KWin {

KWIN_SINGLETON_FACTORY(Outline)

Outline::Outline(QObject *parent)
    : QObject(parent)
    , m_active(false)
{
    connect(Compositor::self(), SIGNAL(compositingToggled(bool)), SLOT(compositingChanged()));
}

Outline::~Outline()
{
}

void Outline::show()
{
    m_active = true;
    if (m_visual.isNull()) {
        createHelper();
    }
    if (m_visual.isNull()) {
        // something went wrong
        return;
    }
    m_visual->show();
}

void Outline::hide()
{
    if (!m_active) {
        return;
    }
    m_active = false;
    if (m_visual.isNull()) {
        return;
    }
    m_visual->hide();
}

void Outline::show(const QRect& outlineGeometry)
{
    setGeometry(outlineGeometry);
    show();
}

void Outline::setGeometry(const QRect& outlineGeometry)
{
    m_outlineGeometry = outlineGeometry;
}

void Outline::createHelper()
{
    if (!m_visual.isNull()) {
        return;
    }
    if (Compositor::compositing()) {
        m_visual.reset(new CompositedOutlineVisual(this));
    } else {
        m_visual.reset(new NonCompositedOutlineVisual(this));
    }
}

void Outline::compositingChanged()
{
    m_visual.reset();
    if (m_active) {
        show();
    }
}

OutlineVisual::OutlineVisual(Outline *outline)
    : m_outline(outline)
{
}

OutlineVisual::~OutlineVisual()
{
}

CompositedOutlineVisual::CompositedOutlineVisual(Outline *outline)
    : QWidget(NULL, Qt::X11BypassWindowManagerHint)
    , OutlineVisual(outline)
    , m_background(new Plasma::FrameSvg(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);
    m_background->setImagePath("widgets/viewitem");
    m_background->setElementPrefix("hover");
    m_background->setCacheAllRenderedFrames(true);
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
}

CompositedOutlineVisual::~CompositedOutlineVisual()
{
}

void CompositedOutlineVisual::hide()
{
    QWidget::hide();
}

void CompositedOutlineVisual::show()
{
    const QRect &outlineGeometry = outline()->geometry();
    m_background->resizeFrame(outlineGeometry.size());
    setGeometry(outlineGeometry);
    QWidget::show();
}

void CompositedOutlineVisual::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    m_background->paintFrame(&painter);
}

NonCompositedOutlineVisual::NonCompositedOutlineVisual(Outline *outline)
    : OutlineVisual(outline)
    , m_initialized(false)
{
}

NonCompositedOutlineVisual::~NonCompositedOutlineVisual()
{
}

void NonCompositedOutlineVisual::show()
{
    if (!m_initialized) {
        const QRect geo(0, 0, 1, 1);
        const uint32_t values[] = {true};
        // TODO: use template variant
        m_leftOutline.create(geo, XCB_CW_OVERRIDE_REDIRECT, values);
        m_rightOutline.create(geo, XCB_CW_OVERRIDE_REDIRECT, values);
        m_topOutline.create(geo, XCB_CW_OVERRIDE_REDIRECT, values);
        m_bottomOutline.create(geo, XCB_CW_OVERRIDE_REDIRECT, values);
        m_initialized   = true;
    }

    const int defaultDepth = Xcb::defaultDepth();

    const QRect &outlineGeometry = outline()->geometry();
    // left/right parts are between top/bottom, they don't reach as far as the corners
    const uint16_t verticalWidth = 5;
    const uint16_t verticalHeight = outlineGeometry.height() - 10;
    const uint16_t horizontalWidth = outlineGeometry.width();
    const uint horizontalHeight = 5;
    m_leftOutline.setGeometry(outlineGeometry.x(), outlineGeometry.y() + 5, verticalWidth, verticalHeight);
    m_rightOutline.setGeometry(outlineGeometry.x() + outlineGeometry.width() - 5, outlineGeometry.y() + 5, verticalWidth, verticalHeight);
    m_topOutline.setGeometry(outlineGeometry.x(), outlineGeometry.y(), horizontalWidth, horizontalHeight);
    m_bottomOutline.setGeometry(outlineGeometry.x(), outlineGeometry.y() + outlineGeometry.height() - 5, horizontalWidth, horizontalHeight);

    const xcb_render_color_t white = {0xffff, 0xffff, 0xffff, 0xffff};
    QColor qGray(Qt::gray);
    const xcb_render_color_t gray = {
        uint16_t(0xffff * qGray.redF()),
        uint16_t(0xffff * qGray.greenF()),
        uint16_t(0xffff * qGray.blueF()),
        0xffff
    };
    const xcb_render_color_t black = {0, 0, 0, 0xffff};
    {
        xcb_pixmap_t xpix = xcb_generate_id(connection());
        xcb_create_pixmap(connection(), defaultDepth, xpix, rootWindow(), verticalWidth, verticalHeight);
        XRenderPicture pic(xpix, defaultDepth);

        xcb_rectangle_t rect = {0, 0, 5, verticalHeight};
        xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, pic, white, 1, &rect);
        rect.x = 1;
        rect.width = 3;
        xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, pic, gray, 1, &rect);
        rect.x = 2;
        rect.width = 1;
        xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, pic, black, 1, &rect);

        m_leftOutline.setBackgroundPixmap(xpix);
        m_rightOutline.setBackgroundPixmap(xpix);
        // According to the XSetWindowBackgroundPixmap documentation the pixmap can be freed.
        xcb_free_pixmap(connection(), xpix);
    }
    {
        xcb_pixmap_t xpix = xcb_generate_id(connection());
        xcb_create_pixmap(connection(), defaultDepth, xpix, rootWindow(), horizontalWidth, horizontalHeight);
        XRenderPicture pic(xpix, defaultDepth);

        xcb_rectangle_t rect = {0, 0, horizontalWidth, horizontalHeight};
        xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, pic, white, 1, &rect);
        xcb_rectangle_t grayRects[] = {
            {1, 1, uint16_t(horizontalWidth -2), 3},
            {1, 4, 3, 1},
            {int16_t(horizontalWidth - 4), 4, 3, 1}
        };
        xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, pic, gray, 3, grayRects);
        xcb_rectangle_t blackRects[] = {
            {2, 2, uint16_t(horizontalWidth -4), 1},
            {2, 3, 1, 2},
            {int16_t(horizontalWidth - 3), 3, 1, 2}
        };
        xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, pic, black, 3, blackRects);
        m_topOutline.setBackgroundPixmap(xpix);
        // According to the XSetWindowBackgroundPixmap documentation the pixmap can be freed.
        xcb_free_pixmap(connection(), xpix);
    }
    {
        xcb_pixmap_t xpix = xcb_generate_id(connection());
        xcb_create_pixmap(connection(), defaultDepth, xpix, rootWindow(), outlineGeometry.width(), 5);
        XRenderPicture pic(xpix, defaultDepth);

        xcb_rectangle_t rect = {0, 0, horizontalWidth, horizontalHeight};
        xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, pic, white, 1, &rect);
        xcb_rectangle_t grayRects[] = {
            {1, 1, uint16_t(horizontalWidth -2), 3},
            {1, 0, 3, 1},
            {int16_t(horizontalWidth - 4), 0, 3, 1}
        };
        xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, pic, gray, 3, grayRects);
        xcb_rectangle_t blackRects[] = {
            {2, 2, uint16_t(horizontalWidth -4), 1},
            {2, 0, 1, 2},
            {int16_t(horizontalWidth - 3), 0, 1, 2}
        };
        xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, pic, black, 3, blackRects);
        m_bottomOutline.setBackgroundPixmap(xpix);
        // According to the XSetWindowBackgroundPixmap documentation the pixmap can be freed.
        xcb_free_pixmap(connection(), xpix);
    }
    forEachWindow(&Xcb::Window::clear);
    forEachWindow(&Xcb::Window::map);
}

void NonCompositedOutlineVisual::hide()
{
    forEachWindow(&Xcb::Window::unmap);
}

} // namespace
