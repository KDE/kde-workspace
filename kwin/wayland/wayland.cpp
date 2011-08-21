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

#include "wayland.h"
#include "surface.h"
#include "shell.h"

// kwin
#include <kwinglobals.h>
// KDE
#include <KDE/KDebug>
#include "workspace.h"
// Qt
#include <QtCore/QLatin1String>
#include <QtCore/QSocketNotifier>

namespace KWin
{

namespace Wayland
{

/********************************************************************
 * Callback for the wl_compositor_interface
 * Needed for creating a surface.
 *******************************************************************/

void compositorCreateSurface(struct wl_client *client,
                               struct wl_compositor *compositor, uint32_t id)
{
    Q_UNUSED(compositor)
    Workspace::self()->wayland()->addClient(client, id);
    kDebug(1212) << "Compositor Create Surface called with id " << id;
}

const static struct wl_compositor_interface compositorInterface = {
    compositorCreateSurface
};

/********************************************************************
 * Callbacks for the wl_shm_callbacks interface
 * Why we need it, I don't know yet.
 *******************************************************************/

void shmBufferCreated(struct wl_buffer *buffer)
{
    Q_UNUSED(buffer)
    kDebug(1212) << "SHM Buffer Created not yet implemented";
}

void shmBufferDamaged(struct wl_buffer *buffer, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(buffer)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
    kDebug(1212) << "SHM Buffer Damaged not yet implemented";
}

void shmBufferDestroyed(struct wl_buffer *buffer)
{
    Q_UNUSED(buffer)
    kDebug(1212) << "SHM Buffer Destroyed not yet implemented";
}

const static struct wl_shm_callbacks shm_callbacks = {
        shmBufferCreated,
        shmBufferDamaged,
        shmBufferDestroyed
};

/********************************************************************
 * Callbacks for the wl_input_device_callbacks interface
 *******************************************************************/
void inputDeviceAttach(struct wl_client *client, struct wl_input_device *device_base, uint32_t time, struct wl_buffer *buffer, int32_t x, int32_t y) {
    Q_UNUSED(client)
    Q_UNUSED(device_base)
    Q_UNUSED(time)
    Q_UNUSED(buffer)
    Q_UNUSED(x)
    Q_UNUSED(y)
    kDebug(1212) << "Input Device Attach not yet implemented";
}

const static struct wl_input_device_interface inputDeviceCallbacks = {
    inputDeviceAttach
};

/********************************************************************
 * Callback for the Wayland output
 *******************************************************************/

void outputPostGeometry(struct wl_client *client, struct wl_object *global, uint32_t version)
{
    Q_UNUSED(version)
    wl_client_post_event(client, global,
            WL_OUTPUT_GEOMETRY,
            0,
            0,
            displayWidth(),
            displayHeight(),
            WL_OUTPUT_SUBPIXEL_UNKNOWN,
            NULL,   // make
            NULL);  // model
    kDebug(1212) << "Output Post Geometry";
}

/********************************************************************
 * KWin::Wayland::Server
********************************************************************/

Server::Server()
    : QObject(NULL)
    , m_initOk(false)
    , m_display(NULL)
    , m_shm(NULL)
{
    if (!init()) {
        return;
    }
    m_initOk = true;
}

Server::~Server()
{
    if (m_display) {
        tearDown();
    }
}

bool Server::init()
{
    m_display = wl_display_create();
    if (!m_display) {
        kDebug(1212) << "Could not create Wayland Display";
        return false;
    }

    memset(&m_compositor, 0, sizeof(m_compositor));
    if (wl_compositor_init(&m_compositor, &compositorInterface, m_display)) {
        kDebug(1212) << "Failed to init Wayland Compositor";
        return false;
    }
    m_shm = wl_shm_init(m_display, &shm_callbacks);

    // create the output
    memset(&m_output, 0, sizeof(m_output));
    m_output.interface = &wl_output_interface;
    wl_display_add_object(m_display, &m_output);
    if (wl_display_add_global(m_display, &m_output, outputPostGeometry)) {
        kDebug(1212) << "Failed to add global Output Wayland object";
        return false;
    }

    m_shell = new Shell();
    wl_display_add_object(m_display, m_shell->wlObjectHandle());
    if (wl_display_add_global(m_display, m_shell->wlObjectHandle(), NULL)) {
        kDebug(1212) << "Failed to add global Shell object";
        return false;
    }

    // TODO: read socket name from command line
    if (wl_display_add_socket(m_display, QLatin1String("wayland-0").latin1())) {
        kDebug(1212) << "Failed to create Wayland Socket";
        return false;
    }

    memset(&m_input, 0, sizeof(m_input));
    wl_input_device_init(&m_input, &m_compositor);
    m_input.object.interface = &wl_input_device_interface;
    m_input.object.implementation = (void (**)())(&inputDeviceCallbacks);
    wl_display_add_object(m_display, &m_input.object);
    if (wl_display_add_global(m_display, &m_input.object, NULL)) {
        kDebug(1212) << "Failed to add global Input Device Wayland object";
        return false;
    }

    wl_event_loop *loop = wl_display_get_event_loop(m_display);
    int fd = wl_event_loop_get_fd(loop);

    QSocketNotifier *socketNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(socketNotifier, SIGNAL(activated(int)), this, SLOT(processWaylandEvents()));
    return true;
}

void Server::tearDown()
{
    wl_display_destroy(m_display);
}

void Server::addClient(wl_client* client, uint32_t id)
{
    Surface *surface = new Surface(this, client, id);
    m_surfaces << surface;
}

void Server::removeClient(uint32_t id)
{
    Surface *surface = findSurface(id);
    if (surface) {
        m_surfaces.removeOne(surface);
        delete surface;
        kDebug(1212) << "Deleted Surface for id " << id;
    }
}
Surface* Server::client(uint32_t id)
{
    return findSurface(id);
}

Surface* Server::findSurface(uint32_t id)
{
    foreach(Surface *surface, m_surfaces) {
        if (id == surface->clientId()) {
            return surface;
        }
    }
    return NULL;
}

void Server::processWaylandEvents()
{
    int ret = wl_event_loop_dispatch(wl_display_get_event_loop(m_display), 0);
    if (ret) {
        kDebug(1212) << "wl_event_loop_dispatch error: " << ret;
    }
}

} // namespace Wayland
} // namespace KWin

