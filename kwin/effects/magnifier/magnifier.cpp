/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2007 Christian Nitschkowski <christian.nitschkowski@kdemail.net>
Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

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

#include "magnifier.h"
// KConfigSkeleton
#include "magnifierconfig.h"

#include <kwinconfig.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>

#include <kwinglutils.h>
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
#include <kwinxrenderutils.h>
#include <xcb/render.h>
#endif

namespace KWin
{

KWIN_EFFECT(magnifier, MagnifierEffect)
KWIN_EFFECT_SUPPORTED(magnifier, MagnifierEffect::supported())

const int FRAME_WIDTH = 5;

MagnifierEffect::MagnifierEffect()
    : zoom(1)
    , target_zoom(1)
    , polling(false)
    , m_texture(0)
    , m_fbo(0)
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
    , m_pixmap(XCB_PIXMAP_NONE)
#endif
{
    KActionCollection* actionCollection = new KActionCollection(this);
    KAction* a;
    a = static_cast< KAction* >(actionCollection->addAction(KStandardAction::ZoomIn, this, SLOT(zoomIn())));
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Equal));
    a = static_cast< KAction* >(actionCollection->addAction(KStandardAction::ZoomOut, this, SLOT(zoomOut())));
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Minus));
    a = static_cast< KAction* >(actionCollection->addAction(KStandardAction::ActualSize, this, SLOT(toggle())));
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_0));
    connect(effects, SIGNAL(mouseChanged(QPoint,QPoint,Qt::MouseButtons,Qt::MouseButtons,Qt::KeyboardModifiers,Qt::KeyboardModifiers)),
            this, SLOT(slotMouseChanged(QPoint,QPoint,Qt::MouseButtons,Qt::MouseButtons,Qt::KeyboardModifiers,Qt::KeyboardModifiers)));
    reconfigure(ReconfigureAll);
}

MagnifierEffect::~MagnifierEffect()
{
    delete m_fbo;
    delete m_texture;
    destroyPixmap();
    // Save the zoom value.
    KConfigGroup conf = EffectsHandler::effectConfig("Magnifier");
    conf.writeEntry("InitialZoom", target_zoom);
    conf.sync();
}

void MagnifierEffect::destroyPixmap()
{
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
    if (effects->compositingType() != XRenderCompositing) {
        return;
    }
    m_picture.reset();
    if (m_pixmap != XCB_PIXMAP_NONE) {
        xcb_free_pixmap(connection(), m_pixmap);
        m_pixmap = XCB_PIXMAP_NONE;
    }
#endif
}

bool MagnifierEffect::supported()
{
    return  effects->compositingType() == XRenderCompositing ||
            (effects->isOpenGLCompositing() && GLRenderTarget::blitSupported());
}

void MagnifierEffect::reconfigure(ReconfigureFlags)
{
    MagnifierConfig::self()->readConfig();
    int width, height;
    width = MagnifierConfig::width();
    height = MagnifierConfig::height();
    magnifier_size = QSize(width, height);
    // Load the saved zoom value.
    target_zoom = MagnifierConfig::initialZoom();
    if (target_zoom != zoom)
        toggle();
}

void MagnifierEffect::prePaintScreen(ScreenPrePaintData& data, int time)
{
    if (zoom != target_zoom) {
        double diff = time / animationTime(500.0);
        if (target_zoom > zoom)
            zoom = qMin(zoom * qMax(1 + diff, 1.2), target_zoom);
        else {
            zoom = qMax(zoom * qMin(1 - diff, 0.8), target_zoom);
            if (zoom == 1.0) {
                // zoom ended - delete FBO and texture
                delete m_fbo;
                delete m_texture;
                m_fbo = NULL;
                m_texture = NULL;
                destroyPixmap();
            }
        }
    }
    effects->prePaintScreen(data, time);
    if (zoom != 1.0)
        data.paint |= magnifierArea().adjusted(-FRAME_WIDTH, -FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH);
}

