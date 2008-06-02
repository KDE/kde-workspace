/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

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

 This file contains things relevant to geometry, i.e. workspace size,
 window positions and window sizes.

*/

#include "client.h"
#include "workspace.h"

#include <kapplication.h>
#include <kglobal.h>
#include <QPainter>
#include <kwindowsystem.h>

#include "placement.h"
#include "notifications.h"
#include "geometrytip.h"
#include "rules.h"
#include "effects.h"
#include <QX11Info>
#include <QDesktopWidget>

namespace KWin
{

//********************************************
// Workspace
//********************************************

/*!
  Resizes the workspace after an XRANDR screen size change
 */
void Workspace::desktopResized()
    {
    QRect geom = QApplication::desktop()->geometry();
    NETSize desktop_geometry;
    desktop_geometry.width = geom.width();
    desktop_geometry.height = geom.height();
    rootInfo->setDesktopGeometry( -1, desktop_geometry );

    updateClientArea();
    destroyElectricBorders();
    updateElectricBorders();
    if( compositing() )
        {
        finishCompositing();
        QTimer::singleShot( 0, this, SLOT( setupCompositing() ) );
        }
    }

/*!
  Updates the current client areas according to the current clients.

  If the area changes or force is true, the new areas are propagated to the world.

  The client area is the area that is available for clients (that
  which is not taken by windows like panels, the top-of-screen menu
  etc).

  \sa clientArea()
 */

void Workspace::updateClientArea( bool force )
    {
    QDesktopWidget *desktopwidget = KApplication::desktop();
    int nscreens = desktopwidget -> numScreens ();
//    kDebug () << "screens: " << nscreens;
    QVector< QRect > new_wareas( numberOfDesktops() + 1 );
    QVector< QVector< QRect > > new_sareas( numberOfDesktops() + 1 );
    QVector< QRect > screens( nscreens );
    QRect desktopArea = desktopwidget -> geometry ();
    for( int iS = 0;
            iS < nscreens;
            iS ++ )
        {
            screens [iS] = desktopwidget -> screenGeometry (iS);
        }
    for( int i = 1;
            i <= numberOfDesktops();
            ++i )
        {
            new_wareas[ i ] = desktopArea;
            new_sareas[ i ].resize( nscreens );
            for( int iS = 0;
                    iS < nscreens;
                    iS ++ )
                new_sareas[ i ][ iS ] = screens[ iS ];
        }
    for ( ClientList::ConstIterator it = clients.begin(); it != clients.end(); ++it)
        {
            if( !(*it)->hasStrut())
                continue;
            QRect r = (*it)->adjustedClientArea( desktopArea, desktopArea );
            if( (*it)->isOnAllDesktops())
                for( int i = 1;
                        i <= numberOfDesktops();
                        ++i )
                    {
                        new_wareas[ i ] = new_wareas[ i ].intersect( r );
                        for( int iS = 0;
                                iS < nscreens;
                                iS ++ )
                            new_sareas[ i ][ iS ] =
                                new_sareas[ i ][ iS ].intersect(
                                        (*it)->adjustedClientArea( desktopArea, screens[ iS ] )
                                    );
                    }
            else
                {
                    new_wareas[ (*it)->desktop() ] = new_wareas[ (*it)->desktop() ].intersect( r );
                    for( int iS = 0;
                            iS < nscreens;
                            iS ++ )
                        {
//                            kDebug () << "adjusting new_sarea: " << screens[ iS ];
                            new_sareas[ (*it)->desktop() ][ iS ] =
                                new_sareas[ (*it)->desktop() ][ iS ].intersect(
                                        (*it)->adjustedClientArea( desktopArea, screens[ iS ] )
                                        );
                        }
                }
        }
#if 0
    for( int i = 1;
            i <= numberOfDesktops();
            ++i )
        {
            for( int iS = 0;
                    iS < nscreens;
                    iS ++ )
                kDebug () << "new_sarea: " << new_sareas[ i ][ iS ];
        }
#endif
    // TODO topmenu update for screenarea changes?
    if( topmenu_space != NULL )
        {
        QRect topmenu_area = desktopArea;
        topmenu_area.setTop( topMenuHeight());
        for( int i = 1;
             i <= numberOfDesktops();
             ++i )
            new_wareas[ i ] = new_wareas[ i ].intersect( topmenu_area );
        }

    bool changed = force;

    if(screenarea.isEmpty())
        changed = true;

    for( int i = 1;
         !changed && i <= numberOfDesktops();
         ++i )
        {
            if( workarea[ i ] != new_wareas[ i ] )
                changed = true;
            for( int iS = 0;
                    iS < nscreens;
                    iS ++ )
                if (new_sareas[ i ][ iS ] != screenarea [ i ][ iS ])
                    changed = true;
        }

    if ( changed )
        {
        workarea = new_wareas;
        screenarea = new_sareas;
        NETRect r;
        for( int i = 1; i <= numberOfDesktops(); i++)
            {
            r.pos.x = workarea[ i ].x();
            r.pos.y = workarea[ i ].y();
            r.size.width = workarea[ i ].width();
            r.size.height = workarea[ i ].height();
            rootInfo->setWorkArea( i, r );
            }

        updateTopMenuGeometry();
        for( ClientList::ConstIterator it = clients.begin();
             it != clients.end();
             ++it)
            (*it)->checkWorkspacePosition();
        for( ClientList::ConstIterator it = desktops.begin();
             it != desktops.end();
             ++it)
            (*it)->checkWorkspacePosition();
        }
    }

void Workspace::updateClientArea()
    {
    updateClientArea( false );
    }


/*!
  returns the area available for clients. This is the desktop
  geometry minus windows on the dock.  Placement algorithms should
  refer to this rather than geometry().

  \sa geometry()
 */
QRect Workspace::clientArea( clientAreaOption opt, int screen, int desktop ) const
    {
    if( desktop == NETWinInfo::OnAllDesktops || desktop == 0 )
        desktop = currentDesktop();
    if( screen == -1 )
        screen = activeScreen();
    QDesktopWidget *desktopwidget = KApplication::desktop();
    QRect sarea = !screenarea.isEmpty() // may be empty during KWin initialization
        ? screenarea[ desktop ][ screen ]
        : desktopwidget->screenGeometry( screen );
    QRect warea = workarea[ desktop ].isNull()
        ? QApplication::desktop()->geometry()
        : workarea[ desktop ];
    switch (opt)
        {
        case MaximizeArea:
            if (options->xineramaMaximizeEnabled)
                return sarea;
            else
                return warea;
        case MaximizeFullArea:
            if (options->xineramaMaximizeEnabled)
                return desktopwidget->screenGeometry( screen );
            else
                return desktopwidget->geometry();
        case FullScreenArea:
            if (options->xineramaFullscreenEnabled)
                return desktopwidget->screenGeometry( screen );
            else
                return desktopwidget->geometry();
        case PlacementArea:
            if (options->xineramaPlacementEnabled)
                return sarea;
            else
                return warea;
        case MovementArea:
            if (options->xineramaMovementEnabled)
                return desktopwidget->screenGeometry( screen );
            else
                return desktopwidget->geometry();
        case WorkArea:
            return warea;
        case FullArea:
            return desktopwidget->geometry();
        case ScreenArea:
            return desktopwidget->screenGeometry( screen );
        }
    assert( false );
    return QRect();
    }

QRect Workspace::clientArea( clientAreaOption opt, const QPoint& p, int desktop ) const
    {
    QDesktopWidget *desktopwidget = KApplication::desktop();
    int screen = desktopwidget->isVirtualDesktop() ? desktopwidget->screenNumber( p ) : desktopwidget->primaryScreen();
    if( screen < 0 )
        screen = desktopwidget->primaryScreen();
    return clientArea( opt, screen, desktop );
    }

QRect Workspace::clientArea( clientAreaOption opt, const Client* c ) const
    {
    return clientArea( opt, c->geometry().center(), c->desktop());
    }


/*!
  Client \a c is moved around to position \a pos. This gives the
  workspace the opportunity to interveniate and to implement
  snap-to-windows functionality.
 */
QPoint Workspace::adjustClientPosition( Client* c, QPoint pos )
    {
   //CT 16mar98, 27May98 - magics: BorderSnapZone, WindowSnapZone
   //CT adapted for kwin on 25Nov1999
   //aleXXX 02Nov2000 added second snapping mode
    if (options->windowSnapZone || options->borderSnapZone )
        {
        const bool sOWO=options->snapOnlyWhenOverlapping;
        const QRect maxRect = clientArea(MovementArea, pos+c->rect().center(), c->desktop());
        const int xmin = maxRect.left();
        const int xmax = maxRect.right()+1;               //desk size
        const int ymin = maxRect.top();
        const int ymax = maxRect.bottom()+1;

        const int cx(pos.x());
        const int cy(pos.y());
        const int cw(c->width());
        const int ch(c->height());
        const int rx(cx+cw);
        const int ry(cy+ch);                 //these don't change

        int nx(cx), ny(cy);                         //buffers
        int deltaX(xmax);
        int deltaY(ymax);   //minimum distance to other clients

        int lx, ly, lrx, lry; //coords and size for the comparison client, l

      // border snap
        int snap = options->borderSnapZone; //snap trigger
        if (snap)
            {
            if ((sOWO?(cx<xmin):true) && (qAbs(xmin-cx)<snap))
                {
                deltaX = xmin-cx;
                nx = xmin;
                }
            if ((sOWO?(rx>xmax):true) && (qAbs(rx-xmax)<snap) && (qAbs(xmax-rx) < deltaX))
                {
                deltaX = rx-xmax;
                nx = xmax - cw;
                }

            if ((sOWO?(cy<ymin):true) && (qAbs(ymin-cy)<snap))
                {
                deltaY = ymin-cy;
                ny = ymin;
                }
            if ((sOWO?(ry>ymax):true) && (qAbs(ry-ymax)<snap) && (qAbs(ymax-ry) < deltaY))
                {
                deltaY =ry-ymax;
                ny = ymax - ch;
                }
            }

      // windows snap
        snap = options->windowSnapZone;
        if (snap)
            {
            QList<Client *>::ConstIterator l;
            for (l = clients.begin();l != clients.end();++l )
                {
                if ((*l)->isOnDesktop(currentDesktop()) &&
                   !(*l)->isMinimized()
                    && (*l) != c )
                    {
                    lx = (*l)->x();
                    ly = (*l)->y();
                    lrx = lx + (*l)->width();
                    lry = ly + (*l)->height();

                    if ( (( cy <= lry ) && ( cy  >= ly  ))  ||
                         (( ry >= ly  ) && ( ry  <= lry ))  ||
                         (( cy <= ly  ) && ( ry >= lry  )) )
                        {
                        if ((sOWO?(cx<lrx):true) && (qAbs(lrx-cx)<snap) && ( qAbs(lrx -cx) < deltaX) )
                            {
                            deltaX = qAbs( lrx - cx );
                            nx = lrx;
                            }
                        if ((sOWO?(rx>lx):true) && (qAbs(rx-lx)<snap) && ( qAbs( rx - lx )<deltaX) )
                            {
                            deltaX = qAbs(rx - lx);
                            nx = lx - cw;
                            }
                        }

                    if ( (( cx <= lrx ) && ( cx  >= lx  ))  ||
                         (( rx >= lx  ) && ( rx  <= lrx ))  ||
                         (( cx <= lx  ) && ( rx >= lrx  )) )
                        {
                        if ((sOWO?(cy<lry):true) && (qAbs(lry-cy)<snap) && (qAbs( lry -cy ) < deltaY))
                            {
                            deltaY = qAbs( lry - cy );
                            ny = lry;
                            }
                  //if ( (qAbs( ry-ly ) < snap) && (qAbs( ry - ly ) < deltaY ))
                        if ((sOWO?(ry>ly):true) && (qAbs(ry-ly)<snap) && (qAbs( ry - ly ) < deltaY ))
                            {
                            deltaY = qAbs( ry - ly );
                            ny = ly - ch;
                            }
                        }
                    }
                }
            }
        pos = QPoint(nx, ny);
        }
    return pos;
    }

QRect Workspace::adjustClientSize( Client* c, QRect moveResizeGeom, int mode )
    {
   //adapted from adjustClientPosition on 29May2004
   //this function is called when resizing a window and will modify
   //the new dimensions to snap to other windows/borders if appropriate
    if ( options->windowSnapZone || options->borderSnapZone  )
        {
        const bool sOWO=options->snapOnlyWhenOverlapping;

        const QRect maxRect = clientArea(MovementArea, c->rect().center(), c->desktop());
        const int xmin = maxRect.left();
        const int xmax = maxRect.right();               //desk size
        const int ymin = maxRect.top();
        const int ymax = maxRect.bottom();

        const int cx(moveResizeGeom.left());
        const int cy(moveResizeGeom.top());
        const int rx(moveResizeGeom.right());
        const int ry(moveResizeGeom.bottom());

        int newcx(cx), newcy(cy);                         //buffers
        int newrx(rx), newry(ry);
        int deltaX(xmax);
        int deltaY(ymax);   //minimum distance to other clients

        int lx, ly, lrx, lry; //coords and size for the comparison client, l

      // border snap
        int snap = options->borderSnapZone; //snap trigger
        if (snap)
            {
            deltaX = int(snap);
            deltaY = int(snap);

#define SNAP_BORDER_TOP \
            if ((sOWO?(newcy<ymin):true) && (qAbs(ymin-newcy)<deltaY)) \
              { \
                deltaY = qAbs(ymin-newcy); \
                newcy = ymin; \
               }

#define SNAP_BORDER_BOTTOM \
            if ((sOWO?(newry>ymax):true) && (qAbs(ymax-newry)<deltaY)) \
              { \
                deltaY = qAbs(ymax-newcy); \
                newry = ymax; \
               }

#define SNAP_BORDER_LEFT \
            if ((sOWO?(newcx<xmin):true) && (qAbs(xmin-newcx)<deltaX)) \
              { \
                deltaX = qAbs(xmin-newcx); \
                newcx = xmin; \
               }

#define SNAP_BORDER_RIGHT \
            if ((sOWO?(newrx>xmax):true) && (qAbs(xmax-newrx)<deltaX)) \
              { \
                deltaX = qAbs(xmax-newrx); \
                newrx = xmax; \
               }
                     switch ( mode )
                      {
                      case PositionBottomRight:
                        SNAP_BORDER_BOTTOM
                        SNAP_BORDER_RIGHT
                        break;
                      case PositionRight:
                        SNAP_BORDER_RIGHT
                        break;
                      case PositionBottom:
                        SNAP_BORDER_BOTTOM
                        break;
                      case PositionTopLeft:
                        SNAP_BORDER_TOP
                        SNAP_BORDER_LEFT
                        break;
                      case PositionLeft:
                        SNAP_BORDER_LEFT
                        break;
                      case PositionTop:
                        SNAP_BORDER_TOP
                        break;
                      case PositionTopRight:
                        SNAP_BORDER_TOP
                        SNAP_BORDER_RIGHT
                        break;
                      case PositionBottomLeft:
                        SNAP_BORDER_BOTTOM
                        SNAP_BORDER_LEFT
                        break;
                      default:
                        assert( false );
                        break;
                      }


            }

      // windows snap
        snap = options->windowSnapZone;
        if (snap)
            {
            deltaX = int(snap);
            deltaY = int(snap);
            QList<Client *>::ConstIterator l;
            for (l = clients.begin();l != clients.end();++l )
                {
                if ((*l)->isOnDesktop(currentDesktop()) &&
                   !(*l)->isMinimized()
                    && (*l) != c )
                    {
                    lx = (*l)->x()-1;
                    ly = (*l)->y()-1;
                    lrx =(*l)->x() + (*l)->width();
                    lry =(*l)->y() + (*l)->height();

#define WITHIN_HEIGHT ((( newcy <= lry ) && ( newcy  >= ly  ))  || \
                         (( newry >= ly  ) && ( newry  <= lry ))  || \
                         (( newcy <= ly  ) && ( newry >= lry  )) )

#define WITHIN_WIDTH  ( (( cx <= lrx ) && ( cx  >= lx  ))  || \
                         (( rx >= lx  ) && ( rx  <= lrx ))  || \
                         (( cx <= lx  ) && ( rx >= lrx  )) )

#define SNAP_WINDOW_TOP  if ( (sOWO?(newcy<lry):true) \
                  && WITHIN_WIDTH  \
                  && (qAbs( lry - newcy ) < deltaY) ) {  \
                  deltaY = qAbs( lry - newcy ); \
                  newcy=lry; \
                  }

#define SNAP_WINDOW_BOTTOM  if ( (sOWO?(newry>ly):true)  \
                     && WITHIN_WIDTH  \
                     && (qAbs( ly - newry ) < deltaY) ) {  \
                     deltaY = qAbs( ly - newry );  \
                     newry=ly;  \
                     }

#define SNAP_WINDOW_LEFT  if ( (sOWO?(newcx<lrx):true)  \
                   && WITHIN_HEIGHT  \
                   && (qAbs( lrx - newcx ) < deltaX)) {  \
                   deltaX = qAbs( lrx - newcx );  \
                   newcx=lrx;  \
                   }

#define SNAP_WINDOW_RIGHT  if ( (sOWO?(newrx>lx):true)  \
                    && WITHIN_HEIGHT  \
                    && (qAbs( lx - newrx ) < deltaX))  \
                    {  \
                    deltaX = qAbs( lx - newrx );  \
                    newrx=lx;  \
                    }

                    switch ( mode )
                      {
                      case PositionBottomRight:
                        SNAP_WINDOW_BOTTOM
                        SNAP_WINDOW_RIGHT
                        break;
                      case PositionRight:
                        SNAP_WINDOW_RIGHT
                        break;
                      case PositionBottom:
                        SNAP_WINDOW_BOTTOM
                        break;
                      case PositionTopLeft:
                        SNAP_WINDOW_TOP
                        SNAP_WINDOW_LEFT
                        break;
                      case PositionLeft:
                        SNAP_WINDOW_LEFT
                        break;
                      case PositionTop:
                        SNAP_WINDOW_TOP
                        break;
                      case PositionTopRight:
                        SNAP_WINDOW_TOP
                        SNAP_WINDOW_RIGHT
                        break;
                      case PositionBottomLeft:
                        SNAP_WINDOW_BOTTOM
                        SNAP_WINDOW_LEFT
                        break;
                      default:
                        assert( false );
                        break;
                      }
                    }
                }
            }
       moveResizeGeom = QRect(QPoint(newcx, newcy), QPoint(newrx, newry));
       }
    return moveResizeGeom;
    }

