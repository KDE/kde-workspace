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

#include "wayland_client.h"
#include "wayland.h"
#include "surface.h"
// wayland
#include <wayland-server.h>

namespace KWin
{
namespace Wayland
{
Client::Client(Workspace* ws, Surface* surface)
    : Toplevel(ws)
    , m_surface(surface)
{
    geom = surface->geometry();
    // all Wayland Clients have alpha
    bit_depth = 32;
    setupCompositing();
    connect(m_surface, SIGNAL(geometryChanged(QRect)), SLOT(setGeometry(QRect)));
    connect(m_surface, SIGNAL(damaged(QRect)), SLOT(surfaceDamaged(QRect)));
}

Client::~Client()
{
    workspace()->removeWaylandClient(this);
}

bool Client::shouldUnredirect() const
{
    return false;
}

void Client::debug(QDebug& stream) const
{
    Q_UNUSED(stream)
    // TODO implement me
}

QStringList Client::activities() const
{
    return QStringList();
}

int Client::desktop() const
{
    return NET::OnAllDesktops;
}

QRect Client::transparentRect() const
{
    return QRect();
}

QSize Client::clientSize() const
{
    return m_surface->geometry().size();
}

QPoint Client::clientPos() const
{
    return m_surface->geometry().topLeft();
}

double Client::opacity() const
{
    return 1.0;
}

bool Client::isWayland() const
{
    return true;
}

wl_buffer *Client::buffer()
{
    return m_surface->buffer();
}

void Client::setGeometry(const QRect &geometry)
{
    if (geom == geometry) {
        return;
    }
    addRepaint(geom);
    geom = geometry;
    addRepaint(geom);
}
void Client::frameRendered(int timeStamp)
{
    wl_display_post_frame(workspace()->wayland()->display(), m_surface->surface(), timeStamp);
}

void Client::surfaceDamaged(const QRect &damage)
{
    addDamage(damage);
}

} // namespace Wayland
} // namespace KWin
