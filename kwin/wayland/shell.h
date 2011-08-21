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

#ifndef KWIN_WAYLAND_SHELL_H
#define KWIN_WAYLAND_SHELL_H
#include <wayland-server.h>
#include <QtCore/QObject>

namespace KWin
{

namespace Wayland
{

/**
 * @short Wayland Shell implementation.
 *
 * This class represents a Wayland Shell. The Shell is created by the @link Server and managed
 * by that one.
 *
 * @author Philipp Brüschweiler <blei42@gmail.com>
 **/
class Shell : public QObject
{
    Q_OBJECT
public:
    Shell();
    virtual ~Shell();

    wl_object *wlObjectHandle() {
        return &m_object;
    }

private:
    wl_object m_object;
};

} // namespace Wayland

} // namespace KWin

#endif // KWIN_WAYLAND_SHELL_H
