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

#include "oxygentileset_x11.h"

#include <QtGui/QPainter>

#ifdef Q_WS_X11
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#endif

namespace Oxygen
{

    //______________________________________________________________
    TileSet_x11::TileSet_x11( void )
    {
        #ifdef Q_WS_X11
        _x11Pixmaps.reserve(9);
        #endif
    }

    //______________________________________________________________
    TileSet_x11::TileSet_x11( const TileSet& other ):
        TileSet( other )
    {
        #ifdef Q_WS_X11
        // clear pixmaps, and make deep copy with explicit X11 pixmap attached
        clearPixmaps();
        foreach( const QPixmap& value, other.pixmaps() ) copyPixmap( value );
        #endif
    }

    //______________________________________________________________
    TileSet_x11& TileSet_x11::operator = ( const TileSet& other )
    {

        // call parent copy constructor
        TileSet::operator = (other);

        #ifdef Q_WS_X11
        // free X11 pixmaps
        foreach( const Qt::HANDLE& value, _x11Pixmaps  ) XFreePixmap( QX11Info::display(), value );
        _x11Pixmaps.clear();

        // clear pixmaps, and make deep copy with explicit X11 pixmap attached
        clearPixmaps();
        foreach( const QPixmap& value, other.pixmaps() ) copyPixmap( value );
        #endif

        return *this;

    }

    //______________________________________________________________
    TileSet_x11::~TileSet_x11( void )
    {
        #ifdef Q_WS_X11
        // free X11 pixmaps
        foreach( const Qt::HANDLE& value, _x11Pixmaps  ) XFreePixmap( QX11Info::display(), value );
        #endif
    }

    //______________________________________________________________
    #ifdef Q_WS_X11
    void TileSet_x11::copyPixmap( const QPixmap& source )
    {
        if( source.isNull() )
        {
            addPixmap( source );

        } else {

            Pixmap x11Pixmap = XCreatePixmap( QX11Info::display(), QX11Info::appRootWindow(), source.width(), source.height(), 32 );
            QPixmap pixmap( QPixmap::fromX11Pixmap( x11Pixmap, QPixmap::ExplicitlyShared ) );
            QPainter p(&pixmap);
            p.drawPixmap(0, 0, source);
            p.end();
            addPixmap( pixmap );
            _x11Pixmaps.push_back( x11Pixmap );

        }

        return;

    }
    #endif

    //______________________________________________________________
    #ifdef Q_WS_X11
    void TileSet_x11::initPixmap( const QPixmap &pix, int w, int h, const QRect &rect)
    {
        QSize size( w, h );
        if( !( size.isValid() && rect.isValid() ) )
        {
            addPixmap( QPixmap() );

        } else if( size != rect.size() ) {

            const QPixmap tile( pix.copy(rect) );
            Pixmap x11Pixmap( XCreatePixmap( QX11Info::display(), QX11Info::appRootWindow(), w, h, 32 ) );
            QPixmap pixmap( QPixmap::fromX11Pixmap( x11Pixmap, QPixmap::ExplicitlyShared ) );

            pixmap.fill(Qt::transparent);
            QPainter p(&pixmap);
            p.drawTiledPixmap(0, 0, w, h, tile);
            p.end();

            addPixmap( pixmap );
            _x11Pixmaps.push_back( x11Pixmap );

        } else {

            // create X11 pixmap and explicitly shared QPixmap from it
            Pixmap x11Pixmap = XCreatePixmap( QX11Info::display(), QX11Info::appRootWindow(), w, h, 32 );
            QPixmap pixmap( QPixmap::fromX11Pixmap( x11Pixmap, QPixmap::ExplicitlyShared ) );
            pixmap.fill(Qt::transparent);

            QPainter p(&pixmap);
            p.drawPixmap( QPoint(0,0), pix, rect );
            p.end();

            addPixmap( pixmap );
            _x11Pixmaps.push_back( x11Pixmap );

        }

    }
    #endif
}
