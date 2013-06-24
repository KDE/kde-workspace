/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Philip Falkner <philip.falkner@gmail.com>
Copyright (C) 2009 Martin Gräßlin <mgraesslin@kde.org>
Copyright (C) 2010 Alexandre Pereira <pereira.alex@gmail.com>

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

#ifndef KWIN_GLIDE_H
#define KWIN_GLIDE_H

#include <kwineffects.h>

class QTimeLine;

namespace KWin
{

class GlideEffect
    : public Effect
{
    Q_OBJECT
    Q_PROPERTY(int duration READ configuredDuration)
    Q_PROPERTY(int effect READ configuredEffect)
    Q_PROPERTY(int angle READ configuredAngle)
public:
    GlideEffect();
    ~GlideEffect();
    virtual void reconfigure(ReconfigureFlags);
    virtual void prePaintScreen(ScreenPrePaintData& data, int time);
    virtual void prePaintWindow(EffectWindow* w, WindowPrePaintData& data, int time);
    virtual void paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data);
    virtual void postPaintWindow(EffectWindow* w);
    virtual bool isActive() const;

    static bool supported();

    // for properties
    int configuredDuration() const {
        return duration;
    }
    int configuredEffect() const {
        return effect;
    }
    int configuredAngle() const {
        return angle;
    }
public Q_SLOTS:
    void slotWindowAdded(KWin::EffectWindow* c);
    void slotWindowClosed(KWin::EffectWindow *c);
    void slotWindowDeleted(KWin::EffectWindow *w);

private:
    class WindowInfo;
    typedef QMap< const EffectWindow*, WindowInfo > InfoHash;
    void glideIn(EffectWindow* w, WindowPaintData& data);
    void glideOut(EffectWindow* w, WindowPaintData& data);
    bool isGlideWindow(EffectWindow* w);
    InfoHash windows;
    float duration;
    int angle;
    enum EffectStyle {
        GlideIn = 0,
        GlideInOut = 1,
        GlideOutIn = 2,
        GlideOut = 3
    };
    EffectStyle effect;
};

class GlideEffect::WindowInfo
{
public:
    WindowInfo();
    ~WindowInfo();
    bool deleted;
    bool added;
    bool closed;
    QTimeLine *timeLine;
};

} // namespace

#endif
