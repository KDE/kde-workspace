/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2009 Fredrik Höglund <fredrik@kde.org>

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

/*
 This is the XRender-based compositing code. The primary compositing
 backend is the OpenGL-based one, which should be more powerful
 and also possibly better documented. This backend is mostly for cases
 when the OpenGL backend cannot be used for some reason (insufficient
 performance, no usable OpenGL support at all, etc.)
 The plan is to keep it around as long as needed/possible, but if it
 proves to be too much hassle it will be dropped in the future.

 Docs:

 XRender (the protocol, but the function calls map to it):
 http://gitweb.freedesktop.org/?p=xorg/proto/renderproto.git;a=blob_plain;hb=HEAD;f=renderproto.txt

 XFixes (again, the protocol):
 http://gitweb.freedesktop.org/?p=xorg/proto/fixesproto.git;a=blob_plain;hb=HEAD;f=fixesproto.txt

*/

#include "scene_xrender.h"

#ifdef KWIN_HAVE_XRENDER_COMPOSITING

#include "toplevel.h"
#include "client.h"
#include "deleted.h"
#include "effects.h"
#include "overlaywindow.h"
#include "paintredirector.h"
#include "kwinxrenderutils.h"

#include <X11/extensions/Xcomposite.h>

#include <kxerrorhandler.h>

#include <QtGui/QPainter>