/*!
  Marks the client as being moved around by the user.
 */
void Workspace::setClientIsMoving( Client *c )
    {
    Q_ASSERT(!c || !movingClient); // Catch attempts to move a second
    // window while still moving the first one.
    movingClient = c;
    if (movingClient)
        ++block_focus;
    else
        --block_focus;
    }

/*!
  Cascades all clients on the current desktop
 */
void Workspace::cascadeDesktop()
    {
// TODO XINERAMA this probably is not right for xinerama
    Q_ASSERT( block_stacking_updates == 0 );
    ClientList::ConstIterator it(stackingOrder().begin());
    initPositioning->reinitCascading( currentDesktop());
    QRect area = clientArea( PlacementArea, QPoint( 0, 0 ), currentDesktop());
    for (; it != stackingOrder().end(); ++it)
        {
        if((!(*it)->isOnDesktop(currentDesktop())) ||
           ((*it)->isMinimized())                  ||
           ((*it)->isOnAllDesktops())              ||
           (!(*it)->isMovable()) )
            continue;
        initPositioning->placeCascaded(*it, area);
        }
    }

/*!
  Unclutters the current desktop by smart-placing all clients
  again.
 */
void Workspace::unclutterDesktop()
    {
	for ( int i = clients.size() - 1; i>=0; i-- )
        {
        if( ( !clients.at( i )->isOnDesktop( currentDesktop() ) ) ||
           (clients.at( i )->isMinimized())                  ||
           (clients.at( i )->isOnAllDesktops())              ||
           (!clients.at( i )->isMovable()) )
            continue;
        initPositioning->placeSmart(clients.at(  i ), QRect());
        }
    }


void Workspace::updateTopMenuGeometry( Client* c )
    {
    if( !managingTopMenus())
        return;
    if( c != NULL )
        {
        XEvent ev;
        ev.xclient.display = display();
        ev.xclient.type = ClientMessage;
        ev.xclient.window = c->window();
        static Atom msg_type_atom = XInternAtom( display(), "_KDE_TOPMENU_MINSIZE", False );
        ev.xclient.message_type = msg_type_atom;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = xTime();
        ev.xclient.data.l[1] = topmenu_space->width();
        ev.xclient.data.l[2] = topmenu_space->height();
        ev.xclient.data.l[3] = 0;
        ev.xclient.data.l[4] = 0;
        XSendEvent( display(), c->window(), False, NoEventMask, &ev );
        KWindowSystem::setStrut( c->window(), 0, 0, topmenu_height, 0 ); // so that kicker etc. know
        c->checkWorkspacePosition();
        return;
        }
    // c == NULL - update all, including topmenu_space
    QRect area;
    area = clientArea( MaximizeFullArea, QPoint( 0, 0 ), 1 ); // HACK desktop ?
    area.setHeight( topMenuHeight());
    topmenu_space->setGeometry( area );
    for( ClientList::ConstIterator it = topmenus.begin();
         it != topmenus.end();
         ++it )
        updateTopMenuGeometry( *it );
    }

//********************************************
// Client
//********************************************


void Client::keepInArea( QRect area, bool partial )
    {
    if( partial )
        {
        // increase the area so that can have only 100 pixels in the area
        area.setLeft( qMin( area.left() - width() + 100, area.left()));
        area.setTop( qMin( area.top() - height() + 100, area.top()));
        area.setRight( qMax( area.right() + width() - 100, area.right()));
        area.setBottom( qMax( area.bottom() + height() - 100, area.bottom()));
        }
    if ( geometry().right() > area.right() && width() < area.width() )
        move( area.right() - width(), y() );
    if ( geometry().bottom() > area.bottom() && height() < area.height() )
        move( x(), area.bottom() - height() );
    if( !area.contains( geometry().topLeft() ))
        {
        int tx = x();
        int ty = y();
        if ( tx < area.x() )
            tx = area.x();
        if ( ty < area.y() )
            ty = area.y();
        move( tx, ty );
        }
    }

/*!
  Returns \a area with the client's strut taken into account.

  Used from Workspace in updateClientArea.
 */
// TODO move to Workspace?

