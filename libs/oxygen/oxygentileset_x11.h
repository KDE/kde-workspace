#ifndef oxygentileset_x11_h
#define oxygentileset_x11_h

/*
 * Copyright 2009-2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
 * Copyright 2008 Long Huynh Huu <long.upcase@googlemail.com>
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
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

#include "oxygen_export.h"
#include "oxygentileset.h"

#include <QtGui/QPixmap>
#include <QtCore/QRect>
#include <QtCore/QVector>

#include <cassert>

//! handles proper scaling of pixmap to match widget rect.
/*!
tilesets are collections of stretchable pixmaps corresponding to a given widget corners, sides, and center.
corner pixmaps are never stretched. center pixmaps are
*/
namespace Oxygen
{
    class OXYGEN_EXPORT TileSet_x11: public TileSet
    {
        public:

        //! empty constructor
        TileSet_x11();

        //! copy constructor
        TileSet_x11( const TileSet& );

        // assignment operator
        TileSet_x11& operator = (const TileSet& );

        //! destructor
        virtual ~TileSet_x11();

        #ifdef Q_WS_X11
        //! returns X11 pixmap for given index
        Qt::HANDLE x11Pixmap( int index ) const
        {
            assert( index >= 0 && index < _x11Pixmaps.size() );
            return _x11Pixmaps[index];
        }
        #endif

        protected:

        #ifdef Q_WS_X11
        //! copy pixmap inside local list
        virtual void copyPixmap( const QPixmap& );

        //! initialize pixmap
        virtual void initPixmap( const QPixmap&, int w, int h, const QRect& );

        private:

        //! list of X11 pixmaps
        QVector<Qt::HANDLE> _x11Pixmaps;
        #endif

    };

}

#endif
