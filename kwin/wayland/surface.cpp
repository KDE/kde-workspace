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

#include "surface.h"
#include "wayland.h"

#include <KDE/KDebug>
#include <workspace.h>
#include "wayland_client.h"
// other
#include <sys/time.h>

namespace KWin
{

namespace Wayland
{
/********************************************************************
 * Callback for the wl_surface_interface
 * Used to attach buffers to the surface and map the window (whatever it's used for)
 *******************************************************************/

void destroySurface(struct wl_resource *resource, struct wl_client *client)
{
    Q_UNUSED(client)
    kDebug(1212) << "Destroy surface called";
    Workspace::self()->wayland()->removeClient(resource->object.id);
}

void surfaceDestroy(struct wl_client *client, struct wl_surface *surface)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    wl_resource_destroy(&surface->resource, client, (tv.tv_sec * 1000 + tv.tv_usec / 1000));
}

void surfaceAttach(struct wl_client *client, struct wl_surface *surface, struct wl_buffer *buffer, int32_t x, int32_t y)
{
    Q_UNUSED(client)
    kDebug(1212) << "Surface Attach called " << x << "/" << y << "(" << buffer->width << "/" << buffer->height << ")";
    Surface *s = Workspace::self()->wayland()->client(surface->resource.object.id);
    s->setGeometry(QRect(x, y, buffer->width, buffer->height));
    s->setBuffer(buffer);
    s->setSurface(surface);
}

void surfaceMapToplevel(struct wl_client *client, struct wl_surface *surface)
{
    Q_UNUSED(client)
    Q_UNUSED(surface)
    kDebug(1212) << "Surface Map Toplevel not yet implemented";
}

void surfaceMapTransient(struct wl_client *client, struct wl_surface *surface, struct wl_surface *parent, int x, int y, uint32_t flags)
{
    Q_UNUSED(client)
    Q_UNUSED(surface)
    Q_UNUSED(parent)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(flags)
    kDebug(1212) << "Surface Map transient not yet implemented";
}

void surfaceMapFullscreen(struct wl_client *client, struct wl_surface *surface)
{
    Q_UNUSED(client)
    Q_UNUSED(surface)
    kDebug(1212) << "Surface Map Fullscreen not yet implemented";
}

void surfaceDamage(struct wl_client *client, struct wl_surface *surface, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    kDebug(1212) << "Surface Damage called" << x << "/" << y << "(" << width << "/" << height << ")";
    Workspace::self()->wayland()->client(surface->resource.object.id)->damage(QRect(x, y, width, height));
}

const static struct wl_surface_interface surfaceInterface = {
        surfaceDestroy,
        surfaceAttach,
        surfaceMapToplevel,
        surfaceMapTransient,
        surfaceMapFullscreen,
        surfaceDamage
};

Surface::Surface(Server *parent, struct wl_client *client, uint32_t id)
    : QObject(parent)
    , m_server(parent)
    , m_surface(NULL)
    , m_buffer(NULL)
    , m_client(NULL)
    , m_nativeClient(client)
{
    m_resource.object.id = id;
    m_resource.object.interface = &wl_surface_interface;
    m_resource.object.implementation = (void (**)())(&surfaceInterface);
    m_resource.destroy = &destroySurface;
    wl_client_add_resource(client, &m_resource);
    m_client = Workspace::self()->createWaylandClient(this);
    m_client->setParent(this);
}

Surface::~Surface()
{
}

void Surface::setGeometry(const QRect& geometry)
{
    if (m_geometry == geometry) {
        return;
    }
    m_geometry = geometry;
    emit geometryChanged(geometry);
}

void Surface::setBuffer(wl_buffer* buffer)
{
    if (buffer == m_buffer) {
        return;
    }
    m_buffer = buffer;
}

void Surface::damage(const QRect& rect)
{
    emit damaged(rect);
}

} // namespace Wayland
} // namespace KWin
