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

namespace Wayland
{
// forward declaration
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

    wl_buffer *buffer();

public Q_SLOTS:
    void setGeometry(const QRect &geometry);
    void surfaceDamaged(const QRect &damage);

protected:
    virtual bool shouldUnredirect() const;
    virtual void debug(QDebug& stream) const;

private:
    Surface *m_surface;
};

} // namespace Wayland

} // namespace KWin

#endif // KWIN_WAYLAND_CLIENT_H