namespace KWin
{

//****************************************
// SceneXrender
//****************************************

// kDebug() support for the XserverRegion type
struct RegionDebug {
    RegionDebug(XserverRegion r) : rr(r) {}
    XserverRegion rr;
};

QDebug& operator<<(QDebug& stream, RegionDebug r)
{
    if (r.rr == None)
        return stream << "EMPTY";
    int num;
    XRectangle* rects = XFixesFetchRegion(display(), r.rr, &num);
    if (rects == NULL || num == 0)
        return stream << "EMPTY";
    for (int i = 0;
            i < num;
            ++i)
        stream << "[" << rects[ i ].x << "+" << rects[ i ].y << " " << rects[ i ].width << "x" << rects[ i ].height << "]";
    return stream;
}

Picture SceneXrender::buffer = None;
ScreenPaintData SceneXrender::screen_paint;

SceneXrender::SceneXrender(Workspace* ws)
    : Scene(ws)
    , front(None)
    , m_overlayWindow(new OverlayWindow())
    , init_ok(false)
{
    if (!Extensions::renderAvailable()) {
        kError(1212) << "No XRender extension available";
        return;
    }
    if (!Extensions::fixesRegionAvailable()) {
        kError(1212) << "No XFixes v3+ extension available";
        return;
    }
    initXRender(true);
}

SceneXrender::~SceneXrender()
{
    if (!init_ok) {
        // TODO this probably needs to clean up whatever has been created until the failure
        m_overlayWindow->destroy();
        return;
    }
    XRenderFreePicture(display(), front);
    XRenderFreePicture(display(), buffer);
    buffer = None;
    m_overlayWindow->destroy();
    foreach (Window * w, windows)
    delete w;
    delete m_overlayWindow;
}

void SceneXrender::initXRender(bool createOverlay)
{
    init_ok = false;
    if (front != None)
        XRenderFreePicture(display(), front);
    KXErrorHandler xerr;
    bool haveOverlay = createOverlay ? m_overlayWindow->create() : (m_overlayWindow->window() != None);
    if (haveOverlay) {
        m_overlayWindow->setup(None);
        XWindowAttributes attrs;
        XGetWindowAttributes(display(), m_overlayWindow->window(), &attrs);
        format = XRenderFindVisualFormat(display(), attrs.visual);
        if (format == NULL) {
            kError(1212) << "Failed to find XRender format for overlay window";
            return;
        }
        front = XRenderCreatePicture(display(), m_overlayWindow->window(), format, 0, NULL);
    } else {
        // create XRender picture for the root window
        format = XRenderFindVisualFormat(display(), DefaultVisual(display(), DefaultScreen(display())));
        if (format == NULL) {
            kError(1212) << "Failed to find XRender format for root window";
            return; // error
        }
        XRenderPictureAttributes pa;
        pa.subwindow_mode = IncludeInferiors;
        front = XRenderCreatePicture(display(), rootWindow(), format, CPSubwindowMode, &pa);
    }
    createBuffer();
    if (xerr.error(true)) {
        kError(1212) << "XRender compositing setup failed";
        return;
    }
    init_ok = true;
}

bool SceneXrender::initFailed() const
{
    return !init_ok;
}

// Create the compositing buffer. The root window is not double-buffered,
// so it is done manually using this buffer,
void SceneXrender::createBuffer()
{
    if (buffer != None)
        XRenderFreePicture(display(), buffer);
    Pixmap pixmap = XCreatePixmap(display(), rootWindow(), displayWidth(), displayHeight(), DefaultDepth(display(), DefaultScreen(display())));
    buffer = XRenderCreatePicture(display(), pixmap, format, 0, 0);
    XFreePixmap(display(), pixmap);   // The picture owns the pixmap now
}

// the entry point for painting
int SceneXrender::paint(QRegion damage, ToplevelList toplevels)
{
    QElapsedTimer renderTimer;
    renderTimer.start();

    foreach (Toplevel * c, toplevels) {
        assert(windows.contains(c));
        stacking_order.append(windows[ c ]);
    }

    int mask = 0;
    paintScreen(&mask, &damage);

    if (m_overlayWindow->window())  // show the window only after the first pass, since
        m_overlayWindow->show();   // that pass may take long

    present(mask, damage);
    // do cleanup
    stacking_order.clear();

    return renderTimer.elapsed();
}

void SceneXrender::present(int mask, QRegion damage)
{
    if (mask & PAINT_SCREEN_REGION) {
        // Use the damage region as the clip region for the root window
        XserverRegion front_region = toXserverRegion(damage);
        XFixesSetPictureClipRegion(display(), front, 0, 0, front_region);
        XFixesDestroyRegion(display(), front_region);
        // copy composed buffer to the root window
        XFixesSetPictureClipRegion(display(), buffer, 0, 0, None);
        XRenderComposite(display(), PictOpSrc, buffer, None, front, 0, 0, 0, 0, 0, 0, displayWidth(), displayHeight());
        XFixesSetPictureClipRegion(display(), front, 0, 0, None);
        XSync(display(), false);
    } else {
        // copy composed buffer to the root window
        XRenderComposite(display(), PictOpSrc, buffer, None, front, 0, 0, 0, 0, 0, 0, displayWidth(), displayHeight());
        XSync(display(), false);
    }
}

void SceneXrender::paintGenericScreen(int mask, ScreenPaintData data)
{
    screen_paint = data; // save, transformations will be done when painting windows
    Scene::paintGenericScreen(mask, data);
}

// fill the screen background
void SceneXrender::paintBackground(QRegion region)
{
    PaintClipper pc(region);
    for (PaintClipper::Iterator iterator;
            !iterator.isDone();
            iterator.next()) {
        XRenderColor col = { 0, 0, 0, 0xffff }; // black
        XRenderFillRectangle(display(), PictOpSrc, buffer, &col, 0, 0, displayWidth(), displayHeight());
    }
}

void SceneXrender::windowGeometryShapeChanged(KWin::Toplevel* c)
{
    if (!windows.contains(c))    // this is ok, shape is not valid by default
        return;
    Window* w = windows[ c ];
    w->discardPicture();
    w->discardShape();
    w->discardAlpha();
}

void SceneXrender::windowOpacityChanged(KWin::Toplevel* c)
{
    if (!windows.contains(c))    // this is ok, alpha is created on demand
        return;
    Window* w = windows[ c ];
    w->discardAlpha();
}

void SceneXrender::windowClosed(KWin::Toplevel* c, KWin::Deleted* deleted)
{
    assert(windows.contains(c));
    if (deleted != NULL) {
        // replace c with deleted
        Window* w = windows.take(c);
        w->updateToplevel(deleted);
        if (w->shadow()) {
            w->shadow()->setToplevel(deleted);
        }
        windows[ deleted ] = w;
    } else {
        delete windows.take(c);
        c->effectWindow()->setSceneWindow(NULL);
    }
}

void SceneXrender::windowDeleted(Deleted* c)
{
    assert(windows.contains(c));
    delete windows.take(c);
    c->effectWindow()->setSceneWindow(NULL);
}

void SceneXrender::windowAdded(Toplevel* c)
{
    assert(!windows.contains(c));
    windows[ c ] = new Window(c);
    connect(c, SIGNAL(opacityChanged(KWin::Toplevel*,qreal)), SLOT(windowOpacityChanged(KWin::Toplevel*)));
    connect(c, SIGNAL(geometryShapeChanged(KWin::Toplevel*,QRect)), SLOT(windowGeometryShapeChanged(KWin::Toplevel*)));
    connect(c, SIGNAL(windowClosed(KWin::Toplevel*,KWin::Deleted*)), SLOT(windowClosed(KWin::Toplevel*,KWin::Deleted*)));
    c->effectWindow()->setSceneWindow(windows[ c ]);
    c->getShadow();
    windows[ c ]->updateShadow(c->shadow());
}

//****************************************
// SceneXrender::Window
//****************************************

QPixmap *SceneXrender::Window::temp_pixmap = 0;
QRect SceneXrender::Window::temp_visibleRect;

SceneXrender::Window::Window(Toplevel* c)
    : Scene::Window(c)
    , _picture(None)
    , format(XRenderFindVisualFormat(display(), c->visual()))
    , alpha(None)
    , alpha_cached_opacity(0.0)
{
}

SceneXrender::Window::~Window()
{
    discardPicture();
    discardAlpha();
    discardShape();
}

// Create XRender picture for the pixmap with the window contents.
Picture SceneXrender::Window::picture()
{
    if (!toplevel->damage().isEmpty() && _picture != None) {
        XRenderFreePicture(display(), _picture);
        _picture = None;
    }
    if (_picture == None && format != NULL) {
        // Get the pixmap with the window contents.
        Pixmap pix = toplevel->windowPixmap();
        if (pix == None)
            return None;
        _picture = XRenderCreatePicture(display(), pix, format, 0, 0);
        toplevel->resetDamage(toplevel->rect());
    }
    return _picture;
}


void SceneXrender::Window::discardPicture()
{
    if (_picture != None)
        XRenderFreePicture(display(), _picture);
    _picture = None;
}

void SceneXrender::Window::discardAlpha()
{
    if (alpha != None)
        XRenderFreePicture(display(), alpha);
    alpha = None;
}

// Create XRender picture for the alpha mask.
Picture SceneXrender::Window::alphaMask(double opacity)
{
    if (isOpaque() && qFuzzyCompare(opacity, 1.0))
        return None;

    bool created = false;
    if (alpha == None) {
        // Create a 1x1 8bpp pixmap containing the given opacity in the alpha channel.
        Pixmap pixmap = XCreatePixmap(display(), rootWindow(), 1, 1, 8);
        XRenderPictFormat* format = XRenderFindStandardFormat(display(), PictStandardA8);
        XRenderPictureAttributes pa;
        pa.repeat = True;
        alpha = XRenderCreatePicture(display(), pixmap, format, CPRepeat, &pa);
        XFreePixmap(display(), pixmap);
        created = true;
    }
    if (created || !qFuzzyCompare(alpha_cached_opacity + 1.0, opacity + 1.0)) {
        XRenderColor col;
        col.alpha = int(opacity * 0xffff);
        XRenderFillRectangle(display(), PictOpSrc, alpha, &col, 0, 0, 1, 1);
        alpha_cached_opacity = opacity;
    }
    return alpha;
}

// Maps window coordinates to screen coordinates
QRect SceneXrender::Window::mapToScreen(int mask, const WindowPaintData &data, const QRect &rect) const
{
    QRect r = rect;

    if (mask & PAINT_WINDOW_TRANSFORMED) {
        // Apply the window transformation
        r.moveTo(r.x() * data.xScale() + data.xTranslation(),
                 r.y() * data.yScale() + data.yTranslation());
        r.setWidth(r.width() * data.xScale());
        r.setHeight(r.height() * data.yScale());
    }

    // Move the rectangle to the screen position
    r.translate(x(), y());

    if (mask & PAINT_SCREEN_TRANSFORMED) {
        // Apply the screen transformation
        r.moveTo(r.x() * screen_paint.xScale() + screen_paint.xTranslation(),
                 r.y() * screen_paint.yScale() + screen_paint.yTranslation());
        r.setWidth(r.width() * screen_paint.xScale());
        r.setHeight(r.height() * screen_paint.yScale());
    }

    return r;
}

// Maps window coordinates to screen coordinates
QPoint SceneXrender::Window::mapToScreen(int mask, const WindowPaintData &data, const QPoint &point) const
{
    QPoint pt = point;

    if (mask & PAINT_WINDOW_TRANSFORMED) {
        // Apply the window transformation
        pt.rx() = pt.x() * data.xScale() + data.xTranslation();
        pt.ry() = pt.y() * data.yScale() + data.yTranslation();
    }

    // Move the point to the screen position
    pt += QPoint(x(), y());

    if (mask & PAINT_SCREEN_TRANSFORMED) {
        // Apply the screen transformation
        pt.rx() = pt.x() * screen_paint.xScale() + screen_paint.xTranslation();
        pt.ry() = pt.y() * screen_paint.yScale() + screen_paint.yTranslation();
    }

    return pt;
}

void SceneXrender::Window::prepareTempPixmap()
{
    temp_visibleRect = toplevel->visibleRect().translated(-toplevel->pos());

    if (temp_pixmap && Extensions::nonNativePixmaps())
        XFreePixmap(display(), temp_pixmap->handle());   // The picture owns the pixmap now
    if (!temp_pixmap)
        temp_pixmap = new QPixmap(temp_visibleRect.size());
    else if (temp_pixmap->width() < temp_visibleRect.width() || temp_pixmap->height() < temp_visibleRect.height()) {
        *temp_pixmap = QPixmap(temp_visibleRect.size());
        scene_setXRenderOffscreenTarget(0); // invalidate, better crash than cause weird results for developers
    }
    if (Extensions::nonNativePixmaps()) {
        Pixmap pix = XCreatePixmap(display(), rootWindow(), temp_pixmap->width(), temp_pixmap->height(), DefaultDepth(display(), DefaultScreen(display())));
        *temp_pixmap = QPixmap::fromX11Pixmap(pix);
    }
    temp_pixmap->fill(Qt::transparent);
}

// paint the window
void SceneXrender::Window::performPaint(int mask, QRegion region, WindowPaintData data)
{
    setTransformedShape(QRegion());  // maybe nothing will be painted
    // check if there is something to paint
    bool opaque = isOpaque() && qFuzzyCompare(data.opacity(), 1.0);
    /* HACK: It seems this causes painting glitches, disable temporarily
    if (( mask & PAINT_WINDOW_OPAQUE ) ^ ( mask & PAINT_WINDOW_TRANSLUCENT ))
        { // We are only painting either opaque OR translucent windows, not both
        if ( mask & PAINT_WINDOW_OPAQUE && !opaque )
            return; // Only painting opaque and window is translucent
        if ( mask & PAINT_WINDOW_TRANSLUCENT && opaque )
            return; // Only painting translucent and window is opaque
        }*/
    // Intersect the clip region with the rectangle the window occupies on the screen
    if (!(mask & (PAINT_WINDOW_TRANSFORMED | PAINT_SCREEN_TRANSFORMED)))
        region &= toplevel->visibleRect();

    if (region.isEmpty())
        return;
    Picture pic = picture(); // get XRender picture
    if (pic == None)   // The render format can be null for GL and/or Xv visuals
        return;
    // set picture filter
    if (options->isXrenderSmoothScale()) { // only when forced, it's slow
        if (mask & PAINT_WINDOW_TRANSFORMED)
            filter = ImageFilterGood;
        else if (mask & PAINT_SCREEN_TRANSFORMED)
            filter = ImageFilterGood;
        else
            filter = ImageFilterFast;
    } else
        filter = ImageFilterFast;
    // do required transformations
    const QRect wr = mapToScreen(mask, data, QRect(0, 0, width(), height()));
    QRect cr = QRect(toplevel->clientPos(), toplevel->clientSize()); // Client rect (in the window)
    qreal xscale = 1;
    qreal yscale = 1;
    bool scaled = false;

    Client *client = dynamic_cast<Client*>(toplevel);
    Deleted *deleted = dynamic_cast<Deleted*>(toplevel);
    const QRect decorationRect = toplevel->decorationRect();
    if ((client && !client->noBorder()) || (deleted && !deleted->noBorder()) &&
                                                        Workspace::self()->decorationHasAlpha()) {
        // decorated client
        transformed_shape = decorationRect;
        if (toplevel->shape()) {
            // "xeyes" + decoration
            transformed_shape -= cr;
            transformed_shape += shape();
        }
    } else {
        transformed_shape = shape();
    }
    if (toplevel->hasShadow())
        transformed_shape |= toplevel->shadow()->shadowRegion();

    XTransform xform = {{
            { XDoubleToFixed(1), XDoubleToFixed(0), XDoubleToFixed(0) },
            { XDoubleToFixed(0), XDoubleToFixed(1),  XDoubleToFixed(0) },
            { XDoubleToFixed(0), XDoubleToFixed(0), XDoubleToFixed(1) }
        }
    };

    static XTransform identity = {{
            { XDoubleToFixed(1), XDoubleToFixed(0), XDoubleToFixed(0) },
            { XDoubleToFixed(0), XDoubleToFixed(1),  XDoubleToFixed(0) },
            { XDoubleToFixed(0), XDoubleToFixed(0), XDoubleToFixed(1) }
        }
    };

    XRenderPictureAttributes attr;

    if (mask & PAINT_WINDOW_TRANSFORMED) {
        xscale = data.xScale();
        yscale = data.yScale();
    }
    if (mask & PAINT_SCREEN_TRANSFORMED) {
        xscale *= screen_paint.xScale();
        yscale *= screen_paint.yScale();
    }
    if (!qFuzzyCompare(xscale, 1.0) || !qFuzzyCompare(yscale, 1.0)) {
        scaled = true;
        xform.matrix[0][0] = XDoubleToFixed(1.0 / xscale);
        xform.matrix[1][1] = XDoubleToFixed(1.0 / yscale);

        // transform the shape for clipping in paintTransformedScreen()
        QVector<QRect> rects = transformed_shape.rects();
        for (int i = 0; i < rects.count(); ++i) {
            QRect& r = rects[ i ];
            r.setRect(qRound(r.x() * xscale), qRound(r.y() * yscale),
                      qRound(r.width() * xscale), qRound(r.height() * yscale));
        }
        transformed_shape.setRects(rects.constData(), rects.count());
    }

    transformed_shape.translate(mapToScreen(mask, data, QPoint(0, 0)));
    PaintClipper pcreg(region);   // clip by the region to paint
    PaintClipper pc(transformed_shape);   // clip by window's shape

    const bool wantShadow = m_shadow && !m_shadow->shadowRegion().isEmpty();

    // In order to obtain a pixel perfect rescaling
    // we need to blit the window content togheter with
    // decorations in a temporary pixmap and scale
    // the temporary pixmap at the end.
    // We should do this only if there is scaling and
    // the window has border
    // This solves a number of glitches and on top of this
    // it optimizes painting quite a bit
    const bool blitInTempPixmap = xRenderOffscreen() || (scaled && (wantShadow || (client && !client->noBorder()) || (deleted && !deleted->noBorder())));

    Picture renderTarget = buffer;
    if (blitInTempPixmap) {
        if (scene_xRenderOffscreenTarget()) {
            temp_visibleRect = toplevel->visibleRect().translated(-toplevel->pos());
            renderTarget = *scene_xRenderOffscreenTarget();
        } else {
            prepareTempPixmap();
            renderTarget = temp_pixmap->x11PictureHandle();
        }
    } else {
        XRenderSetPictureTransform(display(), pic, &xform);
        if (filter == ImageFilterGood) {
            XRenderSetPictureFilter(display(), pic, const_cast<char*>("good"), NULL, 0);
        }

        //BEGIN OF STUPID RADEON HACK
        // This is needed to avoid hitting a fallback in the radeon driver.
        // The Render specification states that sampling pixels outside the
        // source picture results in alpha=0 pixels. This can be achieved by
        // setting the border color to transparent black, but since the border
        // color has the same format as the texture, it only works when the
        // texture has an alpha channel. So the driver falls back to software
        // when the repeat mode is RepeatNone, the picture has a non-identity
        // transformation matrix, and doesn't have an alpha channel.
        // Since we only scale the picture, we can work around this by setting
        // the repeat mode to RepeatPad.
        if (!window()->hasAlpha()) {
            attr.repeat = RepeatPad;
            XRenderChangePicture(display(), pic, CPRepeat, &attr);
        }
        //END OF STUPID RADEON HACK
    }
#define MAP_RECT_TO_TARGET(_RECT_) \
        if (blitInTempPixmap) _RECT_.translate(-temp_visibleRect.topLeft()); else _RECT_ = mapToScreen(mask, data, _RECT_)

    //BEGIN deco preparations
    bool noBorder = true;
    const QPixmap *left = NULL;
    const QPixmap *top = NULL;
    const QPixmap *right = NULL;
    const QPixmap *bottom = NULL;
    PaintRedirector *redirector = NULL;
    QRect dtr, dlr, drr, dbr;
    if (client || deleted) {
        if (client && !client->noBorder()) {
            redirector = client->decorationPaintRedirector();
            noBorder = client->noBorder();
            client->layoutDecorationRects(dlr, dtr, drr, dbr, Client::WindowRelative);
        }
        if (deleted && !deleted->noBorder()) {
            noBorder = deleted->noBorder();
            redirector = deleted->decorationPaintRedirector();
            deleted->layoutDecorationRects(dlr, dtr, drr, dbr);
        }
        if (redirector) {
            redirector->ensurePixmapsPainted();
            left   = redirector->leftDecoPixmap();
            top    = redirector->topDecoPixmap();
            right  = redirector->rightDecoPixmap();
            bottom = redirector->bottomDecoPixmap();
        }
        if (!noBorder) {
            MAP_RECT_TO_TARGET(dtr);
            MAP_RECT_TO_TARGET(dlr);
            MAP_RECT_TO_TARGET(drr);
            MAP_RECT_TO_TARGET(dbr);
        }
    }
    //END deco preparations

    //BEGIN shadow preparations
    QRect stlr, str, strr, srr, sbrr, sbr, sblr, slr;
    SceneXRenderShadow* m_xrenderShadow = static_cast<SceneXRenderShadow*>(m_shadow);

    if (wantShadow) {
        m_xrenderShadow->layoutShadowRects(str, strr, srr, sbrr, sbr, sblr, slr, stlr);
        MAP_RECT_TO_TARGET(stlr);
        MAP_RECT_TO_TARGET(str);
        MAP_RECT_TO_TARGET(strr);
        MAP_RECT_TO_TARGET(srr);
        MAP_RECT_TO_TARGET(sbrr);
        MAP_RECT_TO_TARGET(sbr);
        MAP_RECT_TO_TARGET(sblr);
        MAP_RECT_TO_TARGET(slr);
    }
    //BEGIN end preparations

    //BEGIN client preparations
    QRect dr = cr;
    if (blitInTempPixmap) {
        dr.translate(-temp_visibleRect.topLeft());
    } else {
        dr = mapToScreen(mask, data, dr); // Destination rect
        if (scaled) {
            cr.moveLeft(cr.x() * xscale);
            cr.moveTop(cr.y() * yscale);
        }
    }

    const int clientRenderOp = (opaque || blitInTempPixmap) ? PictOpSrc : PictOpOver;
    //END client preparations

#undef MAP_RECT_TO_TARGET

    for (PaintClipper::Iterator iterator; !iterator.isDone(); iterator.next()) {

#define RENDER_SHADOW_TILE(_TILE_, _RECT_) \
XRenderComposite(display(), PictOpOver, m_xrenderShadow->shadowPixmap(SceneXRenderShadow::ShadowElement##_TILE_).x11PictureHandle(), \
                 shadowAlpha, renderTarget, 0, 0, 0, 0, _RECT_.x(), _RECT_.y(), _RECT_.width(), _RECT_.height())

        //shadow
        if (wantShadow) {
            Picture shadowAlpha = opaque ? None : alphaMask(data.opacity());
            RENDER_SHADOW_TILE(TopLeft, stlr);
            RENDER_SHADOW_TILE(Top, str);
            RENDER_SHADOW_TILE(TopRight, strr);
            RENDER_SHADOW_TILE(Left, slr);
            RENDER_SHADOW_TILE(Right, srr);
            RENDER_SHADOW_TILE(BottomLeft, sblr);
            RENDER_SHADOW_TILE(Bottom, sbr);
            RENDER_SHADOW_TILE(BottomRight, sbrr);
        }
#undef RENDER_SHADOW_TILE

        // Paint the window contents
        if (!(client && client->isShade())) {
            Picture clientAlpha = opaque ? None : alphaMask(data.opacity());
            XRenderComposite(display(), clientRenderOp, pic, clientAlpha, renderTarget, cr.x(), cr.y(), 0, 0, dr.x(), dr.y(), dr.width(), dr.height());
            if (!opaque)
                transformed_shape = QRegion();
        }

#define RENDER_DECO_PART(_PART_, _RECT_) \
XRenderComposite(display(), PictOpOver, _PART_->x11PictureHandle(), decorationAlpha, renderTarget,\
                 0, 0, 0, 0, _RECT_.x(), _RECT_.y(), _RECT_.width(), _RECT_.height())

        if (client || deleted) {
            if (!noBorder) {
                Picture decorationAlpha = alphaMask(data.opacity() * data.decorationOpacity());
                RENDER_DECO_PART(top, dtr);
                RENDER_DECO_PART(left, dlr);
                RENDER_DECO_PART(right, drr);
                RENDER_DECO_PART(bottom, dbr);
            }
            if (redirector) {
                redirector->markAsRepainted();
            }
        }
#undef RENDER_DECO_PART

        if (data.brightness() != 1.0) {
            // fake brightness change by overlaying black
            const float alpha = (1 - data.brightness()) * data.opacity();
            XRenderColor col = preMultiply(data.brightness() < 1.0 ? QColor(0,0,0,255*alpha) : QColor(255,255,255,-alpha*255));
            if (blitInTempPixmap) {
                XRenderFillRectangle(display(), PictOpOver, renderTarget, &col,
                                     -temp_visibleRect.left(), -temp_visibleRect.top(), width(), height());
            } else {
                XRenderFillRectangle(display(), PictOpOver, renderTarget, &col, wr.x(), wr.y(), wr.width(), wr.height());
            }
        }
        if (blitInTempPixmap) {
            const QRect r = mapToScreen(mask, data, temp_visibleRect);
            XRenderSetPictureTransform(display(), temp_pixmap->x11PictureHandle(), &xform);
            XRenderSetPictureFilter(display(), temp_pixmap->x11PictureHandle(), const_cast<char*>("good"), NULL, 0);
            XRenderComposite(display(), PictOpOver, temp_pixmap->x11PictureHandle(), None, buffer,
                             0, 0, 0, 0, r.x(), r.y(), r.width(), r.height());
            XRenderSetPictureTransform(display(), temp_pixmap->x11PictureHandle(), &identity);
        }
    }
    if (scaled && !blitInTempPixmap) {
        XRenderSetPictureTransform(display(), pic, &identity);
        if (filter == ImageFilterGood)
            XRenderSetPictureFilter(display(), pic, const_cast<char*>("fast"), NULL, 0);
        if (!window()->hasAlpha()) {
            attr.repeat = RepeatNone;
            XRenderChangePicture(display(), pic, CPRepeat, &attr);
        }
    }
    if (xRenderOffscreen())
        scene_setXRenderOffscreenTarget(temp_pixmap);
}

void SceneXrender::screenGeometryChanged(const QSize &size)
{
    Scene::screenGeometryChanged(size);
    initXRender(false);
}

//****************************************
// SceneXrender::EffectFrame
//****************************************

SceneXrender::EffectFrame::EffectFrame(EffectFrameImpl* frame)
    : Scene::EffectFrame(frame)
{
    m_picture = NULL;
    m_textPicture = NULL;
    m_iconPicture = NULL;
    m_selectionPicture = NULL;
}

SceneXrender::EffectFrame::~EffectFrame()
{
    delete m_picture;
    delete m_textPicture;
    delete m_iconPicture;
    delete m_selectionPicture;
}

void SceneXrender::EffectFrame::free()
{
    delete m_picture;
    m_picture = NULL;
    delete m_textPicture;
    m_textPicture = NULL;
    delete m_iconPicture;
    m_iconPicture = NULL;
    delete m_selectionPicture;
    m_selectionPicture = NULL;
}

void SceneXrender::EffectFrame::freeIconFrame()
{
    delete m_iconPicture;
    m_iconPicture = NULL;
}

void SceneXrender::EffectFrame::freeTextFrame()
{
    delete m_textPicture;
    m_textPicture = NULL;
}

void SceneXrender::EffectFrame::freeSelection()
{
    delete m_selectionPicture;
    m_selectionPicture = NULL;
}

void SceneXrender::EffectFrame::crossFadeIcon()
{
    // TODO: implement me
}

void SceneXrender::EffectFrame::crossFadeText()
{
    // TODO: implement me
}

void SceneXrender::EffectFrame::render(QRegion region, double opacity, double frameOpacity)
{
    Q_UNUSED(region)
    if (m_effectFrame->geometry().isEmpty()) {
        return; // Nothing to display
    }

    // Render the actual frame
    if (m_effectFrame->style() == EffectFrameUnstyled)
        xRenderRoundBox(effects->xrenderBufferPicture(), m_effectFrame->geometry().adjusted(-5, -5, 5, 5),
                        5, QColor(0, 0, 0, int(opacity * frameOpacity * 255)));
    else if (m_effectFrame->style() == EffectFrameStyled) {
        if (!m_picture) { // Lazy creation
            updatePicture();
        }
        if (m_picture) {
            qreal left, top, right, bottom;
            m_effectFrame->frame().getMargins(left, top, right, bottom);   // m_geometry is the inner geometry
            QRect geom = m_effectFrame->geometry().adjusted(-left, -top, right, bottom);
            XRenderComposite(display(), PictOpOver, *m_picture, None, effects->xrenderBufferPicture(),
                                0, 0, 0, 0, geom.x(), geom.y(), geom.width(), geom.height());
        }
    }
    if (!m_effectFrame->selection().isNull()) {
        if (!m_selectionPicture) { // Lazy creation
            const QPixmap pix = m_effectFrame->selectionFrame().framePixmap();
            if (!pix.isNull()) // don't try if there's no content
                m_selectionPicture = new XRenderPicture(m_effectFrame->selectionFrame().framePixmap());
        }
        if (m_selectionPicture) {
            const QRect geom = m_effectFrame->selection();
            XRenderComposite(display(), PictOpOver, *m_selectionPicture, None, effects->xrenderBufferPicture(),
                                0, 0, 0, 0, geom.x(), geom.y(), geom.width(), geom.height());
        }
    }

    XRenderPicture fill = xRenderBlendPicture(opacity);

    // Render icon
    if (!m_effectFrame->icon().isNull() && !m_effectFrame->iconSize().isEmpty()) {
        QPoint topLeft(m_effectFrame->geometry().x(), m_effectFrame->geometry().center().y() - m_effectFrame->iconSize().height() / 2);

        if (!m_iconPicture)   // lazy creation
            m_iconPicture = new XRenderPicture(m_effectFrame->icon());
        QRect geom = QRect(topLeft, m_effectFrame->iconSize());
        XRenderComposite(display(), PictOpOver, *m_iconPicture, fill, effects->xrenderBufferPicture(),
                         0, 0, 0, 0, geom.x(), geom.y(), geom.width(), geom.height());
    }

    // Render text
    if (!m_effectFrame->text().isEmpty()) {
        if (!m_textPicture) { // Lazy creation
            updateTextPicture();
        }
        XRenderComposite(display(), PictOpOver, *m_textPicture, fill, effects->xrenderBufferPicture(),
                         0, 0, 0, 0, m_effectFrame->geometry().x(), m_effectFrame->geometry().y(),
                         m_effectFrame->geometry().width(), m_effectFrame->geometry().height());
    }
}

void SceneXrender::EffectFrame::updatePicture()
{
    delete m_picture;
    m_picture = 0L;
    if (m_effectFrame->style() == EffectFrameStyled) {
        const QPixmap pix = m_effectFrame->frame().framePixmap();
        if (!pix.isNull())
            m_picture = new XRenderPicture(pix);
    }
}

void SceneXrender::EffectFrame::updateTextPicture()
{
    // Mostly copied from SceneOpenGL::EffectFrame::updateTextTexture() above
    delete m_textPicture;
    m_textPicture = 0L;

    if (m_effectFrame->text().isEmpty()) {
        return;
    }

    // Determine position on texture to paint text
    QRect rect(QPoint(0, 0), m_effectFrame->geometry().size());
    if (!m_effectFrame->icon().isNull() && !m_effectFrame->iconSize().isEmpty()) {
        rect.setLeft(m_effectFrame->iconSize().width());
    }

    // If static size elide text as required
    QString text = m_effectFrame->text();
    if (m_effectFrame->isStatic()) {
        QFontMetrics metrics(m_effectFrame->text());
        text = metrics.elidedText(text, Qt::ElideRight, rect.width());
    }

    QPixmap pixmap(m_effectFrame->geometry().size());
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setFont(m_effectFrame->font());
    if (m_effectFrame->style() == EffectFrameStyled) {
        p.setPen(m_effectFrame->styledTextColor());
    } else {
        // TODO: What about no frame? Custom color setting required
        p.setPen(Qt::white);
    }
    p.drawText(rect, m_effectFrame->alignment(), text);
    p.end();
    m_textPicture = new XRenderPicture(pixmap);
}

SceneXRenderShadow::SceneXRenderShadow(Toplevel *toplevel)
    :Shadow(toplevel)
{
}

SceneXRenderShadow::~SceneXRenderShadow()
{
}

void SceneXRenderShadow::layoutShadowRects(QRect& top, QRect& topRight,
                                           QRect& right, QRect& bottomRight,
                                           QRect& bottom, QRect& bottomLeft,
                                           QRect& left, QRect& topLeft)
{
    WindowQuadList quads = shadowQuads();

    if (quads.count() == 0) {
        return;
    }

    WindowQuad topQuad = quads.select(WindowQuadShadowTop)[0];
    WindowQuad topRightQuad = quads.select(WindowQuadShadowTopRight)[0];
    WindowQuad topLeftQuad = quads.select(WindowQuadShadowTopLeft)[0];
    WindowQuad leftQuad = quads.select(WindowQuadShadowLeft)[0];
    WindowQuad rightQuad = quads.select(WindowQuadShadowRight)[0];
    WindowQuad bottomQuad = quads.select(WindowQuadShadowBottom)[0];
    WindowQuad bottomRightQuad = quads.select(WindowQuadShadowBottomRight)[0];
    WindowQuad bottomLeftQuad = quads.select(WindowQuadShadowBottomLeft)[0];

    top = QRect(topQuad.left(), topQuad.top(), (topQuad.right()-topQuad.left()), (topQuad.bottom()-topQuad.top()));
    topLeft = QRect(topLeftQuad.left(), topLeftQuad.top(), (topLeftQuad.right()-topLeftQuad.left()), (topLeftQuad.bottom()-topLeftQuad.top()));
    topRight = QRect(topRightQuad.left(), topRightQuad.top(), (topRightQuad.right()-topRightQuad.left()), (topRightQuad.bottom()-topRightQuad.top()));
    left = QRect(leftQuad.left(), leftQuad.top(), (leftQuad.right()-leftQuad.left()), (leftQuad.bottom()-leftQuad.top()));
    right = QRect(rightQuad.left(), rightQuad.top(), (rightQuad.right()-rightQuad.left()), (rightQuad.bottom()-rightQuad.top()));
    bottom = QRect(bottomQuad.left(), bottomQuad.top(), (bottomQuad.right()-bottomQuad.left()), (bottomQuad.bottom()-bottomQuad.top()));
    bottomLeft = QRect(bottomLeftQuad.left(), bottomLeftQuad.top(), (bottomLeftQuad.right()-bottomLeftQuad.left()), (bottomLeftQuad.bottom()-bottomLeftQuad.top()));
    bottomRight = QRect(bottomRightQuad.left(), bottomRightQuad.top(), (bottomRightQuad.right()-bottomRightQuad.left()), (bottomRightQuad.bottom()-bottomRightQuad.top()));
}

void SceneXRenderShadow::buildQuads()
{
    Shadow::buildQuads();

    if (shadowQuads().count() == 0) {
        return;
    }

    QRect stlr, str, strr, srr, sbrr, sbr, sblr, slr;
    layoutShadowRects(str, strr, srr, sbrr, sbr, sblr, slr, stlr);

    XRenderPictureAttributes attr;
    attr.repeat = True;
    for (int i = 0; i < ShadowElementsCount; ++i) {
        XRenderChangePicture(display(), shadowPixmap((Shadow::ShadowElements)i).x11PictureHandle(), CPRepeat, &attr);
    }

}

} // namespace
#endif