QRect Client::adjustedClientArea( const QRect &desktopArea, const QRect& area ) const
    {
    QRect r = area;
    // topmenu area is reserved in updateClientArea()
    if( isTopMenu())
        return r;
    NETExtendedStrut str = strut();
    QRect stareaL = QRect(
            0,
            str . left_start,
            str . left_width,
            str . left_end - str . left_start + 1 );
    QRect stareaR = QRect (
            desktopArea . right () - str . right_width + 1,
            str . right_start,
            str . right_width,
            str . right_end - str . right_start + 1 );
    QRect stareaT = QRect (
            str . top_start,
            0,
            str . top_end - str . top_start + 1,
            str . top_width);
    QRect stareaB = QRect (
            str . bottom_start,
            desktopArea . bottom () - str . bottom_width + 1,
            str . bottom_end - str . bottom_start + 1,
            str . bottom_width);

    QRect screenarea = workspace()->clientArea( ScreenArea, this );
    // HACK: workarea handling is not xinerama aware, so if this strut
    // reserves place at a xinerama edge that's inside the virtual screen,
    // ignore the strut for workspace setting.
    if( area == kapp->desktop()->geometry())
        {
        if( stareaL.left() < screenarea.left())
            stareaL = QRect();
        if( stareaR.right() > screenarea.right())
            stareaR = QRect();
        if( stareaT.top() < screenarea.top())
            stareaT = QRect();
        if( stareaB.bottom() < screenarea.bottom())
            stareaB = QRect();
        }
    // Handle struts at xinerama edges that are inside the virtual screen.
    // They're given in virtual screen coordinates, make them affect only
    // their xinerama screen.
    stareaL.setLeft( qMax( stareaL.left(), screenarea.left()));
    stareaR.setRight( qMin( stareaR.right(), screenarea.right()));
    stareaT.setTop( qMax( stareaT.top(), screenarea.top()));
    stareaB.setBottom( qMin( stareaB.bottom(), screenarea.bottom()));

    if (stareaL . intersects (area)) {
//        kDebug () << "Moving left of: " << r << " to " << stareaL.right() + 1;
        r . setLeft( stareaL . right() + 1 );
    }
    if (stareaR . intersects (area)) {
//        kDebug () << "Moving right of: " << r << " to " << stareaR.left() - 1;
        r . setRight( stareaR . left() - 1 );
    }
    if (stareaT . intersects (area)) {
//        kDebug () << "Moving top of: " << r << " to " << stareaT.bottom() + 1;
        r . setTop( stareaT . bottom() + 1 );
    }
    if (stareaB . intersects (area)) {
//        kDebug () << "Moving bottom of: " << r << " to " << stareaB.top() - 1;
        r . setBottom( stareaB . top() - 1 );
    }
    return r;
    }

NETExtendedStrut Client::strut() const
    {
    NETExtendedStrut ext = info->extendedStrut();
    NETStrut str = info->strut();
    if( ext.left_width == 0 && ext.right_width == 0 && ext.top_width == 0 && ext.bottom_width == 0
        && ( str.left != 0 || str.right != 0 || str.top != 0 || str.bottom != 0 ))
        {
        // build extended from simple
        if( str.left != 0 )
            {
            ext.left_width = str.left;
            ext.left_start = 0;
            ext.left_end = displayHeight();
            }
        if( str.right != 0 )
            {
            ext.right_width = str.right;
            ext.right_start = 0;
            ext.right_end = displayHeight();
            }
        if( str.top != 0 )
            {
            ext.top_width = str.top;
            ext.top_start = 0;
            ext.top_end = displayWidth();
            }
        if( str.bottom != 0 )
            {
            ext.bottom_width = str.bottom;
            ext.bottom_start = 0;
            ext.bottom_end = displayWidth();
            }
        }
    return ext;
    }

bool Client::hasStrut() const
    {
    NETExtendedStrut ext = strut();
    if( ext.left_width == 0 && ext.right_width == 0 && ext.top_width == 0 && ext.bottom_width == 0 )
        return false;
    return true;
    }


// updates differences to workarea edges for all directions
void Client::updateWorkareaDiffs()
    {
    QRect area = workspace()->clientArea( WorkArea, this );
    QRect geom = geometry();
    workarea_diff_x = computeWorkareaDiff( geom.left(), geom.right(), area.left(), area.right());
    workarea_diff_y = computeWorkareaDiff( geom.top(), geom.bottom(), area.top(), area.bottom());
    }

// If the client was inside workarea in the x direction, and if it was close to the left/right
// edge, return the distance from the left/right edge (negative for left, positive for right)
// INT_MIN means 'not inside workarea', INT_MAX means 'not near edge'.
// In order to recognize 'at the left workarea edge' from 'at the right workarea edge'
// (i.e. negative vs positive zero), the distances are one larger in absolute value than they
// really are (i.e. 5 pixels from the left edge is -6, not -5). A bit hacky, but I'm lazy
// to rewrite it just to make it nicer. If this will ever get touched again, perhaps then.
// the y direction is done the same, just the values will be rotated: top->left, bottom->right
int Client::computeWorkareaDiff( int left, int right, int a_left, int a_right )
    {
    int left_diff = left - a_left;
    int right_diff = a_right - right;
    if( left_diff < 0 || right_diff < 0 )
        return INT_MIN;
    else // fully inside workarea in this direction direction
        {
        // max distance from edge where it's still considered to be close and is kept at that distance
        int max_diff = ( a_right - a_left ) / 10;
        if( left_diff < right_diff )
            return left_diff < max_diff ? -left_diff - 1 : INT_MAX;
        else if( left_diff > right_diff )
            return right_diff < max_diff ? right_diff + 1 : INT_MAX;
        return INT_MAX; // not close to workarea edge
        }
    }

void Client::checkWorkspacePosition()
    {
    if( isDesktop())
        {
        QRect area = workspace()->clientArea( FullArea, this );
        if( geometry() != area )
            setGeometry( area );
        return;
        }
    if( isFullScreen())
        {
        QRect area = workspace()->clientArea( FullScreenArea, this );
        if( geometry() != area )
            setGeometry( area );
        return;
        }
    if( isDock())
        return;
    if( isTopMenu())
        {
        if( workspace()->managingTopMenus())
            {
            QRect area;
            ClientList mainclients = mainClients();
            if( mainclients.count() == 1 )
                area = workspace()->clientArea( MaximizeFullArea, mainclients.first());
            else
                area = workspace()->clientArea( MaximizeFullArea, QPoint( 0, 0 ), desktop());
            area.setHeight( workspace()->topMenuHeight());
//            kDebug() << "TOPMENU size adjust: " << area << ":" << this;
            setGeometry( area );
            }
        return;
        }

    if( maximizeMode() != MaximizeRestore )
	// TODO update geom_restore?
        changeMaximize( false, false, true ); // adjust size

    if( !isShade()) // TODO
        {
        int old_diff_x = workarea_diff_x;
        int old_diff_y = workarea_diff_y;
        updateWorkareaDiffs();

        // this can be true only if this window was mapped before KWin
        // was started - in such case, don't adjust position to workarea,
        // because the window already had its position, and if a window
        // with a strut altering the workarea would be managed in initialization
        // after this one, this window would be moved
        if( workspace()->initializing())
            return;

        QRect area = workspace()->clientArea( WorkArea, this );
        QRect new_geom = geometry();
        QRect tmp_rect_x( new_geom.left(), 0, new_geom.width(), 0 );
        QRect tmp_area_x( area.left(), 0, area.width(), 0 );
        checkDirection( workarea_diff_x, old_diff_x, tmp_rect_x, tmp_area_x );
        // the x<->y swapping
        QRect tmp_rect_y( new_geom.top(), 0, new_geom.height(), 0 );
        QRect tmp_area_y( area.top(), 0, area.height(), 0 );
        checkDirection( workarea_diff_y, old_diff_y, tmp_rect_y, tmp_area_y );
        new_geom = QRect( tmp_rect_x.left(), tmp_rect_y.left(), tmp_rect_x.width(), tmp_rect_y.width());
        QRect final_geom( new_geom.topLeft(), adjustedSize( new_geom.size()));
        if( final_geom != new_geom ) // size increments, or size restrictions
            { // adjusted size differing matters only for right and bottom edge
            if( old_diff_x != INT_MAX && old_diff_x > 0 )
                final_geom.moveRight( area.right() - ( old_diff_x - 1 ));
            if( old_diff_y != INT_MAX && old_diff_y > 0 )
                final_geom.moveBottom( area.bottom() - ( old_diff_y - 1 ));
            }
        if( final_geom != geometry() )
            setGeometry( final_geom );
        //    updateWorkareaDiffs(); done already by setGeometry()
        }
    }

// Try to be smart about keeping the clients visible.
// If the client was fully inside the workspace before, try to keep
// it still inside the workarea, possibly moving it or making it smaller if possible,
// and try to keep the distance from the nearest workarea edge.
// On the other hand, it it was partially moved outside of the workspace in some direction,
// don't do anything with that direction if it's still at least partially visible. If it's
// not visible anymore at all, make sure it's visible at least partially
// again (not fully, as that could(?) be potentionally annoying) by
// moving it slightly inside the workarea (those '+ 5').
// Again, this is done for the x direction, y direction will be done by x<->y swapping
void Client::checkDirection( int new_diff, int old_diff, QRect& rect, const QRect& area )
    {
    if( old_diff != INT_MIN ) // was inside workarea
        {
        if( old_diff == INT_MAX ) // was in workarea, but far from edge
            {
            if( new_diff == INT_MIN )  // is not anymore fully in workarea
                {
                rect.setLeft( area.left());
                rect.setRight( area.right());
                }
            return;
            }
        if( isMovable())
            {
            if( old_diff < 0 ) // was in left third, keep distance from left edge
                rect.moveLeft( area.left() + ( -old_diff - 1 ));
            else // old_diff > 0 // was in right third, keep distance from right edge
                rect.moveRight( area.right() - ( old_diff - 1 ));
            }
        else if( isResizable())
            {
            if( old_diff < 0 )
                rect.setLeft( area.left() + ( -old_diff - 1 ) );
            else // old_diff > 0
                rect.setRight( area.right() - ( old_diff - 1 ));
            }
        if( rect.width() > area.width() && isResizable())
            rect.setWidth( area.width());
        if( isMovable())
            {
            if( rect.left() < area.left())
                rect.moveLeft( area.left());
            else if( rect.right() > area.right())
                rect.moveRight( area.right());
            }
        }
    if( rect.right() < area.left() + 5 || rect.left() > area.right() - 5 )
        { // not visible (almost) at all - try to make it at least partially visible
        if( isMovable())
            {
            if( rect.left() < area.left() + 5 )
                rect.moveRight( area.left() + 5 );
            if( rect.right() > area.right() - 5 )
                rect.moveLeft( area.right() - 5 );
            }
        }
    }

/*!
  Adjust the frame size \a frame according to he window's size hints.
 */
QSize Client::adjustedSize( const QSize& frame, Sizemode mode ) const
    {
    // first, get the window size for the given frame size s

    QSize wsize( frame.width() - ( border_left + border_right ),
             frame.height() - ( border_top + border_bottom ));
    if( wsize.isEmpty())
        wsize = QSize( 1, 1 );

    return sizeForClientSize( wsize, mode, false );
    }

// this helper returns proper size even if the window is shaded
// see also the comment in Client::setGeometry()
QSize Client::adjustedSize() const
    {
    return sizeForClientSize( clientSize());
    }

/*!
  Calculate the appropriate frame size for the given client size \a
  wsize.

  \a wsize is adapted according to the window's size hints (minimum,
  maximum and incremental size changes).

 */
