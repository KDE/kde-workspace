/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>

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

#ifndef KWIN_ZOOM_H
#define KWIN_ZOOM_H

#include "focustrackconfig.h"

#include <kwineffects.h>
#include <QtCore/QTime>
#include <QtCore/QTimeLine>
#ifdef LibKdeAccessibilityClient_FOUND
#include <kdeaccessibilityclient/registry.h>
#endif

namespace KWin
{

class GLTexture;
class XRenderPicture;

class ZoomEffect
    : public Effect
{
    Q_OBJECT
public:
    ZoomEffect();
    virtual ~ZoomEffect();
    virtual void reconfigure(ReconfigureFlags flags);
    virtual void prePaintScreen(ScreenPrePaintData& data, int time);
    virtual void paintScreen(int mask, QRegion region, ScreenPaintData& data);
    virtual void postPaintScreen();
    virtual bool isActive() const;
private slots:
    inline void zoomIn() { zoomIn(-1.0); };
    void zoomIn(double to);
    void zoomOut();
    void actualSize();
    void moveZoomLeft();
    void moveZoomRight();
    void moveZoomUp();
    void moveZoomDown();
    void moveMouseToCenter();
    void timelineFrameChanged(int frame);
#ifdef LibKdeAccessibilityClient_FOUND
    void moveMouseToFocus();
    void focusChanged(const KAccessibleClient::AccessibleObject &object);
#endif
    void slotMouseChanged(const QPoint& pos, const QPoint& old,
                              Qt::MouseButtons buttons, Qt::MouseButtons oldbuttons,
                              Qt::KeyboardModifiers modifiers, Qt::KeyboardModifiers oldmodifiers);
private:
    void showCursor();
    void hideCursor();
    void recreateTexture();
    void moveZoom(int x, int y);
private:
    double zoom;
    double target_zoom;
    bool polling; // Mouse polling
    double zoomFactor;
    enum MouseTrackingType { MouseTrackingProportional = 0, MouseTrackingCentred = 1, MouseTrackingPush = 2, MouseTrackingDisabled = 3 };
    MouseTrackingType mouseTracking;
    enum MousePointerType { MousePointerScale = 0, MousePointerKeep = 1, MousePointerHide = 2 };
    MousePointerType mousePointer;
    QPoint cursorPoint;
    QPoint prevPoint;
    QTime lastMouseEvent;
    QTime lastFocusEvent;
    GLTexture* texture;
    XRenderPicture* xrenderPicture;
    int imageWidth;
    int imageHeight;
    bool isMouseHidden;
    QTimeLine timeline;
    int xMove, yMove;
    double moveFactor;
#ifdef LibKdeAccessibilityClient_FOUND
    bool enableFocusTracking;
    bool followFocus;
    int focusDelay;
    KAccessibleClient::Registry *registry;
    QPoint focusPoint;
#endif
};

} // namespace

#endif