void MagnifierEffect::paintScreen(int mask, QRegion region, ScreenPaintData& data)
{
    effects->paintScreen(mask, region, data);   // paint normal screen
    if (zoom != 1.0) {
        // get the right area from the current rendered screen
        const QRect area = magnifierArea();
        const QPoint cursor = cursorPos();

        QRect srcArea(cursor.x() - (double)area.width() / (zoom*2),
                      cursor.y() - (double)area.height() / (zoom*2),
                      (double)area.width() / zoom, (double)area.height() / zoom);
        if (effects->isOpenGLCompositing()) {
            m_fbo->blitFromFramebuffer(srcArea);
            // paint magnifier
            m_texture->bind();
            m_texture->render(infiniteRegion(), area);
            m_texture->unbind();
            QVector<float> verts;
            GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
            vbo->reset();
            vbo->setColor(QColor(0, 0, 0));
            // top frame
            verts << area.right() + FRAME_WIDTH << area.top() - FRAME_WIDTH;
            verts << area.left() - FRAME_WIDTH << area.top() - FRAME_WIDTH;
            verts << area.left() - FRAME_WIDTH << area.top() - 1;
            verts << area.left() - FRAME_WIDTH << area.top() - 1;
            verts << area.right() + FRAME_WIDTH << area.top() - 1;
            verts << area.right() + FRAME_WIDTH << area.top() - FRAME_WIDTH;
            // left frame
            verts << area.left() - 1 << area.top() - FRAME_WIDTH;
            verts << area.left() - FRAME_WIDTH << area.top() - FRAME_WIDTH;
            verts << area.left() - FRAME_WIDTH << area.bottom() + FRAME_WIDTH;
            verts << area.left() - FRAME_WIDTH << area.bottom() + FRAME_WIDTH;
            verts << area.left() - 1 << area.bottom() + FRAME_WIDTH;
            verts << area.left() - 1 << area.top() - FRAME_WIDTH;
            // right frame
            verts << area.right() + FRAME_WIDTH << area.top() - FRAME_WIDTH;
            verts << area.right() + 1 << area.top() - FRAME_WIDTH;
            verts << area.right() + 1 << area.bottom() + FRAME_WIDTH;
            verts << area.right() + 1 << area.bottom() + FRAME_WIDTH;
            verts << area.right() + FRAME_WIDTH << area.bottom() + FRAME_WIDTH;
            verts << area.right() + FRAME_WIDTH << area.top() - FRAME_WIDTH;
            // bottom frame
            verts << area.right() + FRAME_WIDTH << area.bottom() + 1;
            verts << area.left() - FRAME_WIDTH << area.bottom() + 1;
            verts << area.left() - FRAME_WIDTH << area.bottom() + FRAME_WIDTH;
            verts << area.left() - FRAME_WIDTH << area.bottom() + FRAME_WIDTH;
            verts << area.right() + FRAME_WIDTH << area.bottom() + FRAME_WIDTH;
            verts << area.right() + FRAME_WIDTH << area.bottom() + 1;
            vbo->setData(verts.size() / 2, 2, verts.constData(), NULL);

            ShaderBinder binder(ShaderManager::ColorShader);
            vbo->render(GL_TRIANGLES);
        }
        if (effects->compositingType() == XRenderCompositing) {
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
            if (m_pixmap == XCB_PIXMAP_NONE || m_pixmapSize != srcArea.size()) {
                destroyPixmap();
                m_pixmap = xcb_generate_id(connection());
                m_pixmapSize = srcArea.size();
                xcb_create_pixmap(connection(), 32, m_pixmap, rootWindow(), m_pixmapSize.width(), m_pixmapSize.height());
                m_picture.reset(new XRenderPicture(m_pixmap, 32));
            }
#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))
            static xcb_render_transform_t identity = {
                DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0),
                DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0),
                DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1)
            };
            static xcb_render_transform_t xform = {
                DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0),
                DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0),
                DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1)
            };
            xcb_render_composite(connection(), XCB_RENDER_PICT_OP_SRC, effects->xrenderBufferPicture(), 0, *m_picture,
                                srcArea.x(), srcArea.y(), 0, 0, 0, 0, srcArea.width(), srcArea.height());
            xcb_flush(connection());
            xform.matrix11 = DOUBLE_TO_FIXED(1.0/zoom);
            xform.matrix22 = DOUBLE_TO_FIXED(1.0/zoom);