QSize Client::sizeForClientSize( const QSize& wsize, Sizemode mode, bool noframe ) const
    {
    int w = wsize.width();
    int h = wsize.height();
    if( w < 1 || h < 1 )
        {
        kWarning() << "sizeForClientSize() with empty size!" ;
        kWarning() << kBacktrace() ;
        }
    if (w<1) w = 1;
    if (h<1) h = 1;

    // basesize, minsize, maxsize, paspect and resizeinc have all values defined,
    // even if they're not set in flags - see getWmNormalHints()
    QSize min_size = minSize();
    QSize max_size = maxSize();
    if( decoration != NULL )
        {
        QSize decominsize = decoration->minimumSize();
        QSize border_size( border_left + border_right, border_top + border_bottom );
        if( border_size.width() > decominsize.width()) // just in case
            decominsize.setWidth( border_size.width());
        if( border_size.height() > decominsize.height())
            decominsize.setHeight( border_size.height());
        if( decominsize.width() > min_size.width())
                min_size.setWidth( decominsize.width());
        if( decominsize.height() > min_size.height())
                min_size.setHeight( decominsize.height());
        }
    w = qMin( max_size.width(), w );
    h = qMin( max_size.height(), h );
    w = qMax( min_size.width(), w );
    h = qMax( min_size.height(), h );

    int w1 = w;
    int h1 = h;
    int width_inc = xSizeHint.width_inc;
    int height_inc = xSizeHint.height_inc;
    int basew_inc = xSizeHint.min_width; // see getWmNormalHints()
    int baseh_inc = xSizeHint.min_height;
    w = int(( w - basew_inc ) / width_inc ) * width_inc + basew_inc;
    h = int(( h - baseh_inc ) / height_inc ) * height_inc + baseh_inc;
// code for aspect ratios based on code from FVWM
    /*
     * The math looks like this:
     *
     * minAspectX    dwidth     maxAspectX
     * ---------- <= ------- <= ----------
     * minAspectY    dheight    maxAspectY
     *
     * If that is multiplied out, then the width and height are
     * invalid in the following situations:
     *
     * minAspectX * dheight > minAspectY * dwidth
     * maxAspectX * dheight < maxAspectY * dwidth
     *
     */
    if( xSizeHint.flags & PAspect )
        {
        double min_aspect_w = xSizeHint.min_aspect.x; // use doubles, because the values can be MAX_INT
        double min_aspect_h = xSizeHint.min_aspect.y; // and multiplying would go wrong otherwise
        double max_aspect_w = xSizeHint.max_aspect.x;
        double max_aspect_h = xSizeHint.max_aspect.y;
        // According to ICCCM 4.1.2.3 PMinSize should be a fallback for PBaseSize for size increments,
        // but not for aspect ratio. Since this code comes from FVWM, handles both at the same time,
        // and I have no idea how it works, let's hope nobody relies on that.
        w -= xSizeHint.base_width;
        h -= xSizeHint.base_height;
        int max_width = max_size.width() - xSizeHint.base_width;
        int min_width = min_size.width() - xSizeHint.base_width;
        int max_height = max_size.height() - xSizeHint.base_height;
        int min_height = min_size.height() - xSizeHint.base_height;
#define ASPECT_CHECK_GROW_W \
        if( min_aspect_w * h > min_aspect_h * w ) \
            { \
            int delta = int( min_aspect_w * h / min_aspect_h - w ) / width_inc * width_inc; \
            if( w + delta <= max_width ) \
                w += delta; \
            }
#define ASPECT_CHECK_SHRINK_H_GROW_W \
        if( min_aspect_w * h > min_aspect_h * w ) \
            { \
            int delta = int( h - w * min_aspect_h / min_aspect_w ) / height_inc * height_inc; \
            if( h - delta >= min_height ) \
                h -= delta; \
            else \
                { \
                int delta = int( min_aspect_w * h / min_aspect_h - w ) / width_inc * width_inc; \
                if( w + delta <= max_width ) \
                    w += delta; \
                } \
            }
#define ASPECT_CHECK_GROW_H \
        if( max_aspect_w * h < max_aspect_h * w ) \
            { \
            int delta = int( w * max_aspect_h / max_aspect_w - h ) / height_inc * height_inc; \
            if( h + delta <= max_height ) \
                h += delta; \
            }
#define ASPECT_CHECK_SHRINK_W_GROW_H \
        if( max_aspect_w * h < max_aspect_h * w ) \
            { \
            int delta = int( w - max_aspect_w * h / max_aspect_h ) / width_inc * width_inc; \
            if( w - delta >= min_width ) \
                w -= delta; \
            else \
                { \
                int delta = int( w * max_aspect_h / max_aspect_w - h ) / height_inc * height_inc; \
                if( h + delta <= max_height ) \
                    h += delta; \
                } \
            }
        switch( mode )
            {
            case SizemodeAny:
#if 0 // make SizemodeAny equal to SizemodeFixedW - prefer keeping fixed width,
      // so that changing aspect ratio to a different value and back keeps the same size (#87298)
                {
                ASPECT_CHECK_SHRINK_H_GROW_W
                ASPECT_CHECK_SHRINK_W_GROW_H
                ASPECT_CHECK_GROW_H
                ASPECT_CHECK_GROW_W
                break;
                }
#endif
            case SizemodeFixedW:
                {
                // the checks are order so that attempts to modify height are first
                ASPECT_CHECK_GROW_H
                ASPECT_CHECK_SHRINK_H_GROW_W
                ASPECT_CHECK_SHRINK_W_GROW_H
                ASPECT_CHECK_GROW_W
                break;
                }
            case SizemodeFixedH:
                {
                ASPECT_CHECK_GROW_W
                ASPECT_CHECK_SHRINK_W_GROW_H
                ASPECT_CHECK_SHRINK_H_GROW_W
                ASPECT_CHECK_GROW_H
                break;
                }
            case SizemodeMax:
                {
                // first checks that try to shrink
                ASPECT_CHECK_SHRINK_H_GROW_W
                ASPECT_CHECK_SHRINK_W_GROW_H
                ASPECT_CHECK_GROW_W
                ASPECT_CHECK_GROW_H
                break;
                }
            }
#undef ASPECT_CHECK_SHRINK_H_GROW_W
#undef ASPECT_CHECK_SHRINK_W_GROW_H
#undef ASPECT_CHECK_GROW_W
#undef ASPECT_CHECK_GROW_H
        w += xSizeHint.base_width;
        h += xSizeHint.base_height;
        }
    if( !rules()->checkStrictGeometry( false ))
        {
        // disobey increments and aspect when maximized
        if( maximizeMode() & MaximizeHorizontal )
            w = w1;
        if( maximizeMode() & MaximizeVertical )
            h = h1;
        }

    if( !noframe )
        {
        w += border_left + border_right;
        h += border_top + border_bottom;
        }
    return rules()->checkSize( QSize( w, h ));
    }

/*!
  Gets the client's normal WM hints and reconfigures itself respectively.
 */
void Client::getWmNormalHints()
    {
    long msize;
    if (XGetWMNormalHints(display(), window(), &xSizeHint, &msize) == 0 )
        xSizeHint.flags = 0;
    // set defined values for the fields, even if they're not in flags

    if( ! ( xSizeHint.flags & PMinSize ))
        xSizeHint.min_width = xSizeHint.min_height = 0;
    if( xSizeHint.flags & PBaseSize )
        {
        // PBaseSize is a fallback for PMinSize according to ICCCM 4.1.2.3
        // The other way around PMinSize is not a complete fallback for PBaseSize,
        // so that's not handled here.
        if( ! ( xSizeHint.flags & PMinSize ))
            {
            xSizeHint.min_width = xSizeHint.base_width;
            xSizeHint.min_height = xSizeHint.base_height;
            }
        }
    else
        xSizeHint.base_width = xSizeHint.base_height = 0;
    if( ! ( xSizeHint.flags & PMaxSize ))
        xSizeHint.max_width = xSizeHint.max_height = INT_MAX;
    else
        {
        xSizeHint.max_width = qMax( xSizeHint.max_width, 1 );
        xSizeHint.max_height = qMax( xSizeHint.max_height, 1 );
        }
    if( xSizeHint.flags & PResizeInc )
        {
        xSizeHint.width_inc = qMax( xSizeHint.width_inc, 1 );
        xSizeHint.height_inc = qMax( xSizeHint.height_inc, 1 );
        }
    else
        {
        xSizeHint.width_inc = 1;
        xSizeHint.height_inc = 1;
        }
    if( xSizeHint.flags & PAspect )
        { // no dividing by zero
        xSizeHint.min_aspect.y = qMax( xSizeHint.min_aspect.y, 1 );
        xSizeHint.max_aspect.y = qMax( xSizeHint.max_aspect.y, 1 );
        }
    else
        {
        xSizeHint.min_aspect.x = 1;
        xSizeHint.min_aspect.y = INT_MAX;
        xSizeHint.max_aspect.x = INT_MAX;
        xSizeHint.max_aspect.y = 1;
        }
    if( ! ( xSizeHint.flags & PWinGravity ))
        xSizeHint.win_gravity = NorthWestGravity;
    if( isManaged())
        { // update to match restrictions
        QSize new_size = adjustedSize();
        if( new_size != size() && !isFullScreen())
            {
            QRect orig_geometry = geometry();
            resizeWithChecks( new_size );
            if( ( !isSpecialWindow() || isToolbar()) && !isFullScreen())
                {
                // try to keep the window in its xinerama screen if possible,
                // if that fails at least keep it visible somewhere
                QRect area = workspace()->clientArea( MovementArea, this );
                if( area.contains( orig_geometry ))
                    keepInArea( area );
                area = workspace()->clientArea( WorkArea, this );
                if( area.contains( orig_geometry ))
                    keepInArea( area );
                }
            }
        }
    updateAllowedActions(); // affects isResizeable()
    }

QSize Client::minSize() const
    {
    return rules()->checkMinSize( QSize( xSizeHint.min_width, xSizeHint.min_height ));
    }

QSize Client::maxSize() const
    {
    return rules()->checkMaxSize( QSize( xSizeHint.max_width, xSizeHint.max_height ));
    }

/*!
  Auxiliary function to inform the client about the current window
  configuration.

 */
void Client::sendSyntheticConfigureNotify()
    {
    XConfigureEvent c;
    c.type = ConfigureNotify;
    c.send_event = True;
    c.event = window();
    c.window = window();
    c.x = x() + clientPos().x();
    c.y = y() + clientPos().y();
    c.width = clientSize().width();
    c.height = clientSize().height();
    c.border_width = 0;
    c.above = None;
    c.override_redirect = 0;
    XSendEvent( display(), c.event, true, StructureNotifyMask, (XEvent*)&c );
    }

const QPoint Client::calculateGravitation( bool invert, int gravity ) const
    {
    int dx, dy;
    dx = dy = 0;

    if( gravity == 0 ) // default (nonsense) value for the argument
        gravity = xSizeHint.win_gravity;

// dx, dy specify how the client window moves to make space for the frame
    switch (gravity)
        {
        case NorthWestGravity: // move down right
        default:
            dx = border_left;
            dy = border_top;
            break;
        case NorthGravity: // move right
            dx = 0;
            dy = border_top;
            break;
        case NorthEastGravity: // move down left
            dx = -border_right;
            dy = border_top;
            break;
        case WestGravity: // move right
            dx = border_left;
            dy = 0;
            break;
        case CenterGravity:
            break; // will be handled specially
        case StaticGravity: // don't move
            dx = 0;
            dy = 0;
            break;
        case EastGravity: // move left
            dx = -border_right;
            dy = 0;
            break;
        case SouthWestGravity: // move up right
            dx = border_left ;
            dy = -border_bottom;
            break;
        case SouthGravity: // move up
            dx = 0;
            dy = -border_bottom;
            break;
        case SouthEastGravity: // move up left
            dx = -border_right;
            dy = -border_bottom;
            break;
        }
    if( gravity != CenterGravity )
        { // translate from client movement to frame movement
        dx -= border_left;
        dy -= border_top;
        }
    else
        { // center of the frame will be at the same position client center without frame would be
        dx = - ( border_left + border_right ) / 2;
        dy = - ( border_top + border_bottom ) / 2;
        }
    if( !invert )
        return QPoint( x() + dx, y() + dy );
    else
        return QPoint( x() - dx, y() - dy );
    }

