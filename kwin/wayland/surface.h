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

#ifndef KWIN_WAYLAND_SURFACE_H
#define KWIN_WAYLAND_SURFACE_H
#include <wayland-server.h>
#include <QtCore/QObject>
#include <QtCore/QRect>

namespace KWin
{

namespace Wayland
{

class Client;
class Server;

/**
 * @short Wayland Surface implementation.
 *
 * This class represents a Wayland Surface. The Surface is created by the @link Server and managed
 * by that one. The current implementation assumes that one Wayland client may have exactly one
 * Surface, which might be wrong.
 *
 * The class holds a Wayland resource which is added to the Wayland client. Wayland uses the
 * resource to add a buffer to the surface through the surface attach callback. Also the resource is
 * used to notify the server about damaged areas. It seems like the callbacks for damage and surface
 * attach are always invoked when the Wayland client updates the surface.
 *
 * The Surface is linked to a @link Wayland::Client. The lifetime of both is the same.
 * The @link Wayland::Client represents a Client to @link KWin::Workspace, while the Surface holds
 * the Wayland relevant parts of the Wayland "window".
 *
 * @author Martin Gräßlin <mgraesslin@kde.org>
 **/
class Surface : public QObject
{
    Q_OBJECT
public:
    Surface(Server *parent, wl_client* client, uint32_t id);
    virtual ~Surface();

    uint32_t clientId() const {
        return m_resource.object.id;
    }

    wl_resource *handle() {
        return &m_resource;
    }

    void setGeometry(const QRect &geometry);
    const QRect &geometry() const {
        return m_geometry;
    }

    void setBuffer(struct wl_buffer *buffer);
    wl_buffer *buffer() {
        return m_buffer;
    }

    void setSurface(struct wl_surface *surface) {
        m_surface = surface;
    }
    wl_surface *surface() {
        return m_surface;
    }
    wl_client *nativeClient() {
        return m_nativeClient;
    }

    void damage(const QRect &rect);

Q_SIGNALS:
    void geometryChanged(const QRect&);
    void damaged(const QRect&);

private:
    Server *m_server;
    wl_surface *m_surface;
    wl_resource m_resource;
    QRect m_geometry;
    wl_buffer *m_buffer;
    Client *m_client;
    wl_client *m_nativeClient;
};

} // namespace Wayland

} // namespace KWin

#endif // KWIN_WAYLAND_SURFACE_H
