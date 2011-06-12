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

#ifndef KWIN_WAYLAND_WAYLAND_BRIDGE_H
#define KWIN_WAYLAND_WAYLAND_BRIDGE_H

#include <kdecorationbridge.h>

namespace KWin
{

namespace Wayland
{
// forward declaration
class Client;

class Bridge : public KDecorationBridgeUnstable
{

public:
    Bridge(Client *c);
    virtual ~Bridge();

    virtual KDecorationDefines::WindowOperation buttonToWindowOperation(Qt::MouseButtons button);
    virtual void displayClientMenu(int index, const QPoint& pos);
    virtual void closeAllInClientGroup();
    virtual void closeClientGroupItem(int index);
    virtual void removeFromClientGroup(int index, const QRect& newGeom);
    virtual void moveItemToClientGroup(long int itemId, int before);
    virtual void moveItemInClientGroup(int index, int before);
    virtual void setVisibleClientGroupItem(int index);
    virtual int visibleClientGroupItem();
    virtual long int itemId(int index);
    virtual QList< ClientGroupItem > clientGroupItems() const;
    virtual bool isClientGroupActive();
    virtual QRect transparentRect() const;
    virtual bool compositingActive() const;
    virtual void grabXServer(bool grab);
    virtual Qt::WFlags initialWFlags() const;
    virtual QWidget* initialParentWidget() const;
    virtual int currentDesktop() const;
    virtual void setKeepBelow(bool );
    virtual void setKeepAbove(bool );
    virtual void setShade(bool set);
    virtual void titlebarMouseWheelOperation(int delta);
    virtual void titlebarDblClickOperation();
    virtual void setDesktop(int desktop);
    virtual void showContextHelp();
    virtual void minimize();
    virtual void maximize(KDecorationDefines::MaximizeMode mode);
    virtual void closeWindow();
    virtual WId windowId() const;
    virtual QRegion unobscuredRegion(const QRegion& r) const;
    virtual QRect iconGeometry() const;
    virtual QRect geometry() const;
    virtual bool isPreview() const;
    virtual void setMask(const QRegion& , int );
    virtual void performWindowOperation(KDecorationDefines::WindowOperation );
    virtual void showWindowMenu(const QPoint& );
    virtual void showWindowMenu(const QRect& );
    virtual void processMousePressEvent(QMouseEvent* );
    virtual QString caption() const;
    virtual QIcon icon() const;
    virtual NET::WindowType windowType(long unsigned int supported_types) const;
    virtual bool isResizable() const;
    virtual bool isMovable() const;
    virtual bool keepBelow() const;
    virtual bool keepAbove() const;
    virtual bool isSetShade() const;
    virtual bool isShade() const;
    virtual bool isShadeable() const;
    virtual bool isModal() const;
    virtual int desktop() const;
    virtual bool providesContextHelp() const;
    virtual bool isMinimizable() const;
    virtual KDecorationDefines::MaximizeMode maximizeMode() const;
    virtual bool isMaximizable() const;
    virtual bool isCloseable() const;
    virtual bool isActive() const;

private:
    Client *m_client;
};

} // namespace Wayland
} // namespace KWin

#endif // KWIN_WAYLAND_WAYLAND_BRIDGE_H
