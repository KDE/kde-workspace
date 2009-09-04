/*
 * Copyright 2008 Long Huynh Huu <long.upcase@googlemail.com>
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright 2007 Casper Boemann <cbr@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __OXYGEN_STYLE_HELPER_H
#define __OXYGEN_STYLE_HELPER_H

#include "lib/helper.h"

class OxygenStyleHelper : public OxygenHelper
{
public:
    explicit OxygenStyleHelper(const QByteArray &componentName);
    virtual ~OxygenStyleHelper() {}

    virtual void invalidateCaches();

    QColor calcMidColor(const QColor &color) const;

    static void fillSlab(QPainter&, const QRect&, int size = 7);
    static void fillHole(QPainter&, const QRect&, int size = 7);

    QPixmap  roundSlab(const QColor&, double shade, int size = 7);
    QPixmap  roundSlabFocused(const QColor&, const QColor &glowColor, double shade, int size = 7);

    // TODO - need to rebase scrollbars to size=7
    TileSet *slabFocused(const QColor&, const QColor &glowColor, double shade, int size = 7);
    TileSet *slabSunken(const QColor&, double shade, int size = 7);
    TileSet *slabInverted(const QColor&, double shade, int size = 7);

    TileSet *slope(const QColor&, double shade, int size = 7);

    TileSet *hole(const QColor&, double shade, int size = 7);
    TileSet *holeFlat(const QColor&, double shade, int size = 7);
    TileSet *holeFocused(const QColor&, const QColor &glowColor, double shade, int size = 7);

    TileSet *groove(const QColor&, double shade, int size = 7);

    TileSet *slitFocused(const QColor&);

    TileSet *dockFrame(const QColor&, int size);
    TileSet *scrollHole(const QColor&, Qt::Orientation orientation);

protected:

    void drawInverseShadow(QPainter&, const QColor&, int pad, int size, double fuzz) const;
    void drawInverseGlow(QPainter&, const QColor&, int pad, int size, int rsize) const;
    void drawHole(QPainter&, const QColor&, double shade, int r = 7) const;

    QCache<quint64, TileSet> m_slabSunkenCache;
    QCache<quint64, TileSet> m_slabInvertedCache;
    QCache<quint64, TileSet> m_holeCache;
    QCache<quint64, TileSet> m_holeFlatCache;
    QCache<quint64, TileSet> m_slopeCache;
    QCache<quint64, TileSet> m_grooveCache;
    QCache<quint64, TileSet> m_slitCache;
    QCache<quint64, TileSet> m_dockFrameCache;
    QCache<quint64, TileSet> m_scrollHoleCache;
};

#endif // __OXYGEN_STYLE_HELPER_H
