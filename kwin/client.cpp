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

#include "client.h"

#include <QApplication>
#include <QPainter>
#include <QDateTime>
#include <QProcess>
#include <unistd.h>
#include <kstandarddirs.h>
#include <QWhatsThis>
#include <kwindowsystem.h>
#include <kiconloader.h>
#include <stdlib.h>
#include <signal.h>

#include "bridge.h"
#include "group.h"
#include "workspace.h"
#include "atoms.h"
#include "notifications.h"
#include "rules.h"
#include "scene.h"
#include "effects.h"
#include "deleted.h"

#include <X11/extensions/shape.h>
#include <QX11Info>

#ifdef HAVE_XSYNC
#include <X11/extensions/sync.h>
#endif

// put all externs before the namespace statement to allow the linker
// to resolve them properly

namespace KWin
{

/*

 Creating a client:
     - only by calling Workspace::createClient()
         - it creates a new client and calls manage() for it

 Destroying a client:
     - destroyClient() - only when the window itself has been destroyed
     - releaseWindow() - the window is kept, only the client itself is destroyed

*/


/*!
  \class Client client.h

  \brief The Client class encapsulates a window decoration frame.

*/

/*!
 This ctor is "dumb" - it only initializes data. All the real initialization
 is done in manage().
 */
Client::Client( Workspace *ws )
    :   Toplevel( ws ),
        client( None ),
        wrapper( None ),
        decoration( NULL ),
        bridge( new Bridge( this )),
        move_faked_activity( false ),
        move_resize_grab_window( None ),
        move_resize_has_keyboard_grab( false ),
        transient_for( NULL ),
        transient_for_id( None ),
        original_transient_for_id( None ),
        autoRaiseTimer( NULL ),
        shadeHoverTimer( NULL ),
        delayedMoveResizeTimer( NULL ),
        in_group( NULL ),
        window_group( None ),
        in_layer( UnknownLayer ),
        ping_timer( NULL ),
        process_killer( NULL ),
        user_time( CurrentTime ), // not known yet
        allowed_actions( 0 ),
        block_geometry_updates( 0 ),
        pending_geometry_update( PendingGeometryNone ),
        shade_geometry_change( false ),
#ifdef HAVE_XSYNC
        sync_counter( None ),
        sync_alarm( None ),
#endif
        sync_timeout( NULL ),
        sync_resize_pending( false ),
        border_left( 0 ),
        border_right( 0 ),
        border_top( 0 ),
        border_bottom( 0 ),
        sm_stacking_order( -1 ),
        demandAttentionKNotifyTimer( NULL )
// SELI do all as initialization
    {

    // set the initial mapping state
    mapping_state = WithdrawnState;
    desk = 0; // no desktop yet

    mode = PositionCenter;
    buttonDown = false;
    moveResizeMode = false;

    info = NULL;

    shade_mode = ShadeNone;
    active = false;
    deleting = false;
    keep_above = false;
    keep_below = false;
    motif_may_move = true;
    motif_may_resize = true;
    motif_may_close = true;
    fullscreen_mode = FullScreenNone;
    skip_taskbar = false;
    original_skip_taskbar = false;
    minimized = false;
    hidden = false;
    modal = false;
    noborder = false;
    app_noborder = false;
    urgency = false;
    ignore_focus_stealing = false;
    demands_attention = false;
    hidden_preview = false;
    raw_shown = false;
    check_active_modal = false;

    Pdeletewindow = 0;
    Ptakefocus = 0;
    Ptakeactivity = 0;
    Pcontexthelp = 0;
    Pping = 0;
    input = false;
    skip_pager = false;

    max_mode = MaximizeRestore;
    maxmode_restore = MaximizeRestore;
    
    cmap = None;
    
    geom = QRect( 0, 0, 100, 100 ); // so that decorations don't start with size being (0,0)
    client_size = QSize( 100, 100 );
#if defined(HAVE_XSYNC) || defined(HAVE_XDAMAGE)
    ready_for_painting = false; // wait for first damage or sync reply
#endif

    // SELI initialize xsizehints??
    }

/*!
  "Dumb" destructor.
 */
Client::~Client()
    {
#ifdef HAVE_XSYNC
    if( sync_alarm != None )
        XSyncDestroyAlarm( display(), sync_alarm );
#endif
    assert(!moveResizeMode);
    assert( client == None );
    assert( wrapper == None );
//    assert( frameId() == None );
    assert( decoration == NULL );
    assert( block_geometry_updates == 0 );
    assert( !check_active_modal );
    delete bridge;
    }

// use destroyClient() or releaseWindow(), Client instances cannot be deleted directly
void Client::deleteClient( Client* c, allowed_t )
    {
    delete c;
    }

/*!
  Releases the window. The client has done its job and the window is still existing.
 */
void Client::releaseWindow( bool on_shutdown )
    {
    assert( !deleting );
    deleting = true;
    Deleted* del = Deleted::create( this );
    if( effects )
        {
        static_cast<EffectsHandlerImpl*>(effects)->windowClosed( effectWindow());
        scene->windowClosed( this, del );
        }
    finishCompositing();
    workspace()->discardUsedWindowRules( this, true ); // remove ForceTemporarily rules
    StackingUpdatesBlocker blocker( workspace());
    if (moveResizeMode)
       leaveMoveResize();
    finishWindowRules();
    ++block_geometry_updates;
    if( isOnCurrentDesktop() && isShown( true ))
        addWorkspaceRepaint( geometry());
    // grab X during the release to make removing of properties, setting to withdrawn state
    // and repareting to root an atomic operation (http://lists.kde.org/?l=kde-devel&m=116448102901184&w=2)
    grabXServer();
    setMappingState( WithdrawnState );
    setModal( false ); // otherwise its mainwindow wouldn't get focus
    hidden = true; // so that it's not considered visible anymore (can't use hideClient(), it would set flags)
    if( !on_shutdown )
        workspace()->clientHidden( this );
    XUnmapWindow( display(), frameId()); // destroying decoration would cause ugly visual effect
    destroyDecoration();
    cleanGrouping();
    if( !on_shutdown )
        {
        workspace()->removeClient( this, Allowed );
        // only when the window is being unmapped, not when closing down KWin
        // (NETWM sections 5.5,5.7)
        info->setDesktop( 0 );
        desk = 0;
        info->setState( 0, info->state()); // reset all state flags
        }
    XDeleteProperty( display(), client, atoms->kde_net_wm_user_creation_time);
    XDeleteProperty( display(), client, atoms->net_frame_extents );
    XDeleteProperty( display(), client, atoms->kde_net_wm_frame_strut );
    XReparentWindow( display(), client, rootWindow(), x(), y());
    XRemoveFromSaveSet( display(), client );
    XSelectInput( display(), client, NoEventMask );
    if( on_shutdown )
        { // map the window, so it can be found after another WM is started
        XMapWindow( display(), client );
	// TODO preserve minimized, shaded etc. state?
        }
    else
        {
        // Make sure it's not mapped if the app unmapped it (#65279). The app
        // may do map+unmap before we initially map the window by calling rawShow() from manage().
        XUnmapWindow( display(), client ); 
        }
    client = None;
    XDestroyWindow( display(), wrapper );
    wrapper = None;
    XDestroyWindow( display(), frameId());
//    frame = None;
    --block_geometry_updates; // don't use GeometryUpdatesBlocker, it would now set the geometry
    disownDataPassedToDeleted();
    del->unrefWindow();
    checkNonExistentClients();
    deleteClient( this, Allowed );
    ungrabXServer();
    }

// like releaseWindow(), but this one is called when the window has been already destroyed
// (e.g. the application closed it)
void Client::destroyClient()
    {
    assert( !deleting );
    deleting = true;
    Deleted* del = Deleted::create( this );
    if( effects )
        {
        static_cast<EffectsHandlerImpl*>(effects)->windowClosed( effectWindow());
        scene->windowClosed( this, del );
        }
    finishCompositing();
    workspace()->discardUsedWindowRules( this, true ); // remove ForceTemporarily rules
    StackingUpdatesBlocker blocker( workspace());
    if (moveResizeMode)
       leaveMoveResize();
    finishWindowRules();
    ++block_geometry_updates;
    if( isOnCurrentDesktop() && isShown( true ))
        addWorkspaceRepaint( geometry());
    setModal( false );
    hidden = true; // so that it's not considered visible anymore
    workspace()->clientHidden( this );
    destroyDecoration();
    cleanGrouping();
    workspace()->removeClient( this, Allowed );
    client = None; // invalidate
    XDestroyWindow( display(), wrapper );
    wrapper = None;
    XDestroyWindow( display(), frameId());
//    frame = None;
    --block_geometry_updates; // don't use GeometryUpdatesBlocker, it would now set the geometry
    disownDataPassedToDeleted();
    del->unrefWindow();
    checkNonExistentClients();
    deleteClient( this, Allowed );
    }

void Client::updateDecoration( bool check_workspace_pos, bool force )
    {
    if( !force && (( decoration == NULL && noBorder())
                    || ( decoration != NULL && !noBorder())))
        return;
    bool do_show = false;
    QRect oldgeom = geometry();
    blockGeometryUpdates( true );
    if( force )
        destroyDecoration();
    if( !noBorder())
        {
        setMask( QRegion()); // reset shape mask
        decoration = workspace()->createDecoration( bridge );
        // TODO check decoration's minimum size?
        decoration->init();
        decoration->widget()->installEventFilter( this );
        XReparentWindow( display(), decoration->widget()->winId(), frameId(), 0, 0 );
        decoration->widget()->lower();
        decoration->borders( border_left, border_right, border_top, border_bottom );
        int save_workarea_diff_x = workarea_diff_x;
        int save_workarea_diff_y = workarea_diff_y;
        move( calculateGravitation( false ));
        plainResize( sizeForClientSize( clientSize()), ForceGeometrySet );
        workarea_diff_x = save_workarea_diff_x;
        workarea_diff_y = save_workarea_diff_y;
        do_show = true;
        if( compositing() )
            discardWindowPixmap();
        if( scene != NULL )
            scene->windowGeometryShapeChanged( this );
        if( effects != NULL )
            static_cast<EffectsHandlerImpl*>(effects)->windowGeometryShapeChanged( effectWindow(), oldgeom );
        }
    else
        destroyDecoration();
    if( check_workspace_pos )
        checkWorkspacePosition();
    blockGeometryUpdates( false );
    if( do_show )
        decoration->widget()->show();
    updateFrameExtents();
    }

void Client::destroyDecoration()
    {
    QRect oldgeom = geometry();
    if( decoration != NULL )
        {
        delete decoration;
        decoration = NULL;
        QPoint grav = calculateGravitation( true );
        border_left = border_right = border_top = border_bottom = 0;
        setMask( QRegion()); // reset shape mask
        int save_workarea_diff_x = workarea_diff_x;
        int save_workarea_diff_y = workarea_diff_y;
        plainResize( sizeForClientSize( clientSize()), ForceGeometrySet );
        move( grav );
        workarea_diff_x = save_workarea_diff_x;
        workarea_diff_y = save_workarea_diff_y;
        if( compositing() )
            discardWindowPixmap();
        if( scene != NULL && !deleting ) 
            scene->windowGeometryShapeChanged( this );
        if( effects != NULL && !deleting )
            static_cast<EffectsHandlerImpl*>(effects)->windowGeometryShapeChanged( effectWindow(), oldgeom );
        }
    }

bool Client::checkBorderSizes( bool also_resize )
    {
    if( decoration == NULL )
        return false;
    int new_left, new_right, new_top, new_bottom;
    decoration->borders( new_left, new_right, new_top, new_bottom );
    if( new_left == border_left && new_right == border_right
        && new_top == border_top && new_bottom == border_bottom )
        return false;
    if( !also_resize )
        {
        border_left = new_left;
        border_right = new_right;
        border_top = new_top;
        border_bottom = new_bottom;
        return true;
        }
    GeometryUpdatesBlocker blocker( this );
    move( calculateGravitation( true ));
    border_left = new_left;
    border_right = new_right;
    border_top = new_top;
    border_bottom = new_bottom;
    move( calculateGravitation( false ));
    plainResize( sizeForClientSize( clientSize()), ForceGeometrySet );
    checkWorkspacePosition();
    return true;
    }

void Client::repaintDecoration()
    {
    if( decoration != NULL )
        decoration->widget()->update();
    }

void Client::detectNoBorder()
    {
    if( shape())
        {
        noborder = true;
        app_noborder = true;
        return;
        }
    switch( windowType())
        {
        case NET::Desktop :
        case NET::Dock :
        case NET::TopMenu :
        case NET::Splash :
            noborder = true;
            app_noborder = true;
          break;
        case NET::Unknown :
        case NET::Normal :
        case NET::Toolbar :
        case NET::Menu :
        case NET::Dialog :
        case NET::Utility :
            noborder = false;
          break;
        default:
            assert( false );
        }
    // NET::Override is some strange beast without clear definition, usually
    // just meaning "noborder", so let's treat it only as such flag, and ignore it as
    // a window type otherwise (SUPPORTED_WINDOW_TYPES_MASK doesn't include it)
    if( info->windowType( SUPPORTED_MANAGED_WINDOW_TYPES_MASK | NET::OverrideMask ) == NET::Override )
        {
        noborder = true;
        app_noborder = true;
        }
    }

void Client::updateFrameExtents()
    {
    NETStrut strut;
    strut.left = border_left;
    strut.right = border_right;
    strut.top = border_top;
    strut.bottom = border_bottom;
    info->setFrameExtents( strut );
    }

// Resizes the decoration, and makes sure the decoration widget gets resize event
// even if the size hasn't changed. This is needed to make sure the decoration
// re-layouts (e.g. when options()->moveResizeMaximizedWindows() changes,
// the decoration may turn on/off some borders, but the actual size
// of the decoration stays the same).
void Client::resizeDecoration( const QSize& s )
    {
    if( decoration == NULL )
        return;
    QSize oldsize = decoration->widget()->size();
    decoration->resize( s );
    if( oldsize == s )
        {
        QResizeEvent e( s, oldsize );
        QApplication::sendEvent( decoration->widget(), &e );
        }
    }

bool Client::noBorder() const
    {
    return noborder || isFullScreen();
    }

bool Client::userCanSetNoBorder() const
    {
    return !isFullScreen() && !isShade();
    }

void Client::setNoBorder( bool set )
    {
    if( !userCanSetNoBorder())
        return;
    set = rules()->checkNoBorder( set );
    if( noborder == set )
        return;
    noborder = set;
    updateDecoration( true, false );
    updateWindowRules();
    }

void Client::updateShape()
    {
    // workaround for #19644 - shaped windows shouldn't have decoration
    if( shape())
        {
        if( !app_noborder ) // only when shape is detected for the first time,
            {               // still let the user to override
            app_noborder = true;
            noborder = true;
            updateDecoration( true );
            }
        }
    if( shape() && noBorder())
        XShapeCombineShape( display(), frameId(), ShapeBounding,
            clientPos().x(), clientPos().y(), window(), ShapeBounding, ShapeSet );
    // Decoration mask (i.e. 'else' here) setting is done in setMask()
    // when the decoration calls it or when the decoration is created/destroyed
    updateInputShape();
    if( compositing())
        {
        addRepaintFull();
        addWorkspaceRepaint( geometry()); // in case shape change removes part of this window
        }
    if( scene != NULL )
        scene->windowGeometryShapeChanged( this );
    if( effects != NULL )
        static_cast<EffectsHandlerImpl*>(effects)->windowGeometryShapeChanged( effectWindow(), geometry());
    }

static Window shape_helper_window = None;

void Client::updateInputShape()
    {
    if( hidden_preview ) // sets it to none, don't change
        return;
    if( Extensions::shapeInputAvailable())
        { // There appears to be no way to find out if a window has input
          // shape set or not, so always propagate the input shape
          // (it's the same like the bounding shape by default).
          // Also, build the shape using a helper window, not directly
          // in the frame window, because the sequence set-shape-to-frame,
          // remove-shape-of-client, add-input-shape-of-client has the problem
          // that after the second step there's a hole in the input shape
          // until the real shape of the client is added and that can make
          // the window lose focus (which is a problem with mouse focus policies)
          // TODO it seems there is, after all - XShapeGetRectangles() - but maybe this is better
        if( shape_helper_window == None )
            shape_helper_window = XCreateSimpleWindow( display(), rootWindow(),
                0, 0, 1, 1, 0, 0, 0 );
        XResizeWindow( display(), shape_helper_window, width(), height());
        XShapeCombineShape( display(), shape_helper_window, ShapeInput, 0, 0,
                           frameId(), ShapeBounding, ShapeSet );
        XShapeCombineShape( display(), shape_helper_window, ShapeInput,
                           clientPos().x(), clientPos().y(),
                           window(), ShapeBounding, ShapeSubtract );
        XShapeCombineShape( display(), shape_helper_window, ShapeInput,
                           clientPos().x(), clientPos().y(),
                           window(), ShapeInput, ShapeUnion );
        XShapeCombineShape( display(), frameId(), ShapeInput, 0, 0,
                           shape_helper_window, ShapeInput, ShapeSet );
        }
    }

void Client::setMask( const QRegion& reg, int mode )
    {
    if( _mask == reg )
        return;
    _mask = reg;
    Window shape_window = frameId();
    if( shape())
        {
        // the same way of applying a shape without strange intermediate states like above
        if( shape_helper_window == None )
            shape_helper_window = XCreateSimpleWindow( display(), rootWindow(),
                0, 0, 1, 1, 0, 0, 0 );
        shape_window = shape_helper_window;
        }
    if( reg.isEmpty())
        XShapeCombineMask( display(), shape_window, ShapeBounding, 0, 0,
            None, ShapeSet );
    else if( mode == X::Unsorted )
        XShapeCombineRegion( display(), shape_window, ShapeBounding, 0, 0,
            reg.handle(), ShapeSet );
    else
        {
        QVector< QRect > rects = reg.rects();
        XRectangle* xrects = new XRectangle[ rects.count() ];
        for( int i = 0;
             i < rects.count();
             ++i )
            {
            xrects[ i ].x = rects[ i ].x();
            xrects[ i ].y = rects[ i ].y();
            xrects[ i ].width = rects[ i ].width();
            xrects[ i ].height = rects[ i ].height();
            }
        XShapeCombineRectangles( display(), shape_window, ShapeBounding, 0, 0,
            xrects, rects.count(), ShapeSet, mode );
        delete[] xrects;
        }
    if( shape())
        { // the rest of the applyign using a temporary window
        XRectangle rec = { 0, 0, clientSize().width(), clientSize().height() };
        XShapeCombineRectangles( display(), shape_helper_window, ShapeBounding,
                           clientPos().x(), clientPos().y(), &rec, 1, ShapeSubtract, Unsorted );
        XShapeCombineShape( display(), shape_helper_window, ShapeBounding,
                           clientPos().x(), clientPos().y(),
                           window(), ShapeBounding, ShapeUnion );
        XShapeCombineShape( display(), frameId(), ShapeBounding, 0, 0,
                           shape_helper_window, ShapeBounding, ShapeSet );
        }
    if( scene != NULL )
        scene->windowGeometryShapeChanged( this );
    if( effects != NULL )
        static_cast<EffectsHandlerImpl*>(effects)->windowGeometryShapeChanged( effectWindow(), geometry());
    updateShape();
    }

QRegion Client::mask() const
    {
    if( _mask.isEmpty())
        return QRegion( 0, 0, width(), height());
    return _mask;
    }
    
void Client::hideClient( bool hide )
    {
    if( hidden == hide )
        return;
    hidden = hide;
    updateVisibility();
    }
    
/*
  Returns whether the window is minimizable or not
 */
bool Client::isMinimizable() const
    {
    if( isSpecialWindow())
        return false;
    if( isTransient())
        { // #66868 - let other xmms windows be minimized when the mainwindow is minimized
        bool shown_mainwindow = false;
        ClientList mainclients = mainClients();
        for( ClientList::ConstIterator it = mainclients.begin();
             it != mainclients.end();
             ++it )
            {
            if( (*it)->isShown( true ))
                shown_mainwindow = true;
            }
        if( !shown_mainwindow )
            return true;
        }
    // this is here because kicker's taskbar doesn't provide separate entries
    // for windows with an explicitly given parent
    // TODO perhaps this should be redone
    if( transientFor() != NULL )
        return false;
    if( !wantsTabFocus()) // SELI - NET::Utility? why wantsTabFocus() - skiptaskbar? ?
        return false;
    return true;
    }

/*!
  Minimizes this client plus its transients
 */
void Client::minimize( bool avoid_animation )
    {
    if ( !isMinimizable() || isMinimized())
        return;

    Notify::raise( Notify::Minimize );

    minimized = true;

    updateVisibility();
    updateAllowedActions();
    workspace()->updateMinimizedOfTransients( this );
    updateWindowRules();
    workspace()->updateFocusChains( this, Workspace::FocusChainMakeLast );
    if( effects && !avoid_animation ) // TODO shouldn't it tell effects at least about the change?
        static_cast<EffectsHandlerImpl*>(effects)->windowMinimized( effectWindow());
    }

void Client::unminimize( bool avoid_animation )
    {
    if( !isMinimized())
        return;

    Notify::raise( Notify::UnMinimize );
    minimized = false;
    updateVisibility();
    updateAllowedActions();
    workspace()->updateMinimizedOfTransients( this );
    updateWindowRules();
    if( effects && !avoid_animation )
        static_cast<EffectsHandlerImpl*>(effects)->windowUnminimized( effectWindow());
    }

QRect Client::iconGeometry() const
    {
    NETRect r = info->iconGeometry();
    QRect geom( r.pos.x, r.pos.y, r.size.width, r.size.height );
    if( geom.isValid() )
        return geom;
    else
    {
        // Check all mainwindows of this window (recursively)
        foreach( Client* mainwin, mainClients() )
            {
            geom = mainwin->iconGeometry();
            if( geom.isValid() )
                return geom;
            }
        // No mainwindow (or their parents) with icon geometry was found
        return QRect();
        }
    }

bool Client::isShadeable() const
    {
    return !isSpecialWindow() && !noBorder();
    }

void Client::setShade( ShadeMode mode )
    {
    if( !isShadeable())
        return;
    mode = rules()->checkShade( mode );
    if( shade_mode == mode )
        return;
    bool was_shade = isShade();
    ShadeMode was_shade_mode = shade_mode;
    shade_mode = mode;
    if( was_shade == isShade())
        {
        if( decoration != NULL ) // decoration may want to update after e.g. hover-shade changes
            decoration->shadeChange();
        return; // no real change in shaded state
        }

    if( shade_mode == ShadeNormal )
        {
        if ( isShown( true ) && isOnCurrentDesktop())
                Notify::raise( Notify::ShadeUp );
        }
    else if( shade_mode == ShadeNone )
        {
        if( isShown( true ) && isOnCurrentDesktop())
                Notify::raise( Notify::ShadeDown );
        }

    assert( decoration != NULL ); // noborder windows can't be shaded
    GeometryUpdatesBlocker blocker( this );
    // decorations may turn off some borders when shaded
    decoration->borders( border_left, border_right, border_top, border_bottom );

// TODO all this unmapping, resizing etc. feels too much duplicated from elsewhere
    if ( isShade()) 
        { // shade_mode == ShadeNormal
        addWorkspaceRepaint( geometry());
        // shade
        shade_geometry_change = true;
        QSize s( sizeForClientSize( QSize( clientSize())));
        s.setHeight( border_top + border_bottom );
        XSelectInput( display(), wrapper, ClientWinMask ); // avoid getting UnmapNotify
        XUnmapWindow( display(), wrapper );
        XUnmapWindow( display(), client );
        XSelectInput( display(), wrapper, ClientWinMask | SubstructureNotifyMask );
        plainResize( s );
        shade_geometry_change = false;
        if( isActive())
            {
            if( was_shade_mode == ShadeHover )
                workspace()->activateNextClient( this );
            else
                workspace()->focusToNull();
            }
        }
    else 
        {
        shade_geometry_change = true;
        QSize s( sizeForClientSize( clientSize()));
        shade_geometry_change = false;
        plainResize( s );
        if( shade_mode == ShadeHover || shade_mode == ShadeActivated )
            setActive( true );
        XMapWindow( display(), wrapperId());
        XMapWindow( display(), window());
        if ( isActive() )
            workspace()->requestFocus( this );
        }
    checkMaximizeGeometry();
    info->setState( isShade() ? NET::Shaded : 0, NET::Shaded );
    info->setState( isShown( false ) ? 0 : NET::Hidden, NET::Hidden );
    discardWindowPixmap();
    updateVisibility();
    updateAllowedActions();
    workspace()->updateMinimizedOfTransients( this );
    decoration->shadeChange();
    updateWindowRules();
    }

void Client::shadeHover()
    {
    setShade( ShadeHover );
    cancelShadeHover();
    }

void Client::cancelShadeHover()
    {
    delete shadeHoverTimer;
    shadeHoverTimer = 0;
    }

void Client::toggleShade()
    {
    // if the mode is ShadeHover or ShadeActive, cancel shade too
    setShade( shade_mode == ShadeNone ? ShadeNormal : ShadeNone );
    }

void Client::updateVisibility()
    {
    if( deleting )
        return;
    bool show = true;
    if( hidden )
        {
        setMappingState( IconicState );
        info->setState( NET::Hidden, NET::Hidden );
        setSkipTaskbar( true, false ); // also hide from taskbar
        rawHide();
        show = false;
        }
    else
        {
        setSkipTaskbar( original_skip_taskbar, false );
        }
    if( minimized )
        {
        setMappingState( IconicState );
        info->setState( NET::Hidden, NET::Hidden );
        rawHide();
        show = false;
        }
    if( show )
        info->setState( 0, NET::Hidden );
    if( !isOnCurrentDesktop())
        {
        setMappingState( IconicState );
        rawHide();
        show = false;
        }
    if( show )
        {
        bool belongs_to_desktop = false;
        for( ClientList::ConstIterator it = group()->members().begin();
             it != group()->members().end();
             ++it )
            if( (*it)->isDesktop())
                {
                belongs_to_desktop = true;
                break;
                }
        if( !belongs_to_desktop && workspace()->showingDesktop())
            workspace()->resetShowingDesktop( true );
        if( isShade())
            setMappingState( IconicState );
        else
            setMappingState( NormalState );
        rawShow();
        }
    }

/*!
  Sets the client window's mapping state. Possible values are
  WithdrawnState, IconicState, NormalState.
 */
void Client::setMappingState(int s)
    {
    assert( client != None );
    assert( !deleting || s == WithdrawnState );
    if( mapping_state == s )
        return;
    bool was_unmanaged = ( mapping_state == WithdrawnState );
    mapping_state = s;
    if( mapping_state == WithdrawnState )
        {
        XDeleteProperty( display(), window(), atoms->wm_state );
        return;
        }
    assert( s == NormalState || s == IconicState );

    unsigned long data[2];
    data[0] = (unsigned long) s;
    data[1] = (unsigned long) None;
    XChangeProperty(display(), window(), atoms->wm_state, atoms->wm_state, 32,
        PropModeReplace, (unsigned char *)data, 2);

    if( was_unmanaged ) // manage() did block_geometry_updates = 1, now it's ok to finally set the geometry
        blockGeometryUpdates( false );
    }

/*!
  Reimplemented to map the managed window in the window wrapper.
  Proper mapping state should be set before showing the client.
 */
void Client::rawShow()
    {
    if( raw_shown ) // this flag is used to purely avoid repeated calls to rawShow(),
        return;     // it doesn't say anything more about the state
    raw_shown = true;
    if( decoration != NULL )
        decoration->widget()->show(); // not really necessary, but let it know the state
    XMapWindow( display(), frameId());
    if( !isShade())
        {
        XMapWindow( display(), wrapper );
        XMapWindow( display(), client );
        }
    if( options->hiddenPreviews == HiddenPreviewsNever )
        {
        // XComposite invalidates backing pixmaps on unmap (minimize, different
        // virtual desktop, etc.).  We kept the last known good pixmap around
        // for use in effects, but now we want to have access to the new pixmap
        if( compositing() )
            discardWindowPixmap();
        }
    else
        {
        if( hidden_preview )
            setHiddenPreview( false, Allowed );
        }
    }

/*!
  Reimplemented to unmap the managed window in the window wrapper.
  Also informs the workspace.
  Proper mapping state should be set before hiding the client.
*/
void Client::rawHide()
    {
    if( !raw_shown )
        return;
    raw_shown = false;
    StackingUpdatesBlocker blocker( workspace());
    addWorkspaceRepaint( geometry());
    if( options->hiddenPreviews == HiddenPreviewsNever )
        {
        // Here it may look like a race condition, as some other client might try to unmap
        // the window between these two XSelectInput() calls. However, they're supposed to
        // use XWithdrawWindow(), which also sends a synthetic event to the root window,
        // which won't be missed, so this shouldn't be a problem. The chance the real UnmapNotify
        // will be missed is also very minimal, so I don't think it's needed to grab the server
        // here.
            XSelectInput( display(), wrapper, ClientWinMask ); // avoid getting UnmapNotify
            XUnmapWindow( display(), frameId());
            XUnmapWindow( display(), wrapper );
            XUnmapWindow( display(), client );
            XSelectInput( display(), wrapper, ClientWinMask | SubstructureNotifyMask );
            if( decoration != NULL )
                decoration->widget()->hide(); // not really necessary, but let it know the state
        }
    else
        {
        if( !hidden_preview )
            {
            setHiddenPreview( true, Allowed );
            // actually keep the window mapped (taken from rawShow())
            if( decoration != NULL )
                decoration->widget()->show(); // not really necessary, but let it know the state
            XMapWindow( display(), frameId());
            if( !isShade())
                {
                XMapWindow( display(), wrapper );
                XMapWindow( display(), client );
                }
            }
        }
    workspace()->clientHidden( this );
    }

// XComposite doesn't keep window pixmaps of unmapped windows, which means
// there wouldn't be any previews of windows that are minimized or on another
// virtual desktop. Therefore rawHide() actually keeps such windows mapped.
// However special care needs to be taken so that such windows don't interfere.
// Therefore they're put very low in the stacking order and they have input shape
// set to none, which hopefully is enough. If there's no input shape available,
// then it's hoped that there will be some other desktop above it *shrug*.
// Using normal shape would be better, but that'd affect other things, e.g. painting
// of the actual preview.
void Client::setHiddenPreview( bool set, allowed_t )
    {
    if( set && !hidden_preview )
        { // set
        hidden_preview = true;
        workspace()->forceRestacking();
        if( Extensions::shapeInputAvailable())
            XShapeCombineRectangles( display(), frameId(), ShapeInput, 0, 0, NULL, 0, ShapeSet, Unsorted );
        }
    else if( !set && hidden_preview )
        { // unset
        hidden_preview = false;
        workspace()->forceRestacking();
        updateInputShape();
        }
    }

void Client::sendClientMessage(Window w, Atom a, Atom protocol, long data1, long data2, long data3)
    {
    XEvent ev;
    long mask;

    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = w;
    ev.xclient.message_type = a;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = protocol;
    ev.xclient.data.l[1] = xTime();
    ev.xclient.data.l[2] = data1;
    ev.xclient.data.l[3] = data2;
    ev.xclient.data.l[4] = data3;
    mask = 0L;
    if (w == rootWindow())
      mask = SubstructureRedirectMask;        /* magic! */
    XSendEvent(display(), w, False, mask, &ev);
    }

/*
  Returns whether the window may be closed (have a close button)
 */
bool Client::isCloseable() const
    {
    return rules()->checkCloseable( motif_may_close && !isSpecialWindow());
    }

/*!
  Closes the window by either sending a delete_window message or
  using XKill.
 */
void Client::closeWindow()
    {
    if( !isCloseable())
        return;
    // Update user time, because the window may create a confirming dialog.
    updateUserTime(); 
    if ( Pdeletewindow )
        {
        Notify::raise( Notify::Close );
        sendClientMessage( window(), atoms->wm_protocols, atoms->wm_delete_window);
        pingWindow();
        }
    else 
        {
        // client will not react on wm_delete_window. We have not choice
        // but destroy his connection to the XServer.
        killWindow();
        }
    }


/*!
  Kills the window via XKill
 */
void Client::killWindow()
    {
    kDebug( 1212 ) << "Client::killWindow():" << caption();
    // not sure if we need an Notify::Kill or not.. until then, use
    // Notify::Close
    Notify::raise( Notify::Close );

    if( isDialog())
        Notify::raise( Notify::TransDelete );
    if( isNormalWindow())
        Notify::raise( Notify::Delete );
    killProcess( false );
    // always kill this client at the server
    XKillClient(display(), window() );
    destroyClient();
    }

// send a ping to the window using _NET_WM_PING if possible
// if it doesn't respond within a reasonable time, it will be
// killed
void Client::pingWindow()
    {
    if( !Pping )
        return; // can't ping :(
    if( options->killPingTimeout == 0 )
        return; // turned off
    if( ping_timer != NULL )
        return; // pinging already
    ping_timer = new QTimer( this );
    connect( ping_timer, SIGNAL( timeout()), SLOT( pingTimeout()));
    ping_timer->setSingleShot( true );
    ping_timer->start( options->killPingTimeout );
    ping_timestamp = xTime();
    workspace()->sendPingToWindow( window(), ping_timestamp );
    }

void Client::gotPing( Time timestamp )
    {
    // just plain compare is not good enough because of 64bit and truncating and whatnot
    if( NET::timestampCompare( timestamp, ping_timestamp ) != 0 )
        return;
    delete ping_timer;
    ping_timer = NULL;
    if( process_killer != NULL )
        {
        process_killer->kill();
        // recycle when the process manager has noticed that the process exited
        // a delete process_killer here sometimes causes a hang in waitForFinished
        connect(process_killer, SIGNAL(finished(int, QProcess::ExitStatus)), process_killer,
                SLOT(deleteLater()));
        process_killer = NULL;
        }
    }

void Client::pingTimeout()
    {
    kDebug( 1212 ) << "Ping timeout:" << caption();
    ping_timer->deleteLater();
    ping_timer = NULL;
    killProcess( true, ping_timestamp );
    }

void Client::killProcess( bool ask, Time timestamp )
    {
    if( process_killer != NULL )
        return;
    Q_ASSERT( !ask || timestamp != CurrentTime );
    QByteArray machine = wmClientMachine( true );
    pid_t pid = info->pid();
    if( pid <= 0 || machine.isEmpty()) // needed properties missing
        return;
    kDebug( 1212 ) << "Kill process:" << pid << "(" << machine << ")";
    if( !ask )
        {
        if( machine != "localhost" )
            {
                  QStringList lst;
                  lst << machine << "kill" << QString::number( pid );
                  QProcess::startDetached("xon",lst);
            }
        else
            ::kill( pid, SIGTERM );
        }
    else
        {
        process_killer = new QProcess( this );
        connect( process_killer, SIGNAL( error( QProcess::ProcessError )), SLOT( processKillerExited()));
        connect( process_killer, SIGNAL( finished( int, QProcess::ExitStatus )), SLOT( processKillerExited()));
        process_killer->start( KStandardDirs::findExe( "kwin_killer_helper" ),
            QStringList() << "--pid" << QByteArray().setNum( (unsigned)pid ) << "--hostname" << machine
            << "--windowname" << caption()
            << "--applicationname" << resourceClass()
            << "--wid" << QString::number( window() )
            << "--timestamp" << QString::number( timestamp ));
        }
    }

void Client::processKillerExited()
    {
    kDebug( 1212 ) << "Killer exited";
    delete process_killer;
    process_killer = NULL;
    }

void Client::setSkipTaskbar( bool b, bool from_outside )
    {
    int was_wants_tab_focus = wantsTabFocus();
    if( from_outside )
        {
        b = rules()->checkSkipTaskbar( b );
        original_skip_taskbar = b;
        }
    if ( b == skipTaskbar() )
        return;
    skip_taskbar = b;
    info->setState( b?NET::SkipTaskbar:0, NET::SkipTaskbar );
    updateWindowRules();
    if( was_wants_tab_focus != wantsTabFocus())
        workspace()->updateFocusChains( this,
            isActive() ? Workspace::FocusChainMakeFirst : Workspace::FocusChainUpdate );
    }

void Client::setSkipPager( bool b )
    {
    b = rules()->checkSkipPager( b );
    if ( b == skipPager() )
        return;
    skip_pager = b;
    info->setState( b?NET::SkipPager:0, NET::SkipPager );
    updateWindowRules();
    }

void Client::setModal( bool m )
    { // Qt-3.2 can have even modal normal windows :(
    if( modal == m )
        return;
    modal = m;
    if( !modal )
        return;
    // changing modality for a mapped window is weird (?)
    // _NET_WM_STATE_MODAL should possibly rather be _NET_WM_WINDOW_TYPE_MODAL_DIALOG
    }

void Client::setDesktop( int desktop )
    {
    if( desktop != NET::OnAllDesktops ) // do range check
        desktop = qMax( 1, qMin( workspace()->numberOfDesktops(), desktop ));
    desktop = rules()->checkDesktop( desktop );
    if( desk == desktop )
        return;
    int was_desk = desk;
    desk = desktop;
    info->setDesktop( desktop );
    if(( was_desk == NET::OnAllDesktops ) != ( desktop == NET::OnAllDesktops ))
        { // onAllDesktops changed
        if ( isShown( true ))
            Notify::raise( isOnAllDesktops() ? Notify::OnAllDesktops : Notify::NotOnAllDesktops );
        workspace()->updateOnAllDesktopsOfTransients( this );
        }
    if( decoration != NULL )
        decoration->desktopChange();
    workspace()->updateFocusChains( this, Workspace::FocusChainMakeFirst );
    updateVisibility();
    updateWindowRules();
    }

/*!
  Returns the virtual desktop within the workspace() the client window
  is located in, 0 if it isn't located on any special desktop (not mapped yet),
  or NET::OnAllDesktops. Do not use desktop() directly, use
  isOnDesktop() instead.
 */
int Client::desktop() const
    {
    return desk;
    }

void Client::setOnAllDesktops( bool b )
    {
    if(( b && isOnAllDesktops())
        || ( !b && !isOnAllDesktops()))
        return;
    if( b )
        setDesktop( NET::OnAllDesktops );
    else
        setDesktop( workspace()->currentDesktop());
    }

// performs activation and/or raising of the window
void Client::takeActivity( int flags, bool handled, allowed_t )
    {
    if( !handled || !Ptakeactivity )
        {
        if( flags & ActivityFocus )
            takeFocus( Allowed );
        if( flags & ActivityRaise )
            workspace()->raiseClient( this );
        return;
        }

#ifndef NDEBUG
    static Time previous_activity_timestamp;
    static Client* previous_client;
#if 0
    if( previous_activity_timestamp == xTime() && previous_client != this )
        {
        kDebug( 1212 ) << "Repeated use of the same X timestamp for activity";
        kDebug( 1212 ) << kBacktrace();
        }
#endif
    previous_activity_timestamp = xTime();
    previous_client = this;
#endif
    workspace()->sendTakeActivity( this, xTime(), flags );
    }

// performs the actual focusing of the window using XSetInputFocus and WM_TAKE_FOCUS
void Client::takeFocus( allowed_t )
    {
#ifndef NDEBUG
    static Time previous_focus_timestamp;
    static Client* previous_client;
#if 0
    if( previous_focus_timestamp == xTime() && previous_client != this )
        {
        kDebug( 1212 ) << "Repeated use of the same X timestamp for focus";
        kDebug( 1212 ) << kBacktrace();
        }
#endif
    previous_focus_timestamp = xTime();
    previous_client = this;
#endif
    if ( rules()->checkAcceptFocus( input ))
        {
        XSetInputFocus( display(), window(), RevertToPointerRoot, xTime() );
        }
    if ( Ptakefocus )
        sendClientMessage(window(), atoms->wm_protocols, atoms->wm_take_focus);
    workspace()->setShouldGetFocus( this );
    }

/*!
  Returns whether the window provides context help or not. If it does,
  you should show a help menu item or a help button like '?' and call
  contextHelp() if this is invoked.

  \sa contextHelp()
 */
bool Client::providesContextHelp() const
    {
    return Pcontexthelp;
    }


/*!
  Invokes context help on the window. Only works if the window
  actually provides context help.

  \sa providesContextHelp()
 */
void Client::showContextHelp()
    {
    if ( Pcontexthelp ) 
        {
        sendClientMessage(window(), atoms->wm_protocols, atoms->net_wm_context_help);
        QWhatsThis::enterWhatsThisMode(); // SELI?
        }
    }


/*!
  Fetches the window's caption (WM_NAME property). It will be
  stored in the client's caption().
 */
void Client::fetchName()
    {
    setCaption( readName());
    }

QString Client::readName() const
    {
    if ( info->name() && info->name()[ 0 ] != '\0' ) 
        return QString::fromUtf8( info->name() );
    else 
        return KWindowSystem::readNameProperty( window(), XA_WM_NAME );
    }
    
KWIN_COMPARE_PREDICATE( FetchNameInternalPredicate, Client, const Client*, (!cl->isSpecialWindow() || cl->isToolbar()) && cl != value && cl->caption() == value->caption());

void Client::setCaption( const QString& _s, bool force )
    {
	QString s = _s;
    if ( s != cap_normal || force ) 
        {
        bool reset_name = force;
        for( int i = 0;
             i < s.length();
             ++i )
            if( !s[ i ].isPrint())
                s[ i ] = QChar( ' ' );
        cap_normal = s;
        bool was_suffix = ( !cap_suffix.isEmpty());
        QString machine_suffix;
        if( wmClientMachine( false ) != "localhost" && !isLocalMachine( wmClientMachine( false )))
            machine_suffix = " <@" + wmClientMachine( true ) + '>';
        QString shortcut_suffix = !shortcut().isEmpty() ? ( " {" + shortcut().toString() + '}' ) : QString();
        cap_suffix = machine_suffix + shortcut_suffix;
        if ( ( !isSpecialWindow() || isToolbar()) && workspace()->findClient( FetchNameInternalPredicate( this ))) 
            {
            int i = 2;
            do 
                {
                cap_suffix = machine_suffix + " <" + QString::number(i) + '>' + shortcut_suffix;
                i++;
                } while ( workspace()->findClient( FetchNameInternalPredicate( this )));
            info->setVisibleName( caption().toUtf8() );
            reset_name = false;
            }
        if(( was_suffix && cap_suffix.isEmpty()
            || reset_name )) // if it was new window, it may have old value still set, if the window is reused
            {
            info->setVisibleName( "" ); // remove
            info->setVisibleIconName( "" ); // remove
            }
        else if( !cap_suffix.isEmpty() && !cap_iconic.isEmpty()) // keep the same suffix in iconic name if it's set
            info->setVisibleIconName( ( cap_iconic + cap_suffix ).toUtf8() );

        if( isManaged() && decoration != NULL )
                decoration->captionChange();
        }
    }

void Client::updateCaption()
    {
    setCaption( cap_normal, true );
    }

void Client::fetchIconicName()
    {
    QString s;
    if ( info->iconName() && info->iconName()[ 0 ] != '\0' ) 
        s = QString::fromUtf8( info->iconName() );
    else 
        s = KWindowSystem::readNameProperty( window(), XA_WM_ICON_NAME );
    if ( s != cap_iconic ) 
        {
	bool was_set = !cap_iconic.isEmpty();
        cap_iconic = s;
        if( !cap_suffix.isEmpty())
	    {
	    if( !cap_iconic.isEmpty()) // keep the same suffix in iconic name if it's set
        	info->setVisibleIconName( ( s + cap_suffix ).toUtf8() );
	    else if( was_set )
		info->setVisibleIconName( "" ); //remove
	    }
        }
    }

/*!\reimp
 */
QString Client::caption( bool full ) const
    {
    return full ? cap_normal + cap_suffix : cap_normal;
    }

void Client::getWMHints()
    {
    XWMHints *hints = XGetWMHints(display(), window() );
    input = true;
    window_group = None;
    urgency = false;
    if ( hints )
        {
        if( hints->flags & InputHint )
            input = hints->input;
        if( hints->flags & WindowGroupHint )
            window_group = hints->window_group;
        urgency = ( hints->flags & UrgencyHint ) ? true : false; // true/false needed, it's uint bitfield
        XFree( (char*)hints );
        }
    checkGroup();
    updateUrgency();
    updateAllowedActions(); // group affects isMinimizable()
    }

void Client::getMotifHints()
    {
    bool mnoborder, mresize, mmove, mminimize, mmaximize, mclose;
    Motif::readFlags( client, mnoborder, mresize, mmove, mminimize, mmaximize, mclose );
    if( mnoborder )
        {
        noborder = true;
        app_noborder =  true;
        }
    if( !hasNETSupport()) // NETWM apps should set type and size constraints
        {
        motif_may_resize = mresize; // this should be set using minsize==maxsize, but oh well
        motif_may_move = mmove;
        }
    else
        motif_may_resize = motif_may_move = true;
    // mminimize; - ignore, bogus - e.g. shading or sending to another desktop is "minimizing" too
    // mmaximize; - ignore, bogus - maximizing is basically just resizing
    motif_may_close = mclose; // motif apps like to crash when they set this hint and WM closes them anyway
    if( isManaged())
        updateDecoration( true ); // check if noborder state has changed
    }

void Client::readIcons( Window win, QPixmap* icon, QPixmap* miniicon )
    {    
    // get the icons, allow scaling
    if( icon != NULL )
        *icon = KWindowSystem::icon( win, 32, 32, true, KWindowSystem::NETWM | KWindowSystem::WMHints );
    if( miniicon != NULL )
        if( icon == NULL || !icon->isNull())
            *miniicon = KWindowSystem::icon( win, 16, 16, true, KWindowSystem::NETWM | KWindowSystem::WMHints );
        else
            *miniicon = QPixmap();
    }

void Client::getIcons()
    {
    // first read icons from the window itself
    readIcons( window(), &icon_pix, &miniicon_pix );
    if( icon_pix.isNull())
        { // then try window group
        icon_pix = group()->icon();
        miniicon_pix = group()->miniIcon();
        }
    if( icon_pix.isNull() && isTransient())
        { // then mainclients
        ClientList mainclients = mainClients();
        for( ClientList::ConstIterator it = mainclients.begin();
             it != mainclients.end() && icon_pix.isNull();
             ++it )
            {
            icon_pix = (*it)->icon();
            miniicon_pix = (*it)->miniIcon();
            }
        }
    if( icon_pix.isNull())
        { // and if nothing else, load icon from classhint or xapp icon
        icon_pix = KWindowSystem::icon( window(), 32, 32, true, KWindowSystem::ClassHint | KWindowSystem::XApp );
        miniicon_pix = KWindowSystem::icon( window(), 16, 16, true, KWindowSystem::ClassHint | KWindowSystem::XApp );
        }
    if( isManaged() && decoration != NULL )
        decoration->iconChange();
    }

void Client::getWindowProtocols()
    {
    Atom *p;
    int i,n;

    Pdeletewindow = 0;
    Ptakefocus = 0;
    Ptakeactivity = 0;
    Pcontexthelp = 0;
    Pping = 0;

    if (XGetWMProtocols(display(), window(), &p, &n))
        {
        for (i = 0; i < n; i++)
            if (p[i] == atoms->wm_delete_window)
                Pdeletewindow = 1;
            else if (p[i] == atoms->wm_take_focus)
                Ptakefocus = 1;
            else if (p[i] == atoms->net_wm_take_activity)
                Ptakeactivity = 1;
            else if (p[i] == atoms->net_wm_context_help)
                Pcontexthelp = 1;
            else if (p[i] == atoms->net_wm_ping)
                Pping = 1;
        if (n>0)
            XFree(p);
        }
    }

void Client::getSyncCounter() 
{
#ifdef HAVE_XSYNC
    if( !Extensions::syncAvailable())
        return;

    Atom retType;
    unsigned long nItemRet;
    unsigned long byteRet;
    int formatRet;
    unsigned char* propRet;
    int ret = XGetWindowProperty( display(), window(), atoms->net_wm_sync_request_counter,
        0, 1, false, XA_CARDINAL, &retType, &formatRet, &nItemRet, &byteRet, &propRet );
    
    if( ret == Success && formatRet == 32 )
        {
        sync_counter = *(long*)propRet;
        XSyncIntToValue( &sync_counter_value, 0 );
	XSyncValue zero;
	XSyncIntToValue( &zero, 0 );
	XSyncSetCounter( display(), sync_counter, zero );
        if( sync_alarm == None )
            {
            XSyncAlarmAttributes attrs;
            attrs.trigger.counter = sync_counter;
            attrs.trigger.value_type = XSyncRelative;
            attrs.trigger.test_type = XSyncPositiveTransition;
            XSyncIntToValue( &attrs.trigger.wait_value, 1 );
            XSyncIntToValue( &attrs.delta, 1 );
            sync_alarm = XSyncCreateAlarm( display(),
                XSyncCACounter | XSyncCAValueType | XSyncCATestType | XSyncCADelta | XSyncCAValue,
                &attrs );  
            }
        }
#endif
}

// send the client a _NET_SYNC_REQUEST
void Client::sendSyncRequest()
{
#ifdef HAVE_XSYNC
    if( sync_counter == None )
        return;

    // we increment before the notify so that after the notify
    // syncCounterSerial will equal the value we are expecting
    // in the acknowledgement
    int overflow;
    XSyncValue one;
    XSyncIntToValue( &one, 1 );
#undef XSyncValueAdd // it causes a warning :-/
    XSyncValueAdd( &sync_counter_value, sync_counter_value, one, &overflow );

    // send the message to client
    XEvent ev;
    ev.xclient.type = ClientMessage;
    ev.xclient.window = window();
    ev.xclient.format = 32;
    ev.xclient.message_type = atoms->wm_protocols;
    ev.xclient.data.l[ 0 ] = atoms->net_wm_sync_request;
    ev.xclient.data.l[ 1 ] = xTime();
    ev.xclient.data.l[ 2 ] = XSyncValueLow32( sync_counter_value );
    ev.xclient.data.l[ 3 ] = XSyncValueHigh32( sync_counter_value );
    ev.xclient.data.l[ 4 ] = 0;
    XSendEvent( display(), window(), False, NoEventMask, &ev );
    XSync(display(),false);
#endif
}


bool Client::wantsTabFocus() const
    {
    return ( isNormalWindow() || isDialog()) && wantsInput();
    }


bool Client::wantsInput() const
    {
    return rules()->checkAcceptFocus( input || Ptakefocus );
    }

bool Client::isSpecialWindow() const
    {
    return isDesktop() || isDock() || isSplash() || isTopMenu()
        || isToolbar(); // TODO
    }

/*!
  Sets an appropriate cursor shape for the logical mouse position \a m

 */
void Client::updateCursor()
    {
    Position m = mode;
    if( !isResizable() || isShade())
        m = PositionCenter;
    QCursor c;
    switch( m )
        {
        case PositionTopLeft:
        case PositionBottomRight:
            c = Qt::SizeFDiagCursor;
            break;
        case PositionBottomLeft:
        case PositionTopRight:
            c = Qt::SizeBDiagCursor;
            break;
        case PositionTop:
        case PositionBottom:
            c = Qt::SizeVerCursor;
            break;
        case PositionLeft:
        case PositionRight:
            c = Qt::SizeHorCursor;
            break;
        default:
            if( moveResizeMode )
                c = Qt::SizeAllCursor;
            else
                c = Qt::ArrowCursor;
            break;
        }
    if( c.handle() == cursor.handle())
        return;
    cursor = c;
    if( decoration != NULL )
        decoration->widget()->setCursor( cursor );
    XDefineCursor( display(), frameId(), cursor.handle());
    if( moveResizeMode ) // XDefineCursor doesn't change cursor if there's pointer grab active
        XChangeActivePointerGrab( display(),
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask,
            cursor.handle(), xTime());
    }

Client::Position Client::mousePosition( const QPoint& p ) const
    {
    if( decoration != NULL )
        return decoration->mousePosition( p );
    return PositionCenter;
    }

void Client::updateAllowedActions( bool force )
    {
    if( !isManaged() && !force )
        return;
    unsigned long old_allowed_actions = allowed_actions;
    allowed_actions = 0;
    if( isMovable())
        allowed_actions |= NET::ActionMove;
    if( isResizable())
        allowed_actions |= NET::ActionResize;
    if( isMinimizable())
        allowed_actions |= NET::ActionMinimize;
    if( isShadeable())
        allowed_actions |= NET::ActionShade;
    // sticky state not supported
    if( isMaximizable())
        allowed_actions |= NET::ActionMax;
    if( userCanSetFullScreen())
        allowed_actions |= NET::ActionFullScreen;
    allowed_actions |= NET::ActionChangeDesktop; // always (pagers shouldn't show Docks etc.)
    if( isCloseable())
        allowed_actions |= NET::ActionClose;
    if( old_allowed_actions == allowed_actions )
        return;
    // TODO this could be delayed and compressed - it's only for pagers etc. anyway
    info->setAllowedActions( allowed_actions );
    // TODO this should also tell the decoration, so that it can update the buttons
    }

void Client::autoRaise()
    {
    workspace()->raiseClient( this );
    cancelAutoRaise();
    }
    
void Client::cancelAutoRaise()
    {
    delete autoRaiseTimer;
    autoRaiseTimer = 0;
    }

void Client::debug( kdbgstream& stream ) const
    {
    stream << "\'ID:" << window() << ";WMCLASS:" << resourceClass() << ":" << resourceName() << ";Caption:" << caption() << "\'";
    }

QPixmap * kwin_get_menu_pix_hack()
    {
    static QPixmap p;
    if ( p.isNull() )
        p = SmallIcon( "bx2" );
    return &p;
    }

} // namespace

#include "client.moc"