void Client::configureRequest( int value_mask, int rx, int ry, int rw, int rh, int gravity, bool from_tool )
    {
    if( gravity == 0 ) // default (nonsense) value for the argument
        gravity = xSizeHint.win_gravity;
    if( value_mask & ( CWX | CWY ))
        {
        QPoint new_pos = calculateGravitation( true, gravity ); // undo gravitation
        if ( value_mask & CWX )
            new_pos.setX( rx );
        if ( value_mask & CWY )
            new_pos.setY( ry );

        // clever(?) workaround for applications like xv that want to set
        // the location to the current location but miscalculate the
        // frame size due to kwin being a double-reparenting window
        // manager
        if ( new_pos.x() == x() + clientPos().x() && new_pos.y() == y() + clientPos().y()
            && gravity == NorthWestGravity && !from_tool )
            {
            new_pos.setX( x());
            new_pos.setY( y());
            }

        int nw = clientSize().width();
        int nh = clientSize().height();
        if ( value_mask & CWWidth )
            nw = rw;
        if ( value_mask & CWHeight )
            nh = rh;
        QSize ns = sizeForClientSize( QSize( nw, nh ) ); // enforces size if needed
        new_pos = rules()->checkPosition( new_pos );

        // TODO what to do with maximized windows?
        if ( maximizeMode() != MaximizeFull
            || ns != size())
            {
            QRect orig_geometry = geometry();
            GeometryUpdatesBlocker blocker( this );
            move( new_pos );
            plainResize( ns );
            setGeometry( QRect( calculateGravitation( false, gravity ), size()));
            updateFullScreenHack( QRect( new_pos, QSize( nw, nh )));
            QRect area = workspace()->clientArea( WorkArea, this );
            if( !from_tool && ( !isSpecialWindow() || isToolbar()) && !isFullScreen()
                && area.contains( orig_geometry ))
                keepInArea( area );

            // this is part of the kicker-xinerama-hack... it should be
            // safe to remove when kicker gets proper ExtendedStrut support;
            // see Workspace::updateClientArea() and
            // Client::adjustedClientArea()
            if (hasStrut ())
                workspace() -> updateClientArea ();
            }
        }

    if ( value_mask & (CWWidth | CWHeight )
        && ! ( value_mask & ( CWX | CWY )) )  // pure resize
        {
        int nw = clientSize().width();
        int nh = clientSize().height();
        if ( value_mask & CWWidth )
            nw = rw;
        if ( value_mask & CWHeight )
            nh = rh;
        QSize ns = sizeForClientSize( QSize( nw, nh ) );

        if( ns != size())  // don't restore if some app sets its own size again
            {
            QRect orig_geometry = geometry();
            GeometryUpdatesBlocker blocker( this );
            int save_gravity = xSizeHint.win_gravity;
            xSizeHint.win_gravity = gravity;
            resizeWithChecks( ns );
            xSizeHint.win_gravity = save_gravity;
            updateFullScreenHack( QRect( calculateGravitation( true, xSizeHint.win_gravity ), QSize( nw, nh )));
            if( !from_tool && ( !isSpecialWindow() || isToolbar()) && !isFullScreen())
                {
                // try to keep the window in its xinerama screen if possible,
                // if that fails at least keep it visible somewhere
                QRect area = workspace()->clientArea( MovementArea, this );
                if( area.contains( orig_geometry ))
                    keepInArea( area );
                area = workspace()->clientArea( WorkArea, this );
                if( area.contains( orig_geometry ))
                    keepInArea( area );
                }
            }
        }
    // No need to send synthetic configure notify event here, either it's sent together
    // with geometry change, or there's no need to send it.
    // Handling of the real ConfigureRequest event forces sending it, as there it's necessary.
    }

void Client::resizeWithChecks( int w, int h, ForceGeometry_t force )
    {
    if( shade_geometry_change )
        assert( false );
    else if( isShade())
        {
        if( h == border_top + border_bottom )
            {
            kWarning() << "Shaded geometry passed for size:" ;
            kWarning() << kBacktrace() ;
            }
        }
    int newx = x();
    int newy = y();
    QRect area = workspace()->clientArea( WorkArea, this );
    // don't allow growing larger than workarea
    if( w > area.width())
        w = area.width();
    if( h > area.height())
        h = area.height();
    QSize tmp = adjustedSize( QSize( w, h )); // checks size constraints, including min/max size
    w = tmp.width();
    h = tmp.height();
    switch( xSizeHint.win_gravity )
        {
        case NorthWestGravity: // top left corner doesn't move
        default:
            break;
        case NorthGravity: // middle of top border doesn't move
            newx = ( newx + width() / 2 ) - ( w / 2 );
            break;
        case NorthEastGravity: // top right corner doesn't move
            newx = newx + width() - w;
            break;
        case WestGravity: // middle of left border doesn't move
            newy = ( newy + height() / 2 ) - ( h / 2 );
            break;
        case CenterGravity: // middle point doesn't move
            newx = ( newx + width() / 2 ) - ( w / 2 );
            newy = ( newy + height() / 2 ) - ( h / 2 );
            break;
        case StaticGravity: // top left corner of _client_ window doesn't move
            // since decoration doesn't change, equal to NorthWestGravity
            break;
        case EastGravity: // // middle of right border doesn't move
            newx = newx + width() - w;
            newy = ( newy + height() / 2 ) - ( h / 2 );
            break;
        case SouthWestGravity: // bottom left corner doesn't move
            newy = newy + height() - h;
            break;
        case SouthGravity: // middle of bottom border doesn't move
            newx = ( newx + width() / 2 ) - ( w / 2 );
            newy = newy + height() - h;
            break;
        case SouthEastGravity: // bottom right corner doesn't move
            newx = newx + width() - w;
            newy = newy + height() - h;
            break;
        }
    // if it would be moved outside of workarea, keep it inside,
    // see also Client::computeWorkareaDiff()
    if( workarea_diff_x != INT_MIN && w <= area.width()) // was inside and can still fit
        {
        if( newx < area.left())
            newx = area.left();
        if( newx + w > area.right() + 1 )
            newx = area.right() + 1 - w;
        assert( newx >= area.left() && newx + w <= area.right() + 1 ); // width was checked above
        }
    if( workarea_diff_y != INT_MIN && h <= area.height()) // was inside and can still fit
        {
        if( newy < area.top())
            newy = area.top();
        if( newy + h > area.bottom() + 1 )
            newy = area.bottom() + 1 - h;
        assert( newy >= area.top() && newy + h <= area.bottom() + 1 ); // height was checked above
        }
    setGeometry( newx, newy, w, h, force );
    }

// _NET_MOVERESIZE_WINDOW
void Client::NETMoveResizeWindow( int flags, int x, int y, int width, int height )
    {
    int gravity = flags & 0xff;
    int value_mask = 0;
    if( flags & ( 1 << 8 ))
        value_mask |= CWX;
    if( flags & ( 1 << 9 ))
        value_mask |= CWY;
    if( flags & ( 1 << 10 ))
        value_mask |= CWWidth;
    if( flags & ( 1 << 11 ))
        value_mask |= CWHeight;
    configureRequest( value_mask, x, y, width, height, gravity, true );
    }

/*!
  Returns whether the window is moveable or has a fixed
  position.
 */
bool Client::isMovable() const
    {
    if( !motif_may_move || isFullScreen())
        return false;
    if( isSpecialWindow() && !isSplash() && !isToolbar()) // allow moving of splashscreens :)
        return false;
    if( maximizeMode() == MaximizeFull && !options->moveResizeMaximizedWindows() )
        return false;
    if( rules()->checkPosition( invalidPoint ) != invalidPoint ) // forced position
        return false;
    return true;
    }

/*!
  Returns whether the window is resizable or has a fixed size.
 */
bool Client::isResizable() const
    {
    if( !motif_may_resize || isFullScreen())
        return false;
    if( isSpecialWindow() || isSplash() || isToolbar())
        return false;
    if( maximizeMode() == MaximizeFull && !options->moveResizeMaximizedWindows() )
        return false;
    if( rules()->checkSize( QSize()).isValid()) // forced size
        return false;

    QSize min = minSize();
    QSize max = maxSize();
    return min.width() < max.width() || min.height() < max.height();
    }

/*
  Returns whether the window is maximizable or not
 */
bool Client::isMaximizable() const
    {
        { // isMovable() and isResizable() may be false for maximized windows
          // with moving/resizing maximized windows disabled
        TemporaryAssign< MaximizeMode > tmp( max_mode, MaximizeRestore );
        if( !isMovable() || !isResizable() || isToolbar()) // SELI isToolbar() ?
            return false;
        }
    if ( maximizeMode() != MaximizeRestore )
        return true;
    QSize max = maxSize();
#if 0
    if( max.width() < 32767 || max.height() < 32767 ) // sizes are 16bit with X
        return false;
#else
    // apparently there are enough apps which specify some arbitrary value
    // for their maximum size just for the fun of it
    QSize areasize = workspace()->clientArea( MaximizeArea, this ).size();
    if( max.width() < areasize.width() || max.height() < areasize.height())
        return false;
#endif
    return true;
    }


/*!
  Reimplemented to inform the client about the new window position.
 */
void Client::setGeometry( int x, int y, int w, int h, ForceGeometry_t force )
    {
    // this code is also duplicated in Client::plainResize()
    // Ok, the shading geometry stuff. Generally, code doesn't care about shaded geometry,
    // simply because there are too many places dealing with geometry. Those places
    // ignore shaded state and use normal geometry, which they usually should get
    // from adjustedSize(). Such geometry comes here, and if the window is shaded,
    // the geometry is used only for client_size, since that one is not used when
    // shading. Then the frame geometry is adjusted for the shaded geometry.
    // This gets more complicated in the case the code does only something like
    // setGeometry( geometry()) - geometry() will return the shaded frame geometry.
    // Such code is wrong and should be changed to handle the case when the window is shaded,
    // for example using Client::clientSize().
    if( shade_geometry_change )
        ; // nothing
    else if( isShade())
        {
        if( h == border_top + border_bottom )
            {
            kDebug() << "Shaded geometry passed for size:";
            kDebug() << kBacktrace();
            }
        else
            {
            client_size = QSize( w - border_left - border_right, h - border_top - border_bottom );
            h = border_top + border_bottom;
            }
        }
    else
        {
        client_size = QSize( w - border_left - border_right, h - border_top - border_bottom );
        }
    QRect g( x, y, w, h );
    if( block_geometry_updates == 0 && g != rules()->checkGeometry( g ))
        {
        kDebug() << "forced geometry fail:" << g << ":" << rules()->checkGeometry( g );
        kDebug() << kBacktrace();
        }
    if( force == NormalGeometrySet && geom == g && pending_geometry_update == PendingGeometryNone )
        return;
    geom = g;
    updateWorkareaDiffs();
    if( block_geometry_updates != 0 )
        {
        if( pending_geometry_update == PendingGeometryForced )
            {} // maximum, nothing needed
        else if( force == ForceGeometrySet )
            pending_geometry_update = PendingGeometryForced;
        else
            pending_geometry_update = PendingGeometryNormal;
        return;
        }
    bool resized = ( geom_before_block.size() != geom.size() || pending_geometry_update == PendingGeometryForced );
    if( resized )
        {
        resizeDecoration( QSize( w, h ));
        XMoveResizeWindow( display(), frameId(), x, y, w, h );
        if( !isShade())
            {
            QSize cs = clientSize();
            XMoveResizeWindow( display(), wrapperId(), clientPos().x(), clientPos().y(),
                cs.width(), cs.height());
            XMoveResizeWindow( display(), window(), 0, 0, cs.width(), cs.height());
            }
        updateShape();
        }
    else
        XMoveWindow( display(), frameId(), x, y );
    // SELI TODO won't this be too expensive?
    sendSyntheticConfigureNotify();
    updateWindowRules();
    checkMaximizeGeometry();
    workspace()->checkActiveScreen( this );
    if( resized )
        {
        discardWindowPixmap();
        if( scene != NULL )
            scene->windowGeometryShapeChanged( this );
        if( effects != NULL )
            static_cast<EffectsHandlerImpl*>(effects)->windowGeometryShapeChanged( effectWindow(), geom_before_block );
        }
    addWorkspaceRepaint( geom_before_block );
    addWorkspaceRepaint( geom );
    geom_before_block = geom;
    }

