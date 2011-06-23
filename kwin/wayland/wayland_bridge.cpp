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

#include "wayland_bridge.h"
#include "wayland_client.h"

#include <KDE/KDebug>
#include <workspace.h>

namespace KWin
{
namespace Wayland
{

Bridge::Bridge(Client *c)
    : m_client(c)
{
}

Bridge::~Bridge()
{
}

KDecorationDefines::WindowOperation Bridge::buttonToWindowOperation(Qt::MouseButtons button)
{
    Q_UNUSED(button)
    kDebug(1212) << "Button to Window Operation not yet implemented";
    return KDecorationDefines::NoOp;
}

void Bridge::displayClientMenu(int index, const QPoint& pos)
{
    Q_UNUSED(index)
    Q_UNUSED(pos)
    kDebug(1212) << "Display Client Menu not yet implemented";
}

void Bridge::closeAllInClientGroup()
{
    kDebug(1212) << "Close All in Client Group not yet implemented";
}

void Bridge::closeClientGroupItem(int index)
{
    Q_UNUSED(index)
    kDebug(1212) << "Close Client Group Item not yet implemented";
}

void Bridge::removeFromClientGroup(int index, const QRect& newGeom)
{
    Q_UNUSED(index)
    Q_UNUSED(newGeom)
    kDebug(1212) << "Remove From Client Group not yet implemented";
}

void Bridge::moveItemToClientGroup(long int itemId, int before)
{
    Q_UNUSED(itemId)
    Q_UNUSED(before)
    kDebug(1212) << "Move Item to Client Group not yet implemented";
}

void Bridge::moveItemInClientGroup(int index, int before)
{
    Q_UNUSED(index)
    Q_UNUSED(before)
    kDebug(1212) << "Move Item In Client Group not yet implemented";
}

void Bridge::setVisibleClientGroupItem(int index)
{
    Q_UNUSED(index)
    kDebug(1212) << "Set Visible Client Group Item not yet implemented";
}

int Bridge::visibleClientGroupItem()
{
    kDebug(1212) << "Visible Client Group Item not yet implemented";
    return -1;
}

long int Bridge::itemId(int index)
{
    Q_UNUSED(index)
    kDebug(1212) << "Item ID not yet implemented";
    return -1;
}

QList< ClientGroupItem > Bridge::clientGroupItems() const
{
    // TODO: implement real client group support
    QList< ClientGroupItem > items;
    items.append(ClientGroupItem(caption(), icon()));
    return items;
}

bool Bridge::isClientGroupActive()
{
    kDebug(1212) << "Is Client Group Active not yet implemented";
    return false;
}

QRect Bridge::transparentRect() const
{
    kDebug(1212) << "Transparent Rect not yet implemented";
    return QRect();
}

bool Bridge::compositingActive() const
{
    // Wayland is always composited
    return true;
}

void Bridge::grabXServer(bool grab)
{
    Q_UNUSED(grab)
    // cannot grab X Server from Wayland
}

Qt::WFlags Bridge::initialWFlags() const
{
    return 0;
}

QWidget* Bridge::initialParentWidget() const
{
    return NULL;
}

int Bridge::currentDesktop() const
{
    return Workspace::self()->currentDesktop();
}

void Bridge::setKeepBelow(bool below)
{
    Q_UNUSED(below)
    kDebug(1212) << "Keep Below not yet implemented";
}

void Bridge::setKeepAbove(bool above)
{
    Q_UNUSED(above)
    kDebug(1212) << "Keep Above not yet implemented";
}

void Bridge::setShade(bool set)
{
    Q_UNUSED(set)
    kDebug(1212) << "Shade not yet implemented";
}

void Bridge::titlebarMouseWheelOperation(int delta)
{
    Q_UNUSED(delta)
    kDebug(1212) << "titlebar Mouse Wheel Operation not yet implemented";
}

void Bridge::titlebarDblClickOperation()
{
    kDebug(1212) << "titlebar Mouse Wheel Operation not yet implemented";
}

void Bridge::setDesktop(int desktop)
{
    m_client->setDesktop(desktop);
}

void Bridge::showContextHelp()
{
    kDebug(1212) << "Show Context Help not yet implemented";
}

void Bridge::minimize()
{
    kDebug(1212) << "Minimize not yet implemented";
}

void Bridge::maximize(KDecorationDefines::MaximizeMode mode)
{
    Q_UNUSED(mode)
    kDebug(1212) << "Maximize not yet implemented";
}

void Bridge::closeWindow()
{
    m_client->closeWindow();
}

WId Bridge::windowId() const
{
    kDebug(1212) << "Window Id not yet implemented";
    return 0;
}

QRegion Bridge::unobscuredRegion(const QRegion& r) const
{
    kDebug(1212) << "Unobscured Region not yet implemented";
    return r;
}

QRect Bridge::iconGeometry() const
{
    kDebug(1212) << "Icon Geometry not yet implemented";
    return QRect();
}

QRect Bridge::geometry() const
{
    return m_client->geometry();
}

bool Bridge::isPreview() const
{
    // Wayland decorations are not loaded for preview
    return false;
}

void Bridge::setMask(const QRegion &mask, int mode)
{
    Q_UNUSED(mask)
    Q_UNUSED(mode)
    kDebug(1212) << "Set Mask not supported on Wayland";
}

void Bridge::performWindowOperation(KDecorationDefines::WindowOperation operation)
{
    Q_UNUSED(operation)
    kDebug(1212) << "Perform Window Operation not yet implemented";
}

void Bridge::showWindowMenu(const QPoint &point)
{
    Q_UNUSED(point)
    kDebug(1212) << "Show Window Menu not yet implemented";
}

void Bridge::showWindowMenu(const QRect &rect)
{
    Q_UNUSED(rect)
    kDebug(1212) << "Show Window Menu not yet implemented";
}

void Bridge::processMousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    kDebug(1212) << "Process Mouse Event not yet implemented";
}

QString Bridge::caption() const
{
    return QString("Wayland Client");
}

QIcon Bridge::icon() const
{
    return KIcon("kwin", NULL);
}

NET::WindowType Bridge::windowType(long unsigned int supported_types) const
{
    Q_UNUSED(supported_types)
    // currently all Wayland windows are normal
    return NET::Normal;
}

bool Bridge::isResizable() const
{
    return false;
}

bool Bridge::isMovable() const
{
    return true;
}

bool Bridge::keepBelow() const
{
    kDebug(1212) << "Keep Below not yet implemented";
    return false;
}

bool Bridge::keepAbove() const
{
    kDebug(1212) << "Keep Above not yet implemented";
    return false;
}

bool Bridge::isSetShade() const
{
    return false;
}

bool Bridge::isShade() const
{
    return false;
}

bool Bridge::isShadeable() const
{
    return false;
}

bool Bridge::isModal() const
{
    return false;
}

int Bridge::desktop() const
{
    return m_client->desktop();
}

bool Bridge::providesContextHelp() const
{
    return false;
}

bool Bridge::isMinimizable() const
{
    return false;
}

KDecorationDefines::MaximizeMode Bridge::maximizeMode() const
{
    return KDecorationDefines::MaximizeRestore;
}

bool Bridge::isMaximizable() const
{
    return false;
}

bool Bridge::isCloseable() const
{
    return true;
}

bool Bridge::isActive() const
{
    return m_client->isActive();
}

} // namespace Wayland
} // namespace KWin
