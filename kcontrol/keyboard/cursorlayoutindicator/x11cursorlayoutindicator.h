/*
 * X Window System cursor layout indicator realization.
 * Copyright 2013  Victor Polevoy <vityatheboss@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef X11CURSORLAYOUTINDICATOR_H
#define X11CURSORLAYOUTINDICATOR_H

#include "cursorlayoutindicator.h"

class X11CursorImageCombiner;
class QImage;
class LayoutUnit;
class KeyboardConfig;
class CursorThemeModel;
class CursorTheme;

class X11CursorLayoutIndicator : public CursorLayoutIndicator<X11CursorImageCombiner, QImage>, public ICursorLayoutIndicator
{
private:        
    CursorThemeModel *cursorThemeModel;
    
    static const int flagImageSize;
    
    const CursorTheme* getCurrentCursorTheme() const;
    
    QImage getFlagImage(LayoutUnit &, KeyboardConfig &) const;
    QImage getCursorImage(CursorInfo::CursorType) const;
    
    void setCursorImage(CursorInfo::CursorType, QImage) const;
    
public:
    void setLayoutIndicator(CursorInfo::CursorType, LayoutUnit &, KeyboardConfig &) const;
    
    X11CursorLayoutIndicator();
    ~X11CursorLayoutIndicator();
};

#endif // X11CURSORLAYOUTINDICATOR_H