void Client::plainResize( int w, int h, ForceGeometry_t force )
    {
    // this code is also duplicated in Client::setGeometry(), and it's also commented there
    if( shade_geometry_change )
        ; // nothing
    else if( isShade())
        {
        if( h == border_top + border_bottom )
            {
            kDebug() << "Shaded geometry passed for size:";
            kDebug() << kBacktrace();
            }
        else
            {
            client_size = QSize( w - border_left - border_right, h - border_top - border_bottom );
            h = border_top + border_bottom;
            }
        }
    else
        {
        client_size = QSize( w - border_left - border_right, h - border_top - border_bottom );
        }
    QSize s( w, h );
    if( block_geometry_updates == 0 && s != rules()->checkSize( s ))
        {
        kDebug() << "forced size fail:" << s << ":" << rules()->checkSize( s );
        kDebug() << kBacktrace();
        }
    // resuming geometry updates is handled only in setGeometry()
    assert( pending_geometry_update == PendingGeometryNone || block_geometry_updates > 0 );
    if( force == NormalGeometrySet && geom.size() == s )
        return;
    geom.setSize( s );
    updateWorkareaDiffs();
    if( block_geometry_updates != 0 )
        {
        if( pending_geometry_update == PendingGeometryForced )
            {} // maximum, nothing needed
        else if( force == ForceGeometrySet )
            pending_geometry_update = PendingGeometryForced;
        else
            pending_geometry_update = PendingGeometryNormal;
        return;
        }
    resizeDecoration( s );
    XResizeWindow( display(), frameId(), w, h );
//     resizeDecoration( s );
    if( !isShade())
        {
        QSize cs = clientSize();
        XMoveResizeWindow( display(), wrapperId(), clientPos().x(), clientPos().y(),
            cs.width(), cs.height());
        XMoveResizeWindow( display(), window(), 0, 0, cs.width(), cs.height());
        }
    updateShape();
    sendSyntheticConfigureNotify();
    updateWindowRules();
    checkMaximizeGeometry();
    workspace()->checkActiveScreen( this );
    discardWindowPixmap();
    if( scene != NULL )
        scene->windowGeometryShapeChanged( this );
    if( effects != NULL )
        static_cast<EffectsHandlerImpl*>(effects)->windowGeometryShapeChanged( effectWindow(), geom_before_block );
    addWorkspaceRepaint( geom_before_block );
    addWorkspaceRepaint( geom );
    geom_before_block = geom;
    }

/*!
  Reimplemented to inform the client about the new window position.
 */
void Client::move( int x, int y, ForceGeometry_t force )
    {
    // resuming geometry updates is handled only in setGeometry()
    assert( pending_geometry_update == PendingGeometryNone || block_geometry_updates > 0 );
    QPoint p( x, y );
    if( block_geometry_updates == 0 && p != rules()->checkPosition( p ))
        {
        kDebug() << "forced position fail:" << p << ":" << rules()->checkPosition( p );
        kDebug() << kBacktrace();
        }
    if( force == NormalGeometrySet && geom.topLeft() == p )
        return;
    geom.moveTopLeft( p );
    updateWorkareaDiffs();
    if( block_geometry_updates != 0 )
        {
        if( pending_geometry_update == PendingGeometryForced )
            {} // maximum, nothing needed
        else if( force == ForceGeometrySet )
            pending_geometry_update = PendingGeometryForced;
        else
            pending_geometry_update = PendingGeometryNormal;
        return;
        }
    XMoveWindow( display(), frameId(), x, y );
    sendSyntheticConfigureNotify();
    updateWindowRules();
    checkMaximizeGeometry();
    workspace()->checkActiveScreen( this );
    // client itself is not damaged
    addWorkspaceRepaint( geom_before_block );
    addWorkspaceRepaint( geom ); // trigger repaint of window's new location
    geom_before_block = geom;
    }

void Client::blockGeometryUpdates( bool block )
    {
    if( block )
        {
        if( block_geometry_updates == 0 )
            pending_geometry_update = PendingGeometryNone;
        ++block_geometry_updates;
        }
    else
        {
        if( --block_geometry_updates == 0 )
            {
            if( pending_geometry_update != PendingGeometryNone )
                {
                if( isShade())
                    setGeometry( QRect( pos(), adjustedSize()), NormalGeometrySet );
                else
                    setGeometry( geometry(), NormalGeometrySet );
                pending_geometry_update = PendingGeometryNone;
                }
            }
        }
    }

void Client::maximize( MaximizeMode m )
    {
    setMaximize( m & MaximizeVertical, m & MaximizeHorizontal );
    }

/*!
  Sets the maximization according to \a vertically and \a horizontally
 */
void Client::setMaximize( bool vertically, bool horizontally )
    {   // changeMaximize() flips the state, so change from set->flip
    changeMaximize(
        max_mode & MaximizeVertical ? !vertically : vertically,
        max_mode & MaximizeHorizontal ? !horizontally : horizontally,
        false );
    }

void Client::changeMaximize( bool vertical, bool horizontal, bool adjust )
    {
    if( !isMaximizable())
        return;

    MaximizeMode old_mode = max_mode;
    // 'adjust == true' means to update the size only, e.g. after changing workspace size
    if( !adjust )
        {
        if( vertical )
            max_mode = MaximizeMode( max_mode ^ MaximizeVertical );
        if( horizontal )
            max_mode = MaximizeMode( max_mode ^ MaximizeHorizontal );
        }
        
    max_mode = rules()->checkMaximize( max_mode );
    if( !adjust && max_mode == old_mode )
        return;

    GeometryUpdatesBlocker blocker( this );

    // maximing one way and unmaximizing the other way shouldn't happen,
    // so restore first and then maximize the other way
    if( ( old_mode == MaximizeVertical && max_mode == MaximizeHorizontal )
        || ( old_mode == MaximizeHorizontal && max_mode == MaximizeVertical ))
        {
        changeMaximize( false, false, false ); // restore
        }
        

    QRect clientArea = workspace()->clientArea( MaximizeArea, this );

    // save sizes for restoring, if maximalizing
    if( !adjust && !( y() == clientArea.top() && height() == clientArea.height()))
        {
        geom_restore.setTop( y());
        geom_restore.setHeight( height());
        }
    if( !adjust && !( x() == clientArea.left() && width() == clientArea.width()))
        {
        geom_restore.setLeft( x());
        geom_restore.setWidth( width());
        }

    if( !adjust )
        {
        if(( vertical && !(old_mode & MaximizeVertical ))
            || ( horizontal && !( old_mode & MaximizeHorizontal )))
            Notify::raise( Notify::Maximize );
        else
            Notify::raise( Notify::UnMaximize );
        }

    ForceGeometry_t geom_mode = NormalGeometrySet;
    if( decoration != NULL ) // decorations may turn off some borders when maximized
        {
        if( checkBorderSizes( false )) // only query, don't resize
            geom_mode = ForceGeometrySet;
        }

    // restore partial maximizations
    if ( old_mode==MaximizeFull && max_mode==MaximizeRestore )
        {
        if ( maximizeModeRestore()==MaximizeVertical )
	    {
	    max_mode = MaximizeVertical;
	    maxmode_restore = MaximizeRestore;
	    }
	if ( maximizeModeRestore()==MaximizeHorizontal )
	    {
	    max_mode = MaximizeHorizontal;
	    maxmode_restore = MaximizeRestore;
	    }	
	}
    
    switch (max_mode)
        {

        case MaximizeVertical:
            {
            if( old_mode & MaximizeHorizontal ) // actually restoring from MaximizeFull
                {
                if( geom_restore.width() == 0 )
                    { // needs placement
                    plainResize( adjustedSize(QSize(width(), clientArea.height()), SizemodeFixedH ), geom_mode );
                    workspace()->placeSmart( this, clientArea );
                    }
                else
                    setGeometry( QRect(QPoint( geom_restore.x(), clientArea.top()),
                              adjustedSize(QSize( geom_restore.width(), clientArea.height()), SizemodeFixedH )), geom_mode );
                }
            else
                setGeometry( QRect(QPoint(x(), clientArea.top()),
                              adjustedSize(QSize(width(), clientArea.height()), SizemodeFixedH )), geom_mode );
            info->setState( NET::MaxVert, NET::Max );
            break;
            }

        case MaximizeHorizontal:
            {
            if( old_mode & MaximizeVertical ) // actually restoring from MaximizeFull
                {
                if( geom_restore.height() == 0 )
                    { // needs placement
                    plainResize( adjustedSize(QSize(clientArea.width(), height()), SizemodeFixedW ), geom_mode );
                    workspace()->placeSmart( this, clientArea );
                    }
                else
                    setGeometry( QRect( QPoint(clientArea.left(), geom_restore.y()),
                              adjustedSize(QSize(clientArea.width(), geom_restore.height()), SizemodeFixedW )), geom_mode );
                }
            else
                setGeometry( QRect( QPoint(clientArea.left(), y()),
                              adjustedSize(QSize(clientArea.width(), height()), SizemodeFixedW )), geom_mode );
            info->setState( NET::MaxHoriz, NET::Max );
            break;
            }

        case MaximizeRestore:
            {
            QRect restore = geometry();
	// when only partially maximized, geom_restore may not have the other dimension remembered
            if( old_mode & MaximizeVertical )
                {
                restore.setTop( geom_restore.top());
                restore.setBottom( geom_restore.bottom());
                }
            if( old_mode & MaximizeHorizontal )
                {
                restore.setLeft( geom_restore.left());
                restore.setRight( geom_restore.right());
                }
            if( !restore.isValid())
                {
                QSize s = QSize( clientArea.width()*2/3, clientArea.height()*2/3 );
                if( geom_restore.width() > 0 )
                    s.setWidth( geom_restore.width());
                if( geom_restore.height() > 0 )
                    s.setHeight( geom_restore.height());
                plainResize( adjustedSize( s ));
                workspace()->placeSmart( this, clientArea );
                restore = geometry();
                if( geom_restore.width() > 0 )
                    restore.moveLeft( geom_restore.x());
                if( geom_restore.height() > 0 )
                    restore.moveTop( geom_restore.y());
                }
            setGeometry( restore, geom_mode );
            info->setState( 0, NET::Max );
            break;
            }

        case MaximizeFull:
            {
            if( !adjust )
                {
                if( old_mode & MaximizeVertical )
                    maxmode_restore = MaximizeVertical;
                if( old_mode & MaximizeHorizontal )
    	            maxmode_restore = MaximizeHorizontal;
                }
            QSize adjSize = adjustedSize(clientArea.size(), SizemodeMax );
            QRect r = QRect(clientArea.topLeft(), adjSize);
            setGeometry( r, geom_mode );
            info->setState( NET::Max, NET::Max );
            break;
            }
        default:
            break;
        }

    updateAllowedActions();
    if( decoration != NULL )
        decoration->maximizeChange();
    updateWindowRules();
    }

void Client::resetMaximize()
    {
    if( max_mode == MaximizeRestore )
        return;
    max_mode = MaximizeRestore;
    Notify::raise( Notify::UnMaximize );
    info->setState( 0, NET::Max );
    updateAllowedActions();
    if( decoration != NULL )
        decoration->borders( border_left, border_right, border_top, border_bottom );
    if( isShade())
        setGeometry( QRect( pos(), sizeForClientSize( clientSize())), ForceGeometrySet );
    else
        setGeometry( geometry(), ForceGeometrySet );
    if( decoration != NULL )
        decoration->maximizeChange();
    }

void Client::checkMaximizeGeometry()
    {
    // when adding new bail-out conditions here, checkMaximizeGeometry() needs to be called
    // when after the condition is no longer true
    if( isShade())
        return;
    if( isMove() || isResize()) // this is because of the option to disallow moving/resizing of max-ed windows
        return;
    // Just in case.
    static int recursion_protection = 0;
    if( recursion_protection > 3 )
        {
        kWarning( 1212 ) << "Check maximize overflow - you loose!" ;
        kWarning( 1212 ) << kBacktrace() ;
        return;
        }
    ++recursion_protection;
    QRect max_area = workspace()->clientArea( MaximizeArea, this );
    if( geometry() == max_area )
        {
        if( max_mode != MaximizeFull )
            maximize( MaximizeFull );
        }
    else if( x() == max_area.left() && width() == max_area.width())
        {
        if( max_mode != MaximizeHorizontal )
            maximize( MaximizeHorizontal );
        }
    else if( y() == max_area.top() && height() == max_area.height())
        {
        if( max_mode != MaximizeVertical )
            maximize( MaximizeVertical );
        }
    else if( max_mode != MaximizeRestore )
        {
        resetMaximize(); // not maximize( MaximizeRestore ), that'd change geometry - this is called from setGeometry()
        }
    --recursion_protection;
    }

