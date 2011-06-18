/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

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

#ifndef KWIN_WAYLAND_CLIENT_H
#define KWIN_WAYLAND_CLIENT_H

#include <toplevel.h>

// forward declaration
struct wl_buffer;

namespace KWin
{
class PaintRedirector;

namespace Wayland
{
// forward declaration
class Bridge;
class Surface;

/**
 * @short KWin representation of a Wayland Client.
 *
 * The Wayland::Client implements @link KWin::Toplevel and by that can be used by
 * @link Workspace for management and compositing.
 *
 * The Client is linked to @link Surface and is created and destroyed by it.
 * @author Martin Gräßlin <mgraesslin@kde.org>
 **/
class Client : public KWin::Toplevel
{
    Q_OBJECT

public:
    Client(Workspace *ws, Surface *surface);
    virtual ~Client();
    virtual QStringList activities() const;
    virtual int desktop() const;
    virtual QRect transparentRect() const;
    virtual QSize clientSize() const;
    virtual QPoint clientPos() const;
    virtual double opacity() const;
    virtual bool isWayland() const;
    virtual void frameRendered(int timeStamp);
    virtual QRegion decorationPendingRegion() const;
    virtual QRect decorationRect() const;

    wl_buffer *buffer();

    // TODO: methods to inherit together with KWin::Client
    enum CoordinateMode {
        DecorationRelative, // Relative to the top left corner of the decoration
        WindowRelative      // Relative to the top left corner of the window
    };
    virtual void updateDecoration(bool checkWorkspacePos, bool force);
    virtual void triggerDecorationRepaint();
    virtual void layoutDecorationRects(QRect &left, QRect &top, QRect &right, QRect &bottom, Client::CoordinateMode mode) const;
    virtual void move(int x, int y, ForceGeometry_t force = NormalGeometrySet);
    virtual void move(const QPoint& p, ForceGeometry_t force = NormalGeometrySet);
    // Decorations <-> Effects
    virtual const QPixmap *topDecoPixmap() const {
        return &m_decorationPixmapTop;
    }
    virtual const QPixmap *leftDecoPixmap() const {
        return &m_decorationPixmapLeft;
    }
    virtual const QPixmap *bottomDecoPixmap() const {
        return &m_decorationPixmapBottom;
    }
    virtual const QPixmap *rightDecoPixmap() const {
        return &m_decorationPixmapRight;
    }

    virtual bool decorationPixmapRequiresRepaint() const;
    virtual void ensureDecorationPixmapsPainted();

    virtual void setDesktop(int newDesktop);

    virtual void closeWindow();

public Q_SLOTS:
    void setGeometry(const QRect &geometry);
    void surfaceDamaged(const QRect &damage);
    /**
     * Shows/Hide decoration when desktop changes.
     **/
    void updateDecorationVisibility();
    // TODO: slots to inherit together with KWin::Client
    void repaintDecorationPending();

protected:
    virtual bool shouldUnredirect() const;
    virtual void debug(QDebug& stream) const;

    // TODO: methods to inherit together with KWin::Client
    virtual void resizeDecorationPixmaps();
    virtual void resizeDecoration(const QSize &s);
    virtual void destroyDecoration();
    virtual void repaintDecorationPixmap(QPixmap& pix, const QRect& r, const QPixmap& src, QRegion reg);

private:
    Surface *m_surface;
    Bridge *m_decorationBridge;

    // TODO: member variables to inherit together with KWin::Client
    KDecoration *m_decoration;
    PaintRedirector* m_paintRedirector;
    int m_borderLeft, m_borderRight, m_borderTop, m_borderBottom;
    int m_paddingLeft, m_paddingRight, m_paddingTop, m_paddingBottom;
    QPixmap m_decorationPixmapLeft, m_decorationPixmapRight, m_decorationPixmapTop, m_decorationPixmapBottom;
    // size of the client inside the decoration
    QSize m_clientSize;

    int m_desktop;
};

} // namespace Wayland

} // namespace KWin

#endif // KWIN_WAYLAND_CLIENT_H
