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
#include "effects.h"
// KWin libs
#include <kwinxrenderutils.h>
// xcb
#include <xcb/render.h>

namespace KWin {

Outline::Outline()
    : m_initialized(false)
    , m_active(false)
{
}

Outline::~Outline()
{
}

void Outline::show()
{
    m_active = true;
    if (effects && static_cast<EffectsHandlerImpl*>(effects)->provides(Effect::Outline)) {
        static_cast<EffectsHandlerImpl*>(effects)->slotShowOutline(m_outlineGeometry);
        return; // done by effect
    }
    showWithX();
}

void Outline::hide()
{
    if (!m_active) {
        return;
    }
    m_active = false;
    if (effects && static_cast<EffectsHandlerImpl*>(effects)->provides(Effect::Outline)) {
        static_cast<EffectsHandlerImpl*>(effects)->slotHideOutline();
        return; // done by effect
    }
    forEachWindow(&Xcb::Window::unmap);
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

QVector< xcb_window_t > Outline::windowIds() const
{
    QVector<xcb_window_t> windows;
    if (m_initialized) {
        windows.reserve(4);
        windows << m_leftOutline << m_topOutline << m_rightOutline << m_bottomOutline;
    }
    return windows;
}

void Outline::showWithX()
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

// left/right parts are between top/bottom, they don't reach as far as the corners
    const uint16_t verticalWidth = 5;
    const uint16_t verticalHeight = m_outlineGeometry.height() - 10;
    const uint16_t horizontalWidth = m_outlineGeometry.width();
    const uint horizontalHeight = 5;
    m_leftOutline.setGeometry(m_outlineGeometry.x(), m_outlineGeometry.y() + 5, verticalWidth, verticalHeight);
    m_rightOutline.setGeometry(m_outlineGeometry.x() + m_outlineGeometry.width() - 5, m_outlineGeometry.y() + 5, verticalWidth, verticalHeight);
    m_topOutline.setGeometry(m_outlineGeometry.x(), m_outlineGeometry.y(), horizontalWidth, horizontalHeight);
    m_bottomOutline.setGeometry(m_outlineGeometry.x(), m_outlineGeometry.y() + m_outlineGeometry.height() - 5, horizontalWidth, horizontalHeight);

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
        xcb_create_pixmap(connection(), defaultDepth, xpix, rootWindow(), m_outlineGeometry.width(), 5);
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

} // namespace