bool Client::isFullScreenable( bool fullscreen_hack ) const
    {
    if( !rules()->checkFullScreen( true ))
        return false;
    if( fullscreen_hack )
        return isNormalWindow();
    if( rules()->checkStrictGeometry( false ))
        {
        // the app wouldn't fit exactly fullscreen geometry due its strict geometry requirements
        QRect fsarea = workspace()->clientArea( FullScreenArea, this );
        if( sizeForClientSize( fsarea.size(), SizemodeAny, true ) != fsarea.size())
            return false;
        }
     // don't check size constrains - some apps request fullscreen despite requesting fixed size
    return !isSpecialWindow(); // also better disallow only weird types to go fullscreen
    }

bool Client::userCanSetFullScreen() const
    {
    if( fullscreen_mode == FullScreenHack )
        return false;
    if( !isFullScreenable( false ))
        return false;
    // isMaximizable() returns false if fullscreen
    TemporaryAssign< FullScreenMode > tmp( fullscreen_mode, FullScreenNone );
    return isNormalWindow() && isMaximizable();
    }

void Client::setFullScreen( bool set, bool user )
    {
    if( !isFullScreen() && !set )
        return;
    if( fullscreen_mode == FullScreenHack )
        return;
    if( user && !userCanSetFullScreen())
        return;
    set = rules()->checkFullScreen( set );
    setShade( ShadeNone );
    bool was_fs = isFullScreen();
    if( !was_fs )
        geom_fs_restore = geometry();
    fullscreen_mode = set ? FullScreenNormal : FullScreenNone;
    if( was_fs == isFullScreen())
        return;
    StackingUpdatesBlocker blocker1( workspace());
    GeometryUpdatesBlocker blocker2( this );
    workspace()->updateClientLayer( this ); // active fullscreens get different layer
    info->setState( isFullScreen() ? NET::FullScreen : 0, NET::FullScreen );
    updateDecoration( false, false );
    if( isFullScreen())
        setGeometry( workspace()->clientArea( FullScreenArea, this ));
    else
        {
        if( !geom_fs_restore.isNull())
            setGeometry( QRect( geom_fs_restore.topLeft(), adjustedSize( geom_fs_restore.size())));
        // TODO isShaded() ?
        else
            { // does this ever happen?
            setGeometry( workspace()->clientArea( MaximizeArea, this ));
            }
        }
    updateWindowRules();
    }

int Client::checkFullScreenHack( const QRect& geom ) const
    {
    // if it's noborder window, and has size of one screen or the whole desktop geometry, it's fullscreen hack
    if( noBorder() && !isUserNoBorder() && isFullScreenable( true ))
        {
        if( geom.size() == workspace()->clientArea( FullArea, geom.center(), desktop()).size())
            return 2; // full area fullscreen hack
        if( geom.size() == workspace()->clientArea( ScreenArea, geom.center(), desktop()).size())
            return 1; // xinerama-aware fullscreen hack
        }
    return 0;
    }

void Client::updateFullScreenHack( const QRect& geom )
    {
    int type = checkFullScreenHack( geom );
    if( fullscreen_mode == FullScreenNone && type != 0 )
        {
        fullscreen_mode = FullScreenHack;
        updateDecoration( false, false );
        QRect geom;
        if( rules()->checkStrictGeometry( false ))
            {
            geom = type == 2 // 1 - it's xinerama-aware fullscreen hack, 2 - it's full area
                ? workspace()->clientArea( FullArea, geom.center(), desktop())
                : workspace()->clientArea( ScreenArea, geom.center(), desktop());
            }
        else
            geom = workspace()->clientArea( FullScreenArea, geom.center(), desktop());
        setGeometry( geom );
        }
    else if( fullscreen_mode == FullScreenHack && type == 0 )
        {
        fullscreen_mode = FullScreenNone;
        updateDecoration( false, false );
        // whoever called this must setup correct geometry
        }
    StackingUpdatesBlocker blocker( workspace());
    workspace()->updateClientLayer( this ); // active fullscreens get different layer
    }

static QRect*       visible_bound  = 0;
static GeometryTip* geometryTip    = 0;

void Client::drawbound( const QRect& geom )
    {
    assert( visible_bound == NULL );
    visible_bound = new QRect( geom );
    doDrawbound( *visible_bound, false );
    }

void Client::clearbound()
    {
    if( visible_bound == NULL )
        return;
    doDrawbound( *visible_bound, true );
    delete visible_bound;
    visible_bound = 0;
    }

void Client::doDrawbound( const QRect& geom, bool clear )
    {
    if( decoration != NULL && decoration->drawbound( geom, clear ))
        return; // done by decoration
    XGCValues xgc;
    xgc.function = GXxor;
    xgc.foreground = WhitePixel( display(), DefaultScreen( display()));
    xgc.line_width = 5;
    xgc.subwindow_mode = IncludeInferiors;
    GC gc = XCreateGC( display(), DefaultRootWindow( display()),
        GCFunction | GCForeground | GCLineWidth | GCSubwindowMode, &xgc );
    // the line is 5 pixel thick, so compensate for the extra two pixels
    // on outside (#88657)
    QRect g = geom;
    if( g.width() > 5 )
        {
        g.setLeft( g.left() + 2 );
        g.setRight( g.right() - 2 );
        }
    if( g.height() > 5 )
        {
        g.setTop( g.top() + 2 );
        g.setBottom( g.bottom() - 2 );
        }
    XDrawRectangle( display(), DefaultRootWindow( display()), gc, g.x(), g.y(), g.width(), g.height());
    XFreeGC( display(), gc );
    }

void Client::positionGeometryTip()
    {
    assert( isMove() || isResize());
    // Position and Size display
    if (options->showGeometryTip())
        {
        if( !geometryTip )
            { // save under is not necessary with opaque, and seem to make things slower
            bool save_under = ( isMove() && rules()->checkMoveResizeMode( options->moveMode ) != Options::Opaque )
                        || ( isResize() && rules()->checkMoveResizeMode( options->resizeMode ) != Options::Opaque );
            geometryTip = new GeometryTip( &xSizeHint, save_under );
            }
        QRect wgeom( moveResizeGeom ); // position of the frame, size of the window itself
        wgeom.setWidth( wgeom.width() - ( width() - clientSize().width()));
        wgeom.setHeight( wgeom.height() - ( height() - clientSize().height()));
        if( isShade())
            wgeom.setHeight( 0 );
        geometryTip->setGeometry( wgeom );
        if( !geometryTip->isVisible())
            {
            geometryTip->show();
            geometryTip->raise();
            }
        }
    }

class EatAllPaintEvents
    : public QObject
    {
    protected:
        virtual bool eventFilter( QObject* o, QEvent* e )
            { return e->type() == QEvent::Paint && o != geometryTip; }
    };

static EatAllPaintEvents* eater = 0;

bool Client::startMoveResize()
    {
    assert( !moveResizeMode );
    assert( QWidget::keyboardGrabber() == NULL );
    assert( QWidget::mouseGrabber() == NULL );
    stopDelayedMoveResize();
    if( QApplication::activePopupWidget() != NULL )
        return false; // popups have grab
    bool has_grab = false;
    // This reportedly improves smoothness of the moveresize operation,
    // something with Enter/LeaveNotify events, looks like XFree performance problem or something *shrug*
    // (http://lists.kde.org/?t=107302193400001&r=1&w=2)
    XSetWindowAttributes attrs;
    QRect r = workspace()->clientArea( FullArea, this );
    move_resize_grab_window = XCreateWindow( display(), rootWindow(), r.x(), r.y(),
        r.width(), r.height(), 0, CopyFromParent, InputOnly, CopyFromParent, 0, &attrs );
    XMapRaised( display(), move_resize_grab_window );
    if( XGrabPointer( display(), move_resize_grab_window, False,
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask,
        GrabModeAsync, GrabModeAsync, move_resize_grab_window, cursor.handle(), xTime() ) == Success )
        has_grab = true;
    if( grabXKeyboard( frameId()))
        has_grab = move_resize_has_keyboard_grab = true;
    if( !has_grab ) // at least one grab is necessary in order to be able to finish move/resize
        {
        XDestroyWindow( display(), move_resize_grab_window );
        move_resize_grab_window = None;
        return false;
        }
    if ( maximizeMode() != MaximizeRestore )
        resetMaximize();
    moveResizeMode = true;
    workspace()->setClientIsMoving(this);
    initialMoveResizeGeom = moveResizeGeom = geometry();
    checkUnrestrictedMoveResize();
    if ( ( isMove() && rules()->checkMoveResizeMode( options->moveMode ) != Options::Opaque )
      || ( isResize() && rules()->checkMoveResizeMode( options->resizeMode ) != Options::Opaque ) )
        {
        grabXServer();
        kapp->sendPostedEvents();
        // we have server grab -> nothing should cause paint events
        // unfortunately, that's not completely true, Qt may generate
        // paint events on some widgets due to FocusIn(?)
        // eat them, otherwise XOR painting will be broken (#58054)
        // paint events for the geometrytip need to be allowed, though
        eater = new EatAllPaintEvents;
// not needed anymore?        kapp->installEventFilter( eater );
        }
    Notify::raise( isResize() ? Notify::ResizeStart : Notify::MoveStart );
    if( effects )
        static_cast<EffectsHandlerImpl*>(effects)->windowUserMovedResized( effectWindow(), true, false );
    if( options->electricBorders() == Options::ElectricMoveOnly )
        workspace()->reserveElectricBorderSwitching( true );
    return true;
    }

void Client::finishMoveResize( bool cancel )
    {
    leaveMoveResize();
    if( cancel )
        setGeometry( initialMoveResizeGeom );
    else
        setGeometry( moveResizeGeom );
    checkMaximizeGeometry();
// FRAME    update();
    Notify::raise( isResize() ? Notify::ResizeEnd : Notify::MoveEnd );
    if( effects )
        static_cast<EffectsHandlerImpl*>(effects)->windowUserMovedResized( effectWindow(), false, true );
    }

void Client::leaveMoveResize()
    {
    clearbound();
    if (geometryTip)
        {
        geometryTip->hide();
        delete geometryTip;
        geometryTip = NULL;
        }
    if ( ( isMove() && rules()->checkMoveResizeMode( options->moveMode ) != Options::Opaque )
      || ( isResize() && rules()->checkMoveResizeMode( options->resizeMode ) != Options::Opaque ) )
        ungrabXServer();
    if( move_resize_has_keyboard_grab )
        ungrabXKeyboard();
    move_resize_has_keyboard_grab = false;
    XUngrabPointer( display(), xTime() );
    XDestroyWindow( display(), move_resize_grab_window );
    move_resize_grab_window = None;
    workspace()->setClientIsMoving(0);
    if( move_faked_activity )
        workspace()->unfakeActivity( this );
    move_faked_activity = false;
    moveResizeMode = false;
    delete eater;
    eater = 0;
    delete sync_timeout;
    sync_timeout = NULL;
    if( options->electricBorders() == Options::ElectricMoveOnly )
        workspace()->reserveElectricBorderSwitching( false );
    }

