/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2009 Martin Gräßlin <mgraesslin@kde.org>
Copyright (C) 2009, 2010 Lucas Murray <lmurray@undefinedfire.com>

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

#ifndef KWIN_LOGOUT_H
#define KWIN_LOGOUT_H

#include <kwineffects.h>


namespace KWin
{

class GLRenderTarget;
class GLTexture;

class LogoutEffect
    : public Effect
{
    Q_OBJECT
    Q_PROPERTY(bool useBlur READ isUseBlur)
public:
    LogoutEffect();
    ~LogoutEffect();
    virtual void reconfigure(ReconfigureFlags);
    virtual void prePaintScreen(ScreenPrePaintData& data, int time);
    virtual void paintScreen(int mask, QRegion region, ScreenPaintData& data);
    virtual void postPaintScreen();
    virtual void paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data);
    virtual bool isActive() const;

    // for properties
    bool isUseBlur() const {
        return useBlur;
    }
public Q_SLOTS:
    void slotWindowAdded(KWin::EffectWindow* w);
    void slotWindowClosed(KWin::EffectWindow *w);
    void slotWindowDeleted(KWin::EffectWindow *w);
    void slotPropertyNotify(KWin::EffectWindow *w, long a);
private:
    bool isLogoutDialog(EffectWindow* w);
    double progress; // 0-1
    bool displayEffect;
    EffectWindow* logoutWindow;
    bool logoutWindowClosed;
    bool logoutWindowPassed;

    // Persistent effect
    long logoutAtom;
    bool canDoPersistent;
    EffectWindowList ignoredWindows;

    void renderVignetting();
    void renderVignettingLegacy();
    void renderBlurTexture();
    void renderBlurTextureLegacy();
    int frameDelay;
    bool blurSupported, useBlur;
    GLTexture* blurTexture;
    GLRenderTarget* blurTarget;
    double windowOpacity;
    EffectWindowList windows;
    QHash< EffectWindow*, double > windowsOpacities;
    GLShader *m_vignettingShader;
    GLShader *m_blurShader;
    QString m_shadersDir;
};

} // namespace

#endif