#undef DOUBLE_TO_FIXED
            xcb_render_set_picture_transform(connection(), *m_picture, xform);
            xcb_render_set_picture_filter(connection(), *m_picture, 4, const_cast<char*>("good"), 0, NULL);
            xcb_render_composite(connection(), XCB_RENDER_PICT_OP_SRC, *m_picture, 0, effects->xrenderBufferPicture(),
                                 0, 0, 0, 0, area.x(), area.y(), area.width(), area.height() );
            xcb_render_set_picture_filter(connection(), *m_picture, 4, const_cast<char*>("fast"), 0, NULL);
            xcb_render_set_picture_transform(connection(), *m_picture, identity);
            const xcb_rectangle_t rects[4] = {
                { int16_t(area.x()+FRAME_WIDTH), int16_t(area.y()), uint16_t(area.width()-FRAME_WIDTH), uint16_t(FRAME_WIDTH)},
                { int16_t(area.right()-FRAME_WIDTH), int16_t(area.y()+FRAME_WIDTH), uint16_t(FRAME_WIDTH), uint16_t(area.height()-FRAME_WIDTH)},
                { int16_t(area.x()), int16_t(area.bottom()-FRAME_WIDTH), uint16_t(area.width()-FRAME_WIDTH), uint16_t(FRAME_WIDTH)},
                { int16_t(area.x()), int16_t(area.y()), uint16_t(FRAME_WIDTH), uint16_t(area.height()-FRAME_WIDTH)}
            };
            xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, effects->xrenderBufferPicture(),
                                       preMultiply(QColor(0,0,0,255)), 4, rects);
#endif
        }
    }
}

void MagnifierEffect::postPaintScreen()
{
    if (zoom != target_zoom) {
        QRect framedarea = magnifierArea().adjusted(-FRAME_WIDTH, -FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH);
        effects->addRepaint(framedarea);
    }
    effects->postPaintScreen();
}

QRect MagnifierEffect::magnifierArea(QPoint pos) const
{
    return QRect(pos.x() - magnifier_size.width() / 2, pos.y() - magnifier_size.height() / 2,
                 magnifier_size.width(), magnifier_size.height());
}

void MagnifierEffect::zoomIn()
{
    target_zoom *= 1.2;
    if (!polling) {
        polling = true;
        effects->startMousePolling();
    }
    if (effects->isOpenGLCompositing() && !m_texture) {
        m_texture = new GLTexture(magnifier_size.width(), magnifier_size.height());
        m_texture->setYInverted(false);
        m_fbo = new GLRenderTarget(*m_texture);
    }
    effects->addRepaint(magnifierArea().adjusted(-FRAME_WIDTH, -FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH));
}

void MagnifierEffect::zoomOut()
{
    target_zoom /= 1.2;
    if (target_zoom <= 1) {
        target_zoom = 1;
        if (polling) {
            polling = false;
            effects->stopMousePolling();
        }
        if (zoom == target_zoom) {
            delete m_fbo;
            delete m_texture;
            m_fbo = NULL;
            m_texture = NULL;
            destroyPixmap();
        }
    }
    effects->addRepaint(magnifierArea().adjusted(-FRAME_WIDTH, -FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH));
}

void MagnifierEffect::toggle()
{
    if (zoom == 1.0) {
        if (target_zoom == 1.0) {
            target_zoom = 2;
        }
        if (!polling) {
            polling = true;
            effects->startMousePolling();
        }
        if (effects->isOpenGLCompositing() && !m_texture) {
            m_texture = new GLTexture(magnifier_size.width(), magnifier_size.height());
            m_texture->setYInverted(false);
            m_fbo = new GLRenderTarget(*m_texture);
        }
    } else {
        target_zoom = 1;
        if (polling) {
            polling = false;
            effects->stopMousePolling();
        }
    }
    effects->addRepaint(magnifierArea().adjusted(-FRAME_WIDTH, -FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH));
}

void MagnifierEffect::slotMouseChanged(const QPoint& pos, const QPoint& old,
                                   Qt::MouseButtons, Qt::MouseButtons, Qt::KeyboardModifiers, Qt::KeyboardModifiers)
{
    if (pos != old && zoom != 1)
        // need full repaint as we might lose some change events on fast mouse movements
        // see Bug 187658
        effects->addRepaintFull();
}

bool MagnifierEffect::isActive() const
{
    return zoom != 1.0 || zoom != target_zoom;
}

} // namespace

#include "magnifier.moc"
