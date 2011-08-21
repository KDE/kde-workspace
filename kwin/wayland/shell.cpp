/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2011 Philipp Brüschweiler <blei42@gmail.com>

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

#include "shell.h"
#include "wayland.h"

#include <KDE/KDebug>

namespace KWin
{

namespace Wayland
{

/********************************************************************
 * Callbacks for the wl_shell interface
 *******************************************************************/

static void shellMove(struct wl_client *client, struct wl_shell *shell,
        struct wl_surface *surface, struct wl_input_device *input_device,
        uint32_t time)
{
    Q_UNUSED(client);
    Q_UNUSED(shell);
    Q_UNUSED(surface);
    Q_UNUSED(input_device);
    Q_UNUSED(time);
    kDebug(1212) << "Shell Move not yet implemented";
}

static void shellResize(struct wl_client *client, struct wl_shell *shell, struct wl_surface *surface,
                        struct wl_input_device *input_device, uint32_t time, uint32_t edges)
{
    Q_UNUSED(client);
    Q_UNUSED(shell);
    Q_UNUSED(surface);
    Q_UNUSED(input_device);
    Q_UNUSED(time);
    Q_UNUSED(edges);
    kDebug(1212) << "Shell Resize not yet implemented";
}

static void shellCreateDrag(struct wl_client *client, struct wl_shell *shell, uint32_t id)
{
    Q_UNUSED(client);
    Q_UNUSED(shell);
    Q_UNUSED(id);
    kDebug(1212) << "Shell Create Drag not yet implemented";
}

static void shellCreateSelection(struct wl_client *client, struct wl_shell *shell, uint32_t id)
{
    Q_UNUSED(client);
    Q_UNUSED(shell);
    Q_UNUSED(id);
    kDebug(1212) << "Shell Create Selection not yet implemented";
}

static void shellSetToplevel(struct wl_client *client, struct wl_shell *shell, struct wl_surface *surface)
{
    Q_UNUSED(client);
    Q_UNUSED(shell);
    Q_UNUSED(surface);
    kDebug(1212) << "Shell Set Toplevel not yet implemented";
}

static void shellSetTransient(struct wl_client *client, struct wl_shell *shell, struct wl_surface *surface,
                                struct wl_surface *parent, int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(client);
    Q_UNUSED(shell);
    Q_UNUSED(surface);
    Q_UNUSED(parent);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(flags);
    kDebug(1212) << "Shell Set Transient not yet implemented";
}

static void shellSetFullscreen(struct wl_client *client, struct wl_shell *shell, struct wl_surface *surface)
{
    Q_UNUSED(client);
    Q_UNUSED(shell);
    Q_UNUSED(surface);
    kDebug(1212) << "Shell Set Fullscreen not yet implemented";
}

const static struct wl_shell_interface shellInterface = {
    shellMove,
    shellResize,
    shellCreateDrag,
    shellCreateSelection,
    shellSetToplevel,
    shellSetTransient,
    shellSetFullscreen
};

Shell::Shell()
    : QObject()
{
    m_object.interface = &wl_shell_interface;
    m_object.implementation = (void (**)())(&shellInterface);
}

Shell::~Shell()
{
}

} // namespace Wayland
} // namespace KWin
