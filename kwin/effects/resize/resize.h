/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2009 Martin Gräßlin <mgraesslin@kde.org>

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

#ifndef KWIN_RESIZE_H
#define KWIN_RESIZE_H

#include <kwinanimationeffect.h>

namespace KWin
{

class ResizeEffect
    : public AnimationEffect
{
    Q_OBJECT
    Q_PROPERTY(bool textureScale READ isTextureScale)
    Q_PROPERTY(bool outline READ isOutline)
public:
    ResizeEffect();
    ~ResizeEffect();
    virtual inline bool provides(Effect::Feature ef) {
        return ef == Effect::Resize;
    }
    inline bool isActive() const { return m_active || AnimationEffect::isActive(); }
    virtual void prePaintScreen(ScreenPrePaintData& data, int time);
    virtual void prePaintWindow(EffectWindow* w, WindowPrePaintData& data, int time);
    virtual void paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data);
    virtual void reconfigure(ReconfigureFlags);

    bool isTextureScale() const {
        return m_features & TextureScale;
    }
    bool isOutline() const {
        return m_features & Outline;
    }

public Q_SLOTS:
    void slotWindowStartUserMovedResized(KWin::EffectWindow *w);
    void slotWindowStepUserMovedResized(KWin::EffectWindow *w, const QRect &geometry);
    void slotWindowFinishUserMovedResized(KWin::EffectWindow *w);

private:
    enum Feature { TextureScale = 1 << 0, Outline = 1 << 1 };
    bool m_active;
    int m_features;
    EffectWindow* m_resizeWindow;
    QRect m_currentGeometry, m_originalGeometry;
};

}

#endif
