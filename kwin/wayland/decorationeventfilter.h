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

#ifndef KWIN_WAYLAND_DECORATIONEVENTFILTER_H
#define KWIN_WAYLAND_DECORATIONEVENTFILTER_H

#include <QtCore/QObject>
#include <kdecoration.h>

namespace KWin {

namespace Wayland {
class Client;

/**
 * @short Event Filter for passing input events on the decoration to the Wayland Client.
 *
 * This class is used by @link Client as an event filter on the decoration widget.
 * The event filter intercepts all mouse and keyboard events and evaluates whether
 * they are for the decoration or for the client. Events in the client are are filtered
 * out and passed to the @link Client.
 *
 * This event filter is the glue for input events between the X11 decoration and the
 * Wayland client. It allows to use the decoration as an X11 container to receive
 * events on the X11 workspace and pass them to the non-native Wayland client running
 * in an X11 environment. The filter will not be required in case we are in a native
 * Wayland environment.
 * 
 * @author Martin Gräßlin <mgraesslin@kde.org>
 **/
class DecorationEventFilter : public QObject
{
    Q_OBJECT

public:
    /**
     * @param decoration The KDecoration whose widget is used in the event filter.
     * The decoration is used as the parent element of this QObject.
     * @param client The Wayland client to which events have to be passed.
     **/
    DecorationEventFilter(KDecoration *decoration, Client *client);
    virtual ~DecorationEventFilter();
    virtual bool eventFilter(QObject *object, QEvent *event);

private:
    KDecoration *m_decoration;
    Client *m_client;
};

}

}

#endif // KWIN_WAYLAND_DECORATIONEVENTFILTER_H
