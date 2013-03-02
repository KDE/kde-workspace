/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2007 Christian Nitschkowski <christian.nitschkowski@kdemail.net>

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

/*

 Testing of painting a window more than once.

*/

#ifndef KWIN_THUMBNAILASIDE_H
#define KWIN_THUMBNAILASIDE_H

#include <kwineffects.h>

#include <qhash.h>

namespace KWin
{

class ThumbnailAsideEffect
    : public Effect
{
    Q_OBJECT
    Q_PROPERTY(int maxWidth READ configuredMaxWidth)
    Q_PROPERTY(int spacing READ configuredSpacing)
    Q_PROPERTY(qreal opacity READ configuredOpacity)
    Q_PROPERTY(int screen READ configuredScreen)
public:
    ThumbnailAsideEffect();
    virtual void reconfigure(ReconfigureFlags);
    virtual void paintScreen(int mask, QRegion region, ScreenPaintData& data);
    virtual void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data);

    // for properties
    int configuredMaxWidth() const {
        return maxwidth;
    }
    int configuredSpacing() const {
        return spacing;
    }
    qreal configuredOpacity() const {
        return opacity;
    }
    int configuredScreen() const {
        return screen;
    }
private slots:
    void toggleCurrentThumbnail();
    void slotWindowClosed(KWin::EffectWindow *w);
    void slotWindowGeometryShapeChanged(KWin::EffectWindow *w, const QRect &old);
    void slotWindowDamaged(KWin::EffectWindow* w, const QRect& damage);
    virtual bool isActive() const;
    void repaintAll();
private:
    void addThumbnail(EffectWindow* w);
    void removeThumbnail(EffectWindow* w);
    void arrange();
    struct Data {
        EffectWindow* window; // the same like the key in the hash (makes code simpler)
        int index;
        QRect rect;
    };
    QHash< EffectWindow*, Data > windows;
    int maxwidth;
    int spacing;
    double opacity;
    int screen;
    QRegion painted;
};

} // namespace

#endif
