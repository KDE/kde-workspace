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
#ifndef KWIN_SCREENLOCKER_EFFECT_H
#define KWIN_SCREENLOCKER_EFFECT_H
#include <kwineffects.h>


namespace KWin
{

class EffectDeclarativeView;

class ScreenLockerEffect : public Effect
{
    Q_OBJECT
public:
    ScreenLockerEffect();
    virtual ~ScreenLockerEffect();
    virtual void paintScreen(int mask, QRegion region, ScreenPaintData &data);
    virtual void postPaintScreen();
    virtual bool provides(Feature feature);
    virtual bool isActive() const;

    virtual void windowInputMouseEvent(Window w, QEvent *e);
    virtual void grabbedKeyboardEvent(QKeyEvent *e);

public Q_SLOTS:
    void slotRequestLock();
    void slotRequestUnlock();

private:
    void doUnlock();
    void paintGL();
    void paintXrender();
    bool m_locked;
    Window m_inputWindow;
    EffectDeclarativeView *m_declarativeView;
};
} // namespace
#endif
