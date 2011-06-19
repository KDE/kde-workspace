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

#ifndef KWIN_WAYLAND_SERVER_H
#define KWIN_WAYLAND_SERVER_H
#include <wayland-server.h>
#include <QtCore/QObject>
#include <QtCore/QList>

namespace KWin
{

/**
 * @namespace Wayland
 *
 * The Wayland namespace contains all the classes relevant to KWin's Wayland Server implementation.
 * These classes only implement the Server and manage Wayland surfaces. Unlike other implementations
 * of Wayland servers this namespace does not implement a Wayland compositor. The compositor is
 * shared with KWin's X11 compositor @link KWin::Scene.
 *
 * The namespace contains a class @link Server which implements the basic Server functionality such
 * as creating the Wayland socket and display and listening for events on the socket. The server is
 * part of @link KWin::Workspace and created when an EGL compositing scene is created. As Wayland
 * requires compositing the server is automatically stopped when the compositing is suspended.
 *
 * The server also manages the Wayland Surface for which a class @link Surface is used. The Surface
 * is linked to a @link Wayland::Client which inherits @link KWin::Toplevel. So to say it is a KWin
 * compatible representation of a Wayland window which can be included in the normal KWin structures
 * and can be composited in the Scene.
 *
 * The basic idea behind Wayland is that there is a @link Server which opens a socket. Wayland clients
 * connect to the socket and create a @link Surface. The clients create buffers and exchange the
 * buffer information with the Server. The server can create a texture from the buffer data and
 * composite the window. Wayland allows to run your own protocol over the socket, this could be
 * used to exchange window management information just like what was EWMH for X11.
 *
 * Further information can be found on: http://wayland.freedesktop.org
 *
 * The server implementation is inspired by the source code of the
 * Wayland demo Compositor (http://cgit.freedesktop.org/wayland/wayland-demos/)
 * and the Qt-Compositor (http://qt.gitorious.org/qt-labs/qt-compositor).
 *
 * Unfortunately the documentation is currently (state June 2011) almost non existing and the two
 * demo compositors are different to this Wayland implementation as they are focused on being a
 * Wayland compositor, while this implementation focus is on being a Wayland server using the
 * existing X11 compositor.
 **/
namespace Wayland
{
// forward declarations
class Surface;

/**
 * @short Wayland Server Implementation.
 *
 * The Server takes care of creating the Wayland socket and display. The Server is created
 * by Workspace when the compositing scene is started.
 *
 * The Server manages @link Surface.
 *
 * @author Martin Gräßlin <mgraesslin@kde.org>
 **/
class Server : public QObject
{
    Q_OBJECT

public:
    Server();
    virtual ~Server();
    bool initFailed() const {
        return !m_initOk;
    }

    /**
     * Returns the Wayland display handle which is required by the compositor to bind
     * an EGL display to the Wayland display.
     *
     * @returns The Wayland display handle.
     **/
    wl_display *display() const {
        return m_display;
    }
    wl_input_device *input() {
        return &m_input;
    }

    /**
     * Creates a @link Surface for the Wayland @p client with @p id.
     * The Surface implicitly creates a @link Client and adds it to
     * the Workspace for management.
     *
     * @param client The Wayland Client structure
     * @param id The Client's id
     **/
    void addClient(struct wl_client *client, uint32_t id);
    /**
     * Removes the Surface for client with @p id.
     *
     * This will also unmanage the corresponding @link Client in Workspace.
     * @param id The id of the client to remove.
     **/
    void removeClient(uint32_t id);
    /**
     * Returns the Surface of the client with @p id.
     * @returns Surface for the client
     **/
    Surface *client(uint32_t id);

private Q_SLOTS:
    /**
     * Slot which is invoked whenever something arrives at the socket.
     **/
    void processWaylandEvents();

private:
    bool init();
    void tearDown();
    Surface *findSurface(uint32_t id);
    bool m_initOk;
    wl_display *m_display;
    wl_shm *m_shm;
    struct wl_compositor m_compositor;

    struct wl_object m_output;
    // TODO: in future we might need more than one input device
    struct wl_input_device m_input;

    QList<KWin::Wayland::Surface*> m_surfaces;
};

} // namespace Wayland

} // namespace KWin

#endif // KWIN_WAYLAND_SERVER_H
