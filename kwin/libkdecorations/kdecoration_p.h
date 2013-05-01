/*****************************************************************
This file is part of the KDE project.

Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
******************************************************************/

#ifndef KDECORATION_P_H
#define KDECORATION_P_H

//
// This header file is internal. I mean it.
//

#include "kdecoration.h"
#include <QWidget>

class KConfig;

class KDecorationOptionsPrivate : public KDecorationDefines
{
public:
    KDecorationOptionsPrivate();
    ~KDecorationOptionsPrivate();
    unsigned long updateSettings(KConfig*);   // shared implementation
    BorderSize findPreferredBorderSize(BorderSize size, QList< BorderSize >) const;   // shared implementation

    QColor colors[NUM_COLORS*2];
    QPalette *pal[NUM_COLORS*2];
    QFont activeFont, inactiveFont, activeFontSmall, inactiveFontSmall;
    QString title_buttons_left;
    QString title_buttons_right;
    bool custom_button_positions;
    bool show_tooltips;
    BorderSize border_size, cached_border_size;
    WindowOperation opMaxButtonRightClick;
    WindowOperation opMaxButtonMiddleClick;
    WindowOperation opMaxButtonLeftClick;
};

#endif
