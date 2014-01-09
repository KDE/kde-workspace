/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Lubos Lunak <l.lunak@kde.org>

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

#ifndef KWIN_SHOWPAINT_H
#define KWIN_SHOWPAINT_H

#include <kwineffects.h>

namespace KWin
{

class ShowPaintEffect
    : public Effect
{
public:
    ShowPaintEffect();
    ~ShowPaintEffect();
    virtual void paintScreen(int mask, QRegion region, ScreenPaintData& data);
    virtual void paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data);
private:
    void paintGL();
    void paintXrender();
    void paintQPainter();
    QRegion painted; // what's painted in one pass
    int color_index;
};

} // namespace

#endif