// This function checks if it actually makes sense to perform a restricted move/resize.
// If e.g. the titlebar is already outside of the workarea, there's no point in performing
// a restricted move resize, because then e.g. resize would also move the window (#74555).
// NOTE: Most of it is duplicated from handleMoveResize().
void Client::checkUnrestrictedMoveResize()
    {
    if( unrestrictedMoveResize )
        return;
    QRect desktopArea = workspace()->clientArea( WorkArea, moveResizeGeom.center(), desktop());
    int left_marge, right_marge, top_marge, bottom_marge, titlebar_marge;
    // restricted move/resize - keep at least part of the titlebar always visible 
    // how much must remain visible when moved away in that direction
    left_marge = qMin( 100 + border_right, moveResizeGeom.width());
    right_marge = qMin( 100 + border_left, moveResizeGeom.width());
    // width/height change with opaque resizing, use the initial ones
    titlebar_marge = initialMoveResizeGeom.height();
    top_marge = border_bottom;
    bottom_marge = border_top;
    if( isResize())
        {
        if( moveResizeGeom.bottom() < desktopArea.top() + top_marge )
            unrestrictedMoveResize = true;
        if( moveResizeGeom.top() > desktopArea.bottom() - bottom_marge )
            unrestrictedMoveResize = true;
        if( moveResizeGeom.right() < desktopArea.left() + left_marge )
            unrestrictedMoveResize = true;
        if( moveResizeGeom.left() > desktopArea.right() - right_marge )
            unrestrictedMoveResize = true;
        if( !unrestrictedMoveResize && moveResizeGeom.top() < desktopArea.top() ) // titlebar mustn't go out
            unrestrictedMoveResize = true;
        }
    if( isMove())
        {
        if( moveResizeGeom.bottom() < desktopArea.top() + titlebar_marge - 1 ) // titlebar mustn't go out
            unrestrictedMoveResize = true;
        // no need to check top_marge, titlebar_marge already handles it
        if( moveResizeGeom.top() > desktopArea.bottom() - bottom_marge )
            unrestrictedMoveResize = true;
        if( moveResizeGeom.right() < desktopArea.left() + left_marge )
            unrestrictedMoveResize = true;
        if( moveResizeGeom.left() > desktopArea.right() - right_marge )
            unrestrictedMoveResize = true;
        }
    }

// When the user pressed mouse on the titlebar, don't activate move immediatelly,
// since it may be just a click. Activate instead after a delay. Move used to be
// activated only after moving by several pixels, but that looks bad.
void Client::startDelayedMoveResize()
    {
    delete delayedMoveResizeTimer;
    delayedMoveResizeTimer = new QTimer( this );
    connect( delayedMoveResizeTimer, SIGNAL( timeout()), this, SLOT( delayedMoveResize()));
    delayedMoveResizeTimer->setSingleShot( true );
    delayedMoveResizeTimer->start( QApplication::doubleClickInterval());
    }

void Client::stopDelayedMoveResize()
    {
    delete delayedMoveResizeTimer;
    delayedMoveResizeTimer = NULL;
    }

void Client::delayedMoveResize()
    {
    assert( buttonDown );
    if( !startMoveResize())
        buttonDown = false;
    updateCursor();
    stopDelayedMoveResize();
    }

void Client::handleMoveResize( int x, int y, int x_root, int y_root )
    {
    if(( mode == PositionCenter && !isMovable())
        || ( mode != PositionCenter && ( isShade() || !isResizable())))
        return;

    if ( !moveResizeMode )
        {
        QPoint p( QPoint( x, y ) - moveOffset );
        if (p.manhattanLength() >= 6)
            {
            if( !startMoveResize())
                {
                buttonDown = false;
                updateCursor();
                return;
                }
            updateCursor();
            }
        else
            return;
        }

    // ShadeHover or ShadeActive, ShadeNormal was already avoided above
    if ( mode != PositionCenter && shade_mode != ShadeNone )
        setShade( ShadeNone );

    QPoint globalPos( x_root, y_root );
    // these two points limit the geometry rectangle, i.e. if bottomleft resizing is done,
    // the bottomleft corner should be at is at (topleft.x(), bottomright().y())
    QPoint topleft = globalPos - moveOffset;
    QPoint bottomright = globalPos + invertedMoveOffset;
    QRect previousMoveResizeGeom = moveResizeGeom;

    // TODO move whole group when moving its leader or when the leader is not mapped?

    // compute bounds
    // NOTE: This is duped in checkUnrestrictedMoveResize().
    QRect desktopArea = workspace()->clientArea( WorkArea, globalPos, desktop());
    int left_marge, right_marge, top_marge, bottom_marge, titlebar_marge;
    if( unrestrictedMoveResize ) // unrestricted, just don't let it go out completely
        left_marge = right_marge = top_marge = bottom_marge = titlebar_marge = 5;
    else // restricted move/resize - keep at least part of the titlebar always visible 
        {        
        // how much must remain visible when moved away in that direction
        left_marge = qMin( 100 + border_right, moveResizeGeom.width());
        right_marge = qMin( 100 + border_left, moveResizeGeom.width());
        // width/height change with opaque resizing, use the initial ones
        titlebar_marge = initialMoveResizeGeom.height();
        top_marge = border_bottom;
        bottom_marge = border_top;
        }

    bool update = false;
    if( isResize())
        {
        // first resize (without checking constrains), then snap, then check bounds, then check constrains
        QRect orig = initialMoveResizeGeom;
        Sizemode sizemode = SizemodeAny;
        switch ( mode )
            {
            case PositionTopLeft:
                moveResizeGeom =  QRect( topleft, orig.bottomRight() ) ;
                break;
            case PositionBottomRight:
                moveResizeGeom =  QRect( orig.topLeft(), bottomright ) ;
                break;
            case PositionBottomLeft:
                moveResizeGeom =  QRect( QPoint( topleft.x(), orig.y() ), QPoint( orig.right(), bottomright.y()) ) ;
                break;
            case PositionTopRight:
                moveResizeGeom =  QRect( QPoint( orig.x(), topleft.y() ), QPoint( bottomright.x(), orig.bottom()) ) ;
                break;
            case PositionTop:
                moveResizeGeom =  QRect( QPoint( orig.left(), topleft.y() ), orig.bottomRight() ) ;
                sizemode = SizemodeFixedH; // try not to affect height
                break;
            case PositionBottom:
                moveResizeGeom =  QRect( orig.topLeft(), QPoint( orig.right(), bottomright.y() ) ) ;
                sizemode = SizemodeFixedH;
                break;
            case PositionLeft:
                moveResizeGeom =  QRect( QPoint( topleft.x(), orig.top() ), orig.bottomRight() ) ;
                sizemode = SizemodeFixedW;
                break;
            case PositionRight:
                moveResizeGeom =  QRect( orig.topLeft(), QPoint( bottomright.x(), orig.bottom() ) ) ;
                sizemode = SizemodeFixedW;
                break;
            case PositionCenter:
            default:
                assert( false );
                break;
            }

        // adjust new size to snap to other windows/borders
        moveResizeGeom = workspace()->adjustClientSize( this, moveResizeGeom, mode );

        // NOTE: This is duped in checkUnrestrictedMoveResize().
        if( moveResizeGeom.bottom() < desktopArea.top() + top_marge )
            moveResizeGeom.setBottom( desktopArea.top() + top_marge );
        if( moveResizeGeom.top() > desktopArea.bottom() - bottom_marge )
            moveResizeGeom.setTop( desktopArea.bottom() - bottom_marge );
        if( moveResizeGeom.right() < desktopArea.left() + left_marge )
            moveResizeGeom.setRight( desktopArea.left() + left_marge );
        if( moveResizeGeom.left() > desktopArea.right() - right_marge )
            moveResizeGeom.setLeft(desktopArea.right() - right_marge );
        if( !unrestrictedMoveResize && moveResizeGeom.top() < desktopArea.top() ) // titlebar mustn't go out
            moveResizeGeom.setTop( desktopArea.top());

        QSize size = adjustedSize( moveResizeGeom.size(), sizemode );
        // the new topleft and bottomright corners (after checking size constrains), if they'll be needed
        topleft = QPoint( moveResizeGeom.right() - size.width() + 1, moveResizeGeom.bottom() - size.height() + 1 );
        bottomright = QPoint( moveResizeGeom.left() + size.width() - 1, moveResizeGeom.top() + size.height() - 1 );
        orig = moveResizeGeom;
        switch ( mode )
            { // these 4 corners ones are copied from above
            case PositionTopLeft:
                moveResizeGeom =  QRect( topleft, orig.bottomRight() ) ;
                break;
            case PositionBottomRight:
                moveResizeGeom =  QRect( orig.topLeft(), bottomright ) ;
                break;
            case PositionBottomLeft:
                moveResizeGeom =  QRect( QPoint( topleft.x(), orig.y() ), QPoint( orig.right(), bottomright.y()) ) ;
                break;
            case PositionTopRight:
                moveResizeGeom =  QRect( QPoint( orig.x(), topleft.y() ), QPoint( bottomright.x(), orig.bottom()) ) ;
                break;
            // The side ones can't be copied exactly - if aspect ratios are specified, both dimensions may change.
            // Therefore grow to the right/bottom if needed.
            // TODO it should probably obey gravity rather than always using right/bottom ?
            case PositionTop:
                moveResizeGeom =  QRect( QPoint( orig.left(), topleft.y() ), QPoint( bottomright.x(), orig.bottom()) ) ;
                break;
            case PositionBottom:
                moveResizeGeom =  QRect( orig.topLeft(), QPoint( bottomright.x(), bottomright.y() ) ) ;
                break;
            case PositionLeft:
                moveResizeGeom =  QRect( QPoint( topleft.x(), orig.top() ), QPoint( orig.right(), bottomright.y()));
                break;
            case PositionRight:
                moveResizeGeom =  QRect( orig.topLeft(), QPoint( bottomright.x(), bottomright.y() ) ) ;
                break;
            case PositionCenter:
            default:
                assert( false );
                break;
            }
        if( moveResizeGeom.size() != previousMoveResizeGeom.size())
            update = true;
        }
    else if( isMove())
        {
        assert( mode == PositionCenter );
        // first move, then snap, then check bounds
        moveResizeGeom.moveTopLeft( topleft );
        moveResizeGeom.moveTopLeft( workspace()->adjustClientPosition( this, moveResizeGeom.topLeft() ) );
        // NOTE: This is duped in checkUnrestrictedMoveResize().
        if( moveResizeGeom.bottom() < desktopArea.top() + titlebar_marge - 1 ) // titlebar mustn't go out
            moveResizeGeom.moveBottom( desktopArea.top() + titlebar_marge - 1 );
        // no need to check top_marge, titlebar_marge already handles it
        if( moveResizeGeom.top() > desktopArea.bottom() - bottom_marge )
            moveResizeGeom.moveTop( desktopArea.bottom() - bottom_marge );
        if( moveResizeGeom.right() < desktopArea.left() + left_marge )
            moveResizeGeom.moveRight( desktopArea.left() + left_marge );
        if( moveResizeGeom.left() > desktopArea.right() - right_marge )
            moveResizeGeom.moveLeft(desktopArea.right() - right_marge );
        if( moveResizeGeom.topLeft() != previousMoveResizeGeom.topLeft())
            update = true;
        }
    else
        assert( false );

    if( isResize())
        {
        if( sync_timeout != NULL )
            {
            sync_resize_pending = true;
            return;
            }
        }

    if( update )
        performMoveResize();
    if ( isMove() )
        workspace()->checkElectricBorder(globalPos, xTime());
    }

void Client::performMoveResize()
    {    
#ifdef HAVE_XSYNC
    if( isResize() && sync_counter != None )
        {
        sync_timeout = new QTimer( this );
        connect( sync_timeout, SIGNAL( timeout()), SLOT( syncTimeout()));
        sync_timeout->setSingleShot( true );
        sync_timeout->start( 500 );
        sendSyncRequest();
        }
#endif
    sync_resize_pending = false;
    if( rules()->checkMoveResizeMode
        ( isResize() ? options->resizeMode : options->moveMode ) == Options::Opaque )
        {
        setGeometry( moveResizeGeom );
        positionGeometryTip();
        }
    else if( rules()->checkMoveResizeMode
        ( isResize() ? options->resizeMode : options->moveMode ) == Options::Transparent )
        {
        clearbound();  // it's necessary to move the geometry tip when there's no outline
        positionGeometryTip(); // shown, otherwise it would cause repaint problems in case
        drawbound( moveResizeGeom ); // they overlap; the paint event will come after this,
        }                               // so the geometry tip will be painted above the outline
    if( effects )
        static_cast<EffectsHandlerImpl*>(effects)->windowUserMovedResized( effectWindow(), false, false );
    }

void Client::syncTimeout()
    {
    sync_timeout->deleteLater();
    sync_timeout = NULL;
    if( sync_resize_pending )
        performMoveResize();
    }

} // namespace
