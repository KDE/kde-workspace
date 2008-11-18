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

//#define QT_CLEAN_NAMESPACE

#include "workspace.h"

#include <kapplication.h>
#include <kstartupinfo.h>
#include <fixx11h.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <QRegExp>
#include <QPainter>
#include <QBitmap>
#include <QClipboard>
#include <kmenubar.h>
#include <kprocess.h>
#include <kglobalaccel.h>
#include <QDesktopWidget>
#include <QToolButton>
#include <kactioncollection.h>
#include <kaction.h>
#include <kconfiggroup.h>
#include <QtDBus/QtDBus>

#include "plugins.h"
#include "client.h"
#include "popupinfo.h"
#include "tabbox.h"
#include "atoms.h"
#include "placement.h"
#include "notifications.h"
#include "group.h"
#include "rules.h"
#include "kwinadaptor.h"
#include "unmanaged.h"
#include "scene.h"
#include "deleted.h"
#include "effects.h"
#include "kdecorationfactory.h"

#include <X11/extensions/shape.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>
#include <QX11Info>
#include <stdio.h>
#include <kauthorized.h>
#include <ktoolinvocation.h>
#include <kglobalsettings.h>

namespace KWin
{

extern int screen_number;

Workspace *Workspace::_self = 0;

// Rikkus: This class is too complex. It needs splitting further.
// It's a nightmare to understand, especially with so few comments :(

// Matthias: Feel free to ask me questions about it. Feel free to add
// comments. I dissagree that further splittings makes it easier. 2500
// lines are not too much. It's the task that is complex, not the
// code.
Workspace::Workspace( bool restore )
  : QObject           (0),
    current_desktop   (0),
    number_of_desktops(0),
    active_popup( NULL ),
    active_popup_client( NULL ),
    temporaryRulesMessages( "_KDE_NET_WM_TEMPORARY_RULES", NULL, false ),
    rules_updates_disabled( false ),
    active_client     (0),
    last_active_client     (0),
    most_recently_raised (0),
    movingClient(0),
    pending_take_activity ( NULL ),
    active_screen     (0),
    delayfocus_client (0),
    force_restacking( false ),
    showing_desktop( false ),
    block_showing_desktop( 0 ),
    was_user_interaction (false),
    session_saving    (false),
    control_grab      (false),
    tab_grab          (false),
    mouse_emulation   (false),
    block_focus       (0),
    tab_box           (0),
    popupinfo         (0),
    popup             (0),
    advanced_popup    (0),
    trans_popup       (0),
    desk_popup        (0),
    keys              (0),
    client_keys       ( NULL ),
    client_keys_dialog ( NULL ),
    client_keys_client ( NULL ),
    disable_shortcuts_keys ( NULL ),
    global_shortcuts_disabled( false ),
    global_shortcuts_disabled_for_client( false ),
    workspaceInit     (true),
    startup(0),
    layoutOrientation(Qt::Vertical),
    layoutX(-1),
    layoutY(2),
    managing_topmenus( false ),
    topmenu_selection( NULL ),
    topmenu_watcher( NULL ),
    topmenu_height( 0 ),
    topmenu_space( NULL ),
    set_active_client_recursion( 0 ),
    block_stacking_updates( 0 ),
    forced_global_mouse_grab( false ),
    cm_selection( NULL ),
    compositingSuspended( false ),
    compositeRate( 0 ),
    overlay( None ),
    overlay_visible( true ),
    overlay_shown( false ),
    transSlider( NULL ),
    transButton( NULL )
    {
    (void) new KWinAdaptor( this );
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/KWin", this);
    dbus.connect(QString(), "/KWin", "org.kde.KWin", "reloadConfig", this, SLOT(slotReloadConfig()));
    dbus.connect(QString(), "/KWin", "org.kde.KWin", "reinitCompositing", this, SLOT(slotReinitCompositing()));
    _self = this;
    mgr = new PluginMgr;
    QX11Info info;
    default_colormap = DefaultColormap(display(), info.screen() );
    installed_colormap = default_colormap;

    for( int i = 0;
         i < ELECTRIC_COUNT;
         ++i )
        {
        electric_reserved[ i ] = 0;
        electric_windows[ i ] = None;
        }

    connect( &temporaryRulesMessages, SIGNAL( gotMessage( const QString& )),
        this, SLOT( gotTemporaryRulesMessage( const QString& )));
    connect( &rulesUpdatedTimer, SIGNAL( timeout()), this, SLOT( writeWindowRules()));

    updateXTime(); // needed for proper initialization of user_time in Client ctor

    delayFocusTimer = 0;

    if ( restore )
      loadSessionInfo();

    loadWindowRules();

    (void) QApplication::desktop(); // trigger creation of desktop widget

    // call this before XSelectInput() on the root window
    startup = new KStartupInfo(
        KStartupInfo::DisableKWinModule | KStartupInfo::AnnounceSilenceChanges, this );

    // select windowmanager privileges
    XSelectInput(display(), rootWindow(),
                 KeyPressMask |
                 PropertyChangeMask |
                 ColormapChangeMask |
                 SubstructureRedirectMask |
                 SubstructureNotifyMask |
                 FocusChangeMask | // for NotifyDetailNone
                 ExposureMask
                 );

    Extensions::init();
    setupCompositing();

    // compatibility
    long data = 1;

    XChangeProperty(
      display(),
      rootWindow(),
      atoms->kwin_running,
      atoms->kwin_running,
      32,
      PropModeAppend,
      (unsigned char*) &data,
      1
    );

    client_keys = new KActionCollection( this );
    initShortcuts();
    tab_box = new TabBox( this );
    popupinfo = new PopupInfo( this );

    init();

    connect( kapp->desktop(), SIGNAL( resized( int )), SLOT( desktopResized()));
    }

void Workspace::init()
    {
    if( options->electricBorders() == Options::ElectricAlways )
        reserveElectricBorderSwitching( true  );
    updateElectricBorders();

// not used yet
//     topDock = 0L;
//     maximizedWindowCounter = 0;

    supportWindow = new QWidget( NULL, Qt::X11BypassWindowManagerHint );
    XLowerWindow( display(), supportWindow->winId()); // see usage in layers.cpp

    XSetWindowAttributes attr;
    attr.override_redirect = 1;
    null_focus_window = XCreateWindow( display(), rootWindow(), -1,-1, 1, 1, 0, CopyFromParent,
        InputOnly, CopyFromParent, CWOverrideRedirect, &attr );
    XMapWindow(display(), null_focus_window);

    unsigned long protocols[ 5 ] =
        {
        NET::Supported |
        NET::SupportingWMCheck |
        NET::ClientList |
        NET::ClientListStacking |
        NET::DesktopGeometry |
        NET::NumberOfDesktops |
        NET::CurrentDesktop |
        NET::ActiveWindow |
        NET::WorkArea |
        NET::CloseWindow |
        NET::DesktopNames |
        NET::WMName |
        NET::WMVisibleName |
        NET::WMDesktop |
        NET::WMWindowType |
        NET::WMState |
        NET::WMStrut |
        NET::WMIconGeometry |
        NET::WMIcon |
        NET::WMPid |
        NET::WMMoveResize |
        NET::WMFrameExtents |
        NET::WMPing
        ,
        NET::NormalMask |
        NET::DesktopMask |
        NET::DockMask |
        NET::ToolbarMask |
        NET::MenuMask |
        NET::DialogMask |
        NET::OverrideMask |
        NET::TopMenuMask |
        NET::UtilityMask |
        NET::SplashMask |
    // No compositing window types here unless we support them also as managed window types
        0
        ,
        NET::Modal |
//        NET::Sticky |  // large desktops not supported (and probably never will be)
        NET::MaxVert |
        NET::MaxHoriz |
        NET::Shaded |
        NET::SkipTaskbar |
        NET::KeepAbove |
//        NET::StaysOnTop |  the same like KeepAbove
        NET::SkipPager |
        NET::Hidden |
        NET::FullScreen |
        NET::KeepBelow |
        NET::DemandsAttention |
        0
        ,
        NET::WM2UserTime |
        NET::WM2StartupId |
        NET::WM2AllowedActions |
        NET::WM2RestackWindow |
        NET::WM2MoveResizeWindow |
        NET::WM2ExtendedStrut |
        NET::WM2KDETemporaryRules |
        NET::WM2ShowingDesktop |
        NET::WM2DesktopLayout |
        NET::WM2FullPlacement |
        0
        ,
        NET::ActionMove |
        NET::ActionResize |
        NET::ActionMinimize |
        NET::ActionShade |
//        NET::ActionStick | // Sticky state is not supported
        NET::ActionMaxVert |
        NET::ActionMaxHoriz |
        NET::ActionFullScreen |
        NET::ActionChangeDesktop |
        NET::ActionClose |
        0
        ,
        };

	QX11Info info;
    rootInfo = new RootInfo( this, display(), supportWindow->winId(), "KWin",
        protocols, 5, info.screen() );

    loadDesktopSettings();
    updateDesktopLayout();
    // extra NETRootInfo instance in Client mode is needed to get the values of the properties
    NETRootInfo client_info( display(), NET::ActiveWindow | NET::CurrentDesktop );
    int initial_desktop;
    if( !kapp->isSessionRestored())
        initial_desktop = client_info.currentDesktop();
    else
        {
        KConfigGroup group( kapp->sessionConfig(), "Session" );
        initial_desktop = group.readEntry( "desktop", 1 );
        }
    if( !setCurrentDesktop( initial_desktop ))
        setCurrentDesktop( 1 );

    // now we know how many desktops we'll, thus, we initialize the positioning object
    initPositioning = new Placement(this);

    reconfigureTimer.setSingleShot( true );
    updateToolWindowsTimer.setSingleShot( true );

    connect(&reconfigureTimer, SIGNAL(timeout()), this,
            SLOT(slotReconfigure()));
    connect( &updateToolWindowsTimer, SIGNAL( timeout()), this, SLOT( slotUpdateToolWindows()));
    connect( &compositeTimer, SIGNAL( timeout()), SLOT( performCompositing()));

    connect(KGlobalSettings::self(), SIGNAL(appearanceChanged()), this,
            SLOT(slotReconfigure()));
    connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), this,
            SLOT(slotSettingsChanged(int)));
    connect(KGlobalSettings::self(), SIGNAL(blockShortcuts(int)), this,
            SLOT(slotBlockShortcuts(int)));

    active_client = NULL;
    rootInfo->setActiveWindow( None );
    focusToNull();
    if( !kapp->isSessionRestored())
        ++block_focus; // because it will be set below

    char nm[ 100 ];
    sprintf( nm, "_KDE_TOPMENU_OWNER_S%d", DefaultScreen( display()));
    Atom topmenu_atom = XInternAtom( display(), nm, False );
    topmenu_selection = new KSelectionOwner( topmenu_atom );
    topmenu_watcher = new KSelectionWatcher( topmenu_atom );
// TODO grabXServer(); - where exactly put this? topmenu selection claiming down belong must be before

        { // begin updates blocker block
        StackingUpdatesBlocker blocker( this );

        if( options->topMenuEnabled() && topmenu_selection->claim( false ))
            setupTopMenuHandling(); // this can call updateStackingOrder()
        else
            lostTopMenuSelection();

        unsigned int i, nwins;
        Window root_return, parent_return, *wins;
        XQueryTree(display(), rootWindow(), &root_return, &parent_return, &wins, &nwins);
        for (i = 0; i < nwins; i++)
            {
            XWindowAttributes attr;
            XGetWindowAttributes(display(), wins[i], &attr);
            if (attr.override_redirect )
                {
                createUnmanaged( wins[ i ] );
                continue;
                }
            if( topmenu_space && topmenu_space->winId() == wins[ i ] )
                continue;
            if (attr.map_state != IsUnmapped)
                createClient( wins[i], true );
            }
        if ( wins )
            XFree((void *) wins);
    // propagate clients, will really happen at the end of the updates blocker block
        updateStackingOrder( true );

        updateClientArea();

    // NETWM spec says we have to set it to (0,0) if we don't support it
        NETPoint* viewports = new NETPoint[ number_of_desktops ];
        rootInfo->setDesktopViewport( number_of_desktops, *viewports );
        delete[] viewports;
        QRect geom = QApplication::desktop()->geometry();
        NETSize desktop_geometry;
        desktop_geometry.width = geom.width();
        desktop_geometry.height = geom.height();
        rootInfo->setDesktopGeometry( -1, desktop_geometry );
        setShowingDesktop( false );

        } // end updates blocker block

    Client* new_active_client = NULL;
    if( !kapp->isSessionRestored())
        {
        --block_focus;
        new_active_client = findClient( WindowMatchPredicate( client_info.activeWindow()));
        }
    if( new_active_client == NULL
        && activeClient() == NULL && should_get_focus.count() == 0 ) // no client activated in manage()
        {
        if( new_active_client == NULL )
            new_active_client = topClientOnDesktop( currentDesktop());
        if( new_active_client == NULL && !desktops.isEmpty() )
            new_active_client = findDesktop( true, currentDesktop());
        }
    if( new_active_client != NULL )
        activateClient( new_active_client );
    // SELI TODO this won't work with unreasonable focus policies,
    // and maybe in rare cases also if the selected client doesn't
    // want focus
    workspaceInit = false;
// TODO ungrabXServer()
    }

Workspace::~Workspace()
    {
    finishCompositing();
    blockStackingUpdates( true );
// TODO    grabXServer();
    // use stacking_order, so that kwin --replace keeps stacking order
    for( ClientList::ConstIterator it = stacking_order.begin();
         it != stacking_order.end();
         ++it )
        {
	// only release the window
        (*it)->releaseWindow( true );
        // No removeClient() is called, it does more than just removing.
        // However, remove from some lists to e.g. prevent performTransiencyCheck()
        // from crashing.
        clients.removeAll( *it );
        desktops.removeAll( *it );
        }
    for( UnmanagedList::ConstIterator it = unmanaged.begin();
         it != unmanaged.end();
         ++it )
        (*it)->release();
    delete tab_box;
    delete popupinfo;
    discardPopup();
    XDeleteProperty(display(), rootWindow(), atoms->kwin_running);

    writeWindowRules();
    KGlobal::config()->sync();

    delete rootInfo;
    delete supportWindow;
    delete mgr;
    delete startup;
    delete initPositioning;
    delete topmenu_watcher;
    delete topmenu_selection;
    delete topmenu_space;
    delete client_keys_dialog;
    while( !rules.isEmpty())
        {
        delete rules.front();
        rules.pop_front();
        }
	foreach ( SessionInfo* s, session )
		delete s;
    XDestroyWindow( display(), null_focus_window );
// TODO    ungrabXServer();
    _self = 0;
    }

Client* Workspace::createClient( Window w, bool is_mapped )
    {
    StackingUpdatesBlocker blocker( this );
    Client* c = new Client( this );
    if( !c->manage( w, is_mapped ))
        {
        Client::deleteClient( c, Allowed );
        return NULL;
        }
    addClient( c, Allowed );
    if( scene )
        scene->windowAdded( c );
    if( effects )
        static_cast<EffectsHandlerImpl*>(effects)->windowAdded( c->effectWindow());
    return c;
    }

Unmanaged* Workspace::createUnmanaged( Window w )
    {
    if( w == overlay )
        return NULL;
    Unmanaged* c = new Unmanaged( this );
    if( !c->track( w ))
        {
        Unmanaged::deleteUnmanaged( c, Allowed );
        return NULL;
        }
    addUnmanaged( c, Allowed );
    if( scene )
        scene->windowAdded( c );
    if( effects )
        static_cast<EffectsHandlerImpl*>(effects)->windowAdded( c->effectWindow());
    return c;
    }

void Workspace::addClient( Client* c, allowed_t )
    {
    Group* grp = findGroup( c->window());
    if( grp != NULL )
        grp->gotLeader( c );

    if ( c->isDesktop() )
        {
        desktops.append( c );
        if( active_client == NULL && should_get_focus.isEmpty() && c->isOnCurrentDesktop())
            requestFocus( c ); // CHECKME? make sure desktop is active after startup if there's no other window active
        }
    else
        {
        updateFocusChains( c, FocusChainUpdate ); // add to focus chain if not already there
        clients.append( c );
        }
    if( !unconstrained_stacking_order.contains( c ))
        unconstrained_stacking_order.append( c ); // raise if it hasn't got any stacking position yet
    if( !stacking_order.contains( c )) // it'll be updated later, and updateToolWindows() requires
        stacking_order.append( c );    // c to be in stacking_order
    if( c->isTopMenu())
        addTopMenu( c );
    updateClientArea(); // this cannot be in manage(), because the client got added only now
    updateClientLayer( c );
    if( c->isDesktop())
        {
        raiseClient( c );
	// if there's no active client, make this desktop the active one
        if( activeClient() == NULL && should_get_focus.count() == 0 )
            activateClient( findDesktop( true, currentDesktop()));
        }
    c->checkActiveModal();
    checkTransients( c->window()); // SELI does this really belong here?
    updateStackingOrder( true ); // propagate new client
    if( c->isUtility() || c->isMenu() || c->isToolbar())
        updateToolWindows( true );
    checkNonExistentClients();
    if( tab_grab )
        tab_box->reset( true );
    }

void Workspace::addUnmanaged( Unmanaged* c, allowed_t )
    {
    unmanaged.append( c );
    }

/*
  Destroys the client \a c
 */
void Workspace::removeClient( Client* c, allowed_t )
    {
    if (c == active_popup_client)
        closeActivePopup();

    if( client_keys_client == c )
        setupWindowShortcutDone( false );
    if( !c->shortcut().isEmpty())
        {
        c->setShortcut( QString() ); // remove from client_keys
        clientShortcutUpdated( c ); // needed, since this is otherwise delayed by setShortcut() and wouldn't run
        }

    if( c->isDialog())
        Notify::raise( Notify::TransDelete );
    if( c->isNormalWindow())
        Notify::raise( Notify::Delete );

    if( tab_grab )
        {
        if( tab_box->currentClient() == c )
            tab_box->nextPrev( true );
        }

    Q_ASSERT( clients.contains( c ) || desktops.contains( c ));
    clients.removeAll( c );
    desktops.removeAll( c );
    unconstrained_stacking_order.removeAll( c );
    stacking_order.removeAll( c );
    for( int i = 1;
         i <= numberOfDesktops();
         ++i )
        focus_chain[ i ].removeAll( c );
    global_focus_chain.removeAll( c );
    attention_chain.removeAll( c );
    showing_desktop_clients.removeAll( c );
    if( c->isTopMenu())
        removeTopMenu( c );
    Group* group = findGroup( c->window());
    if( group != NULL )
        group->lostLeader();

    if ( c == most_recently_raised )
        most_recently_raised = 0;
    should_get_focus.removeAll( c );
    Q_ASSERT( c != active_client );
    if ( c == last_active_client )
        last_active_client = 0;
    if( c == pending_take_activity )
        pending_take_activity = NULL;
    if( c == delayfocus_client )
        cancelDelayFocus();

    updateStackingOrder( true );

    if( tab_grab )
        tab_box->reset( true );

    updateClientArea();
    }

void Workspace::removeUnmanaged( Unmanaged* c, allowed_t )
    {
    assert( unmanaged.contains( c ));
    unmanaged.removeAll( c );
    }

void Workspace::addDeleted( Deleted* c, allowed_t )
    {
    assert( !deleted.contains( c ));
    deleted.append( c );
    }

void Workspace::removeDeleted( Deleted* c, allowed_t )
    {
    assert( deleted.contains( c ));
    if( scene )
        scene->windowDeleted( c );
    if( effects )
        static_cast<EffectsHandlerImpl*>(effects)->windowDeleted( c->effectWindow());
    deleted.removeAll( c );
    }

void Workspace::updateFocusChains( Client* c, FocusChainChange change )
    {
    if( !c->wantsTabFocus()) // doesn't want tab focus, remove
        {
        for( int i=1;
             i<= numberOfDesktops();
             ++i )
            focus_chain[i].removeAll(c);
        global_focus_chain.removeAll( c );
        return;
        }
    if(c->desktop() == NET::OnAllDesktops)
        { //now on all desktops, add it to focus_chains it is not already in
        for( int i=1; i<= numberOfDesktops(); i++)
            { // making first/last works only on current desktop, don't affect all desktops
            if( i == currentDesktop()
                && ( change == FocusChainMakeFirst || change == FocusChainMakeLast ))
                {
                focus_chain[ i ].removeAll( c );
                if( change == FocusChainMakeFirst )
                    focus_chain[ i ].append( c );
                else
                    focus_chain[ i ].prepend( c );
                }
            else if( !focus_chain[ i ].contains( c ))
                { // add it after the active one
                if( active_client != NULL && active_client != c
                    && !focus_chain[ i ].isEmpty() && focus_chain[ i ].last() == active_client )
                    focus_chain[ i ].insert( focus_chain[ i ].size() - 1, c );
                else
                    focus_chain[ i ].append( c ); // otherwise add as the first one
                }
            }
        }
    else    //now only on desktop, remove it anywhere else
        {
        for( int i=1; i<= numberOfDesktops(); i++)
            {
            if( i == c->desktop())
                {
                if( change == FocusChainMakeFirst )
                    {
                    focus_chain[ i ].removeAll( c );
                    focus_chain[ i ].append( c );
                    }
                else if( change == FocusChainMakeLast )
                    {
                    focus_chain[ i ].removeAll( c );
                    focus_chain[ i ].prepend( c );
                    }
                else if( !focus_chain[ i ].contains( c ))
                    { // add it after the active one
                    if( active_client != NULL && active_client != c
                        && !focus_chain[ i ].isEmpty() && focus_chain[ i ].last() == active_client )
                        focus_chain[ i ].insert( focus_chain[ i ].size() - 1, c );
                    else
                        focus_chain[ i ].append( c ); // otherwise add as the first one
                    }
                }
            else
                focus_chain[ i ].removeAll( c );
            }
        }
    if( change == FocusChainMakeFirst )
        {
        global_focus_chain.removeAll( c );
        global_focus_chain.append( c );
        }
    else if( change == FocusChainMakeLast )
        {
        global_focus_chain.removeAll( c );
        global_focus_chain.prepend( c );
        }
    else if( !global_focus_chain.contains( c ))
        { // add it after the active one
        if( active_client != NULL && active_client != c
            && !global_focus_chain.isEmpty() && global_focus_chain.last() == active_client )
            global_focus_chain.insert( global_focus_chain.size() - 1, c );
        else
            global_focus_chain.append( c ); // otherwise add as the first one
        }
    }

void Workspace::updateCurrentTopMenu()
    {
    if( !managingTopMenus())
        return;
    // toplevel menubar handling
    Client* menubar = 0;
    bool block_desktop_menubar = false;
    if( active_client )
        {
        // show the new menu bar first...
        Client* menu_client = active_client;
        for(;;)
            {
            if( menu_client->isFullScreen())
                block_desktop_menubar = true;
            for( ClientList::ConstIterator it = menu_client->transients().begin();
                 it != menu_client->transients().end();
                 ++it )
                if( (*it)->isTopMenu())
                    {
                    menubar = *it;
                    break;
                    }
            if( menubar != NULL || !menu_client->isTransient())
                break;
            if( menu_client->isModal() || menu_client->transientFor() == NULL )
                break; // don't use mainwindow's menu if this is modal or group transient
            menu_client = menu_client->transientFor();
            }
        if( !menubar )
            { // try to find any topmenu from the application (#72113)
            for( ClientList::ConstIterator it = active_client->group()->members().begin();
                 it != active_client->group()->members().end();
                 ++it )
                if( (*it)->isTopMenu())
                    {
                    menubar = *it;
                    break;
                    }
            }
        }
    if( !menubar && !block_desktop_menubar && options->desktopTopMenu())
        {
        // Find the menubar of the desktop
        Client* desktop = findDesktop( true, currentDesktop());
        if( desktop != NULL )
            {
            for( ClientList::ConstIterator it = desktop->transients().begin();
                 it != desktop->transients().end();
                 ++it )
                if( (*it)->isTopMenu())
                    {
                    menubar = *it;
                    break;
                    }
            }
        // TODO to be cleaned app with window grouping
        // Without qt-copy patch #0009, the topmenu and desktop are not in the same group,
        // thus the topmenu is not transient for it :-/.
        if( menubar == NULL )
            {
            for( ClientList::ConstIterator it = topmenus.begin();
                 it != topmenus.end();
                 ++it )
                if( (*it)->wasOriginallyGroupTransient()) // kdesktop's topmenu has WM_TRANSIENT_FOR
                    {                                     // set pointing to the root window
                    menubar = *it;                        // to recognize it here
                    break;                                // Also, with the xroot hack in kdesktop,
                    }                                     // there's no NET::Desktop window to be transient for
            }
        }

//    kDebug() << "CURRENT TOPMENU:" << menubar << ":" << active_client;
    if ( menubar )
        {
        if( active_client && !menubar->isOnDesktop( active_client->desktop()))
            menubar->setDesktop( active_client->desktop());
        menubar->hideClient( false );
        topmenu_space->hide();
        // make it appear like it's been raised manually - it's in the Dock layer anyway,
        // and not raising it could mess up stacking order of topmenus within one application,
        // and thus break raising of mainclients in raiseClient()
        unconstrained_stacking_order.removeAll( menubar );
        unconstrained_stacking_order.append( menubar );
        }
    else if( !block_desktop_menubar )
        { // no topmenu active - show the space window, so that there's not empty space
        topmenu_space->show();
        }

    // ... then hide the other ones. Avoids flickers.
    for ( ClientList::ConstIterator it = clients.begin(); it != clients.end(); ++it)
        {
        if( (*it)->isTopMenu() && (*it) != menubar )
            (*it)->hideClient( true );
        }
    }


void Workspace::updateToolWindows( bool also_hide )
    {
    // TODO what if Client's transiency/group changes? should this be called too? (I'm paranoid, am I not?)
    if( !options->hideUtilityWindowsForInactive )
        {
        for( ClientList::ConstIterator it = clients.begin();
             it != clients.end();
             ++it )
            (*it)->hideClient( false );
        return;
        }
    const Group* group = NULL;
    const Client* client = active_client;
// Go up in transiency hiearchy, if the top is found, only tool transients for the top mainwindow
// will be shown; if a group transient is group, all tools in the group will be shown
    while( client != NULL )
        {
        if( !client->isTransient())
            break;
        if( client->groupTransient())
            {
            group = client->group();
            break;
            }
        client = client->transientFor();
        }
    // use stacking order only to reduce flicker, it doesn't matter if block_stacking_updates == 0,
    // i.e. if it's not up to date

    // SELI but maybe it should - what if a new client has been added that's not in stacking order yet?
    ClientList to_show, to_hide;
    for( ClientList::ConstIterator it = stacking_order.begin();
         it != stacking_order.end();
         ++it )
        {
        if( (*it)->isUtility() || (*it)->isMenu() || (*it)->isToolbar())
            {
            bool show = true;
            if( !(*it)->isTransient())
                {
                if( (*it)->group()->members().count() == 1 ) // has its own group, keep always visible
                    show = true;
                else if( client != NULL && (*it)->group() == client->group())
                    show = true;
                else
                    show = false;
                }
            else
                {
                if( group != NULL && (*it)->group() == group )
                    show = true;
                else if( client != NULL && client->hasTransient( (*it), true ))
                    show = true;
                else
                    show = false;
                }
            if( !show && also_hide )
                {
                const ClientList mainclients = (*it)->mainClients();
                // don't hide utility windows which are standalone(?) or
                // have e.g. kicker as mainwindow
                if( mainclients.isEmpty())
                    show = true;
                for( ClientList::ConstIterator it2 = mainclients.begin();
                     it2 != mainclients.end();
                     ++it2 )
                    {
                    if( (*it2)->isSpecialWindow())
                        show = true;
                    }
                if( !show )
                    to_hide.append( *it );
                }
            if( show )
                to_show.append( *it );
            }
        } // first show new ones, then hide
	for( int i = to_show.size() - 1;
              i >= 0;
              --i ) //from topmost
        // TODO since this is in stacking order, the order of taskbar entries changes :(
            to_show.at( i )->hideClient( false );
    if( also_hide )
        {
        for( ClientList::ConstIterator it = to_hide.begin();
             it != to_hide.end();
             ++it ) // from bottommost
            (*it)->hideClient( true );
        updateToolWindowsTimer.stop();
        }
    else // setActiveClient() is after called with NULL client, quickly followed
        {    // by setting a new client, which would result in flickering
        updateToolWindowsTimer.start( 50 );
        }
    }

void Workspace::slotUpdateToolWindows()
    {
    updateToolWindows( true );
    }

/*!
  Updates the current colormap according to the currently active client
 */
void Workspace::updateColormap()
    {
    Colormap cmap = default_colormap;
    if ( activeClient() && activeClient()->colormap() != None )
        cmap = activeClient()->colormap();
    if ( cmap != installed_colormap )
        {
        XInstallColormap(display(), cmap );
        installed_colormap = cmap;
        }
    }

void Workspace::slotReloadConfig()
{
  reconfigure();
}

void Workspace::reconfigure()
    {
    reconfigureTimer.start( 200 );
    }


void Workspace::slotSettingsChanged(int category)
    {
    kDebug(1212) << "Workspace::slotSettingsChanged()";
    if( category == KGlobalSettings::SETTINGS_SHORTCUTS )
        readShortcuts();
    }

/*!
  Reread settings
 */
KWIN_PROCEDURE( CheckBorderSizesProcedure, Client, cl->checkBorderSizes( true ) );

void Workspace::slotReconfigure()
    {
    kDebug(1212) << "Workspace::slotReconfigure()";
    reconfigureTimer.stop();

    if( options->electricBorders() == Options::ElectricAlways )
        reserveElectricBorderSwitching( false );

    KGlobal::config()->reparseConfiguration();
    unsigned long changed = options->updateSettings();
    tab_box->reconfigure();
    popupinfo->reconfigure();
    initPositioning->reinitCascading( 0 );
    readShortcuts();
    forEachClient( CheckIgnoreFocusStealingProcedure());
    updateToolWindows( true );

    if( mgr->reset( changed ))
        { // decorations need to be recreated
#if 0 // This actually seems to make things worse now
        QWidget curtain;
        curtain.setBackgroundMode( NoBackground );
        curtain.setGeometry( QApplication::desktop()->geometry() );
        curtain.show();
#endif
        for( ClientList::ConstIterator it = clients.begin();
                it != clients.end();
                ++it )
            {
            (*it)->updateDecoration( true, true );
            }
        mgr->destroyPreviousPlugin();
        }
    else
        {
        forEachClient( CheckBorderSizesProcedure());
        foreach( Client* c, clients )
            c->repaintDecoration();
        }

    if( options->electricBorders() == Options::ElectricAlways )
        reserveElectricBorderSwitching( true );
    updateElectricBorders();

    if( options->topMenuEnabled() && !managingTopMenus())
        {
        if( topmenu_selection->claim( false ))
            setupTopMenuHandling();
        else
            lostTopMenuSelection();
        }
    else if( !options->topMenuEnabled() && managingTopMenus())
        {
        topmenu_selection->release();
        lostTopMenuSelection();
        }
    topmenu_height = 0; // invalidate used menu height
    if( managingTopMenus())
        {
        updateTopMenuGeometry();
        updateCurrentTopMenu();
        }

    if( options->useCompositing && !compositingSuspended )
    {
        setupCompositing();
        if( effects ) // setupCompositing() may fail
            effects->reconfigure();
        addRepaintFull();
    }
    else
        finishCompositing();

    loadWindowRules();
    for( ClientList::Iterator it = clients.begin();
         it != clients.end();
         ++it )
        {
        (*it)->setupWindowRules( true );
        (*it)->applyWindowRules();
        discardUsedWindowRules( *it, false );
        }
    }

void Workspace::slotReinitCompositing()
    {
    // Reparse config. Config options will be reloaded by setupCompositing()
    KGlobal::config()->reparseConfiguration();
    options->updateSettings();

    // Stop any current compositing
    finishCompositing();
    // And start new one
    setupCompositing();
    if( effects ) // setupCompositing() may fail
        effects->reconfigure();
    }

void Workspace::loadDesktopSettings()
    {
    KSharedConfig::Ptr c = KGlobal::config();
    QString groupname;
    if (screen_number == 0)
        groupname = "Desktops";
    else
        groupname.sprintf("Desktops-screen-%d", screen_number);
    KConfigGroup group(c,groupname);

    int n = group.readEntry("Number", 4);
    number_of_desktops = n;
    workarea.clear();
    workarea.resize( n + 1 );
    screenarea.clear();
    rootInfo->setNumberOfDesktops( number_of_desktops );
    desktop_focus_chain.resize( n );
    // make it +1, so that it can be accessed as [1..numberofdesktops]
    focus_chain.resize( n + 1 );
    for(int i = 1; i <= n; i++)
        {
        QString s = group.readEntry(QString("Name_%1").arg(i),
                                i18n("Desktop %1", i));
        rootInfo->setDesktopName( i, s.toUtf8().data() );
        desktop_focus_chain[i-1] = i;
        }
    }

void Workspace::saveDesktopSettings()
    {
    KSharedConfig::Ptr c = KGlobal::config();
    QString groupname;
    if (screen_number == 0)
        groupname = "Desktops";
    else
        groupname.sprintf("Desktops-screen-%d", screen_number);
    KConfigGroup group(c,groupname);

   	group.writeEntry("Number", number_of_desktops );
    for(int i = 1; i <= number_of_desktops; i++)
        {
        QString s = desktopName( i );
        QString defaultvalue = i18n("Desktop %1", i);
        if ( s.isEmpty() )
            {
            s = defaultvalue;
            rootInfo->setDesktopName( i, s.toUtf8().data() );
            }

        if (s != defaultvalue)
            {
            group.writeEntry( QString("Name_%1").arg(i), s );
            }
        else
            {
            QString currentvalue = group.readEntry(QString("Name_%1").arg(i), QString());
            if (currentvalue != defaultvalue)
                group.writeEntry( QString("Name_%1").arg(i), "" );
            }
        }
    }

QStringList Workspace::configModules(bool controlCenter)
    {
    QStringList args;
    args <<  "kwindecoration";
    if (controlCenter)
        args << "kwinoptions";
    else if (KAuthorized::authorizeControlModule("kde-kwinoptions.desktop"))
        args  << "kwinactions" << "kwinfocus" <<  "kwinmoving" << "kwinadvanced" << "kwinrules" << "kwincompositing";
    return args;
    }

void Workspace::configureWM()
    {
        QStringList args;
        args << "--icon" << "preferences-system-windows"
             << configModules(false) ;
        KToolInvocation::kdeinitExec( "kcmshell4", args );
    }

/*!
  avoids managing a window with title \a title
 */
void Workspace::doNotManage( const QString &title )
    {
    doNotManageList.append( title );
    }

/*!
  Hack for java applets
 */
bool Workspace::isNotManaged( const QString& title )
    {
    for ( QStringList::Iterator it = doNotManageList.begin(); it != doNotManageList.end(); ++it )
        {
        QRegExp r( (*it) );
        if (r.indexIn(title) != -1)
            {
            doNotManageList.erase( it );
            return true;
            }
        }
    return false;
    }

/*!
  Refreshes all the client windows
 */
void Workspace::refresh()
    {
    QWidget w( NULL, Qt::X11BypassWindowManagerHint );
    w.setGeometry( QApplication::desktop()->geometry() );
    w.show();
    w.hide();
    QApplication::flush();
    }

/*!
  During virt. desktop switching, desktop areas covered by windows that are
  going to be hidden are first obscured by new windows with no background
  ( i.e. transparent ) placed right below the windows. These invisible windows
  are removed after the switch is complete.
  Reduces desktop ( wallpaper ) repaints during desktop switching
*/
class ObscuringWindows
    {
    public:
        ~ObscuringWindows();
        void create( Client* c );
    private:
        QList<Window> obscuring_windows;
        static QList<Window>* cached;
        static unsigned int max_cache_size;
    };

QList<Window>* ObscuringWindows::cached = 0;
unsigned int ObscuringWindows::max_cache_size = 0;

void ObscuringWindows::create( Client* c )
    {
    if( compositing()) // not needed with compositing
        return;
    if( cached == 0 )
        cached = new QList<Window>;
    Window obs_win;
    XWindowChanges chngs;
    int mask = CWSibling | CWStackMode;
    if( cached->count() > 0 )
        {
        cached->removeAll( obs_win = cached->first());
        chngs.x = c->x();
        chngs.y = c->y();
        chngs.width = c->width();
        chngs.height = c->height();
        mask |= CWX | CWY | CWWidth | CWHeight;
        }
    else
        {
        XSetWindowAttributes a;
        a.background_pixmap = None;
        a.override_redirect = True;
        obs_win = XCreateWindow( display(), rootWindow(), c->x(), c->y(),
            c->width(), c->height(), 0, CopyFromParent, InputOutput,
            CopyFromParent, CWBackPixmap | CWOverrideRedirect, &a );
        }
    chngs.sibling = c->frameId();
    chngs.stack_mode = Below;
    XConfigureWindow( display(), obs_win, mask, &chngs );
    XMapWindow( display(), obs_win );
    obscuring_windows.append( obs_win );
    }

ObscuringWindows::~ObscuringWindows()
    {
    max_cache_size = qMax( ( int )max_cache_size, obscuring_windows.count() + 4 ) - 1;
    for( QList<Window>::ConstIterator it = obscuring_windows.begin();
         it != obscuring_windows.end();
         ++it )
        {
        XUnmapWindow( display(), *it );
        if( cached->count() < ( int )max_cache_size )
            cached->prepend( *it );
        else
            XDestroyWindow( display(), *it );
        }
    }


/*!
  Sets the current desktop to \a new_desktop

  Shows/Hides windows according to the stacking order and finally
  propages the new desktop to the world
 */
bool Workspace::setCurrentDesktop( int new_desktop )
    {
    if (new_desktop < 1 || new_desktop > number_of_desktops )
        return false;

    closeActivePopup();
    ++block_focus;
// TODO    Q_ASSERT( block_stacking_updates == 0 ); // make sure stacking_order is up to date
    StackingUpdatesBlocker blocker( this );

    int old_desktop = current_desktop;
    if (new_desktop != current_desktop)
        {
        ++block_showing_desktop;
        /*
          optimized Desktop switching: unmapping done from back to front
          mapping done from front to back => less exposure events
        */
        Notify::raise((Notify::Event) (Notify::DesktopChange+new_desktop));

        ObscuringWindows obs_wins;

        current_desktop = new_desktop; // change the desktop (so that Client::updateVisibility() works)

        for ( ClientList::ConstIterator it = stacking_order.begin(); it != stacking_order.end(); ++it)
            if ( !(*it)->isOnDesktop( new_desktop ) && (*it) != movingClient )
                {
                if( (*it)->isShown( true ) && (*it)->isOnDesktop( old_desktop ))
                    obs_wins.create( *it );
                (*it)->updateVisibility();
                }

        rootInfo->setCurrentDesktop( current_desktop ); // now propagate the change, after hiding, before showing

        if( movingClient && !movingClient->isOnDesktop( new_desktop ))
            movingClient->setDesktop( new_desktop );

        for( int i = stacking_order.size() - 1; i >= 0 ; --i )
            if ( stacking_order.at( i )->isOnDesktop( new_desktop ) )
                stacking_order.at( i )->updateVisibility();

        --block_showing_desktop;
        if( showingDesktop()) // do this only after desktop change to avoid flicker
            resetShowingDesktop( false );
        }

    // restore the focus on this desktop
    --block_focus;
    Client* c = 0;

    if ( options->focusPolicyIsReasonable())
        {
        // Search in focus chain
        if ( movingClient != NULL && active_client == movingClient
            && focus_chain[currentDesktop()].contains( active_client )
            && active_client->isShown( true ) && active_client->isOnCurrentDesktop())
            {
            c = active_client; // the requestFocus below will fail, as the client is already active
            }
        if( !c )
            {
            for( int i = focus_chain[ currentDesktop() ].size() - 1;
                 i >= 0;
                 --i )
                {
                if( focus_chain[ currentDesktop() ].at( i )->isShown( false )
                    && focus_chain[ currentDesktop() ].at( i )->isOnCurrentDesktop())
                    {
                    c = focus_chain[ currentDesktop() ].at(  i );
                    break;
                    }
                }
            }
        }

    //if "unreasonable focus policy"
    // and active_client is on_all_desktops and under mouse (hence == old_active_client),
    // conserve focus (thanks to Volker Schatz <V.Schatz at thphys.uni-heidelberg.de>)
    else if( active_client && active_client->isShown( true ) && active_client->isOnCurrentDesktop())
      c = active_client;

    if( c == NULL && !desktops.isEmpty())
        c = findDesktop( true, currentDesktop());

    if( c != active_client )
        setActiveClient( NULL, Allowed );

    if ( c )
        requestFocus( c );
    else if( !desktops.isEmpty() )
        requestFocus( findDesktop( true, currentDesktop()));
    else
        focusToNull();

    updateCurrentTopMenu();

    // Update focus chain:
    //  If input: chain = { 1, 2, 3, 4 } and currentDesktop() = 3,
    //   Output: chain = { 3, 1, 2, 4 }.
//    kDebug(1212) << QString("Switching to desktop #%1, at focus_chain index %2\n")
//      .arg(currentDesktop()).arg(desktop_focus_chain.find( currentDesktop() ));
    for( int i = desktop_focus_chain.indexOf( currentDesktop() ); i > 0; i-- )
        desktop_focus_chain[i] = desktop_focus_chain[i-1];
    desktop_focus_chain[0] = currentDesktop();

//    QString s = "desktop_focus_chain[] = { ";
//    for( uint i = 0; i < desktop_focus_chain.size(); i++ )
//        s += QString::number(desktop_focus_chain[i]) + ", ";
//    kDebug(1212) << s << "}\n";

    if( old_desktop != 0 )  // not for the very first time
        popupinfo->showInfo( desktopName(currentDesktop()) );

    if( effects != NULL && old_desktop != 0 && old_desktop != new_desktop )
        static_cast<EffectsHandlerImpl*>(effects)->desktopChanged( old_desktop );
    if( compositing())
        addRepaintFull();

    return true;
    }

// called only from DCOP
void Workspace::nextDesktop()
    {
    int desktop = currentDesktop() + 1;
    setCurrentDesktop(desktop > numberOfDesktops() ? 1 : desktop);
    }

// called only from DCOP
void Workspace::previousDesktop()
    {
    int desktop = currentDesktop() - 1;
    setCurrentDesktop(desktop > 0 ? desktop : numberOfDesktops());
    }

int Workspace::desktopToRight( int desktop, bool wrap ) const
    {
    int x,y;
    Qt::Orientation orientation;
    calcDesktopLayout( &x, &y, &orientation );
    int dt = desktop-1;
    if (orientation == Qt::Vertical)
        {
        dt += y;
        if ( dt >= numberOfDesktops() )
            {
            if ( wrap )
              dt -= numberOfDesktops();
            else
              return desktop;
            }
        }
    else
        {
        int d = (dt % x) + 1;
        if ( d >= x )
            {
            if ( wrap )
              d -= x;
            else
              return desktop;
            }
        dt = dt - (dt % x) + d;
        }
    return dt+1;
    }

int Workspace::desktopToLeft( int desktop, bool wrap ) const
    {
    int x,y;
    Qt::Orientation orientation;
    calcDesktopLayout( &x, &y, &orientation );
    int dt = desktop-1;
    if (orientation == Qt::Vertical)
        {
        dt -= y;
        if ( dt < 0 )
            {
            if ( wrap )
              dt += numberOfDesktops();
            else
              return desktop;
            }
        }
    else
        {
        int d = (dt % x) - 1;
        if ( d < 0 )
            {
            if ( wrap )
              d += x;
            else
              return desktop;
            }
        dt = dt - (dt % x) + d;
        }
    return dt+1;
    }

int Workspace::desktopUp( int desktop, bool wrap ) const
    {
    int x,y;
    Qt::Orientation orientation;
    calcDesktopLayout( &x, &y, &orientation);
    int dt = desktop-1;
    if (orientation == Qt::Horizontal)
        {
        dt -= x;
        if ( dt < 0 )
            {
            if ( wrap )
              dt += numberOfDesktops();
            else
              return desktop;
            }
        }
    else
        {
        int d = (dt % y) - 1;
        if ( d < 0 )
            {
            if ( wrap )
              d += y;
            else
              return desktop;
            }
        dt = dt - (dt % y) + d;
        }
    return dt+1;
    }

int Workspace::desktopDown( int desktop, bool wrap ) const
    {
    int x,y;
    Qt::Orientation orientation;
    calcDesktopLayout( &x, &y, &orientation);
    int dt = desktop-1;
    if (orientation == Qt::Horizontal)
        {
        dt += x;
        if ( dt >= numberOfDesktops() )
            {
            if ( wrap )
              dt -= numberOfDesktops();
            else
              return desktop;
            }
        }
    else
        {
        int d = (dt % y) + 1;
        if ( d >= y )
            {
            if ( wrap )
              d -= y;
            else
              return desktop;
            }
        dt = dt - (dt % y) + d;
        }
    return dt+1;
    }


/*!
  Sets the number of virtual desktops to \a n
 */
void Workspace::setNumberOfDesktops( int n )
    {
    if ( n == number_of_desktops )
        return;
    int old_number_of_desktops = number_of_desktops;
    number_of_desktops = n;

    if( currentDesktop() > numberOfDesktops())
        setCurrentDesktop( numberOfDesktops());

    // if increasing the number, do the resizing now,
    // otherwise after the moving of windows to still existing desktops
    if( old_number_of_desktops < number_of_desktops )
        {
        rootInfo->setNumberOfDesktops( number_of_desktops );
        NETPoint* viewports = new NETPoint[ number_of_desktops ];
        rootInfo->setDesktopViewport( number_of_desktops, *viewports );
        delete[] viewports;
        updateClientArea( true );
        focus_chain.resize( number_of_desktops + 1 );
        }

    // if the number of desktops decreased, move all
    // windows that would be hidden to the last visible desktop
    if( old_number_of_desktops > number_of_desktops )
        {
        for( ClientList::ConstIterator it = clients.begin();
              it != clients.end();
              ++it)
            {
            if( !(*it)->isOnAllDesktops() && (*it)->desktop() > numberOfDesktops())
                sendClientToDesktop( *it, numberOfDesktops(), true );
            }
        }
    if( old_number_of_desktops > number_of_desktops )
        {
        rootInfo->setNumberOfDesktops( number_of_desktops );
        NETPoint* viewports = new NETPoint[ number_of_desktops ];
        rootInfo->setDesktopViewport( number_of_desktops, *viewports );
        delete[] viewports;
        updateClientArea( true );
        focus_chain.resize( number_of_desktops + 1 );
        }

    saveDesktopSettings();

    // Resize and reset the desktop focus chain.
    desktop_focus_chain.resize( n );
    for( int i = 0; i < (int)desktop_focus_chain.size(); i++ )
        desktop_focus_chain[i] = i+1;
    }

/*!
  Sends client \a c to desktop \a desk.

  Takes care of transients as well.
 */
void Workspace::sendClientToDesktop( Client* c, int desk, bool dont_activate )
    {
    bool was_on_desktop = c->isOnDesktop( desk ) || c->isOnAllDesktops();
    c->setDesktop( desk );
    if ( c->desktop() != desk ) // no change or desktop forced
        return;
    desk = c->desktop(); // Client did range checking

    if ( c->isOnDesktop( currentDesktop() ) )
        {
        if ( c->wantsTabFocus() && options->focusPolicyIsReasonable()
            && !was_on_desktop // for stickyness changes
            && !dont_activate )
            requestFocus( c );
        else
            restackClientUnderActive( c );
        }
    else
        {
        raiseClient( c );
        }

    ClientList transients_stacking_order = ensureStackingOrder( c->transients());
    for( ClientList::ConstIterator it = transients_stacking_order.begin();
         it != transients_stacking_order.end();
         ++it )
        sendClientToDesktop( *it, desk, dont_activate );
    updateClientArea();
    }

int Workspace::numScreens() const
    {
    if( !options->xineramaEnabled )
        return 1;
    return qApp->desktop()->numScreens();
    }

int Workspace::activeScreen() const
    {
    if( !options->xineramaEnabled )
        return 0;
    if( !options->activeMouseScreen )
        {
        if( activeClient() != NULL && !activeClient()->isOnScreen( active_screen ))
            return qApp->desktop()->screenNumber( activeClient()->geometry().center());
        return active_screen;
        }
    return qApp->desktop()->screenNumber( cursorPos());
    }

// check whether a client moved completely out of what's considered the active screen,
// if yes, set a new active screen
void Workspace::checkActiveScreen( const Client* c )
    {
    if( !options->xineramaEnabled )
        return;
    if( !c->isActive())
        return;
    if( !c->isOnScreen( active_screen ))
        active_screen = c->screen();
    }

// called e.g. when a user clicks on a window, set active screen to be the screen
// where the click occurred
void Workspace::setActiveScreenMouse( const QPoint &mousepos )
    {
    if( !options->xineramaEnabled )
        return;
    active_screen = qApp->desktop()->screenNumber( mousepos );
    }

QRect Workspace::screenGeometry( int screen ) const
    {
    if( !options->xineramaEnabled )
        return qApp->desktop()->geometry();
    return qApp->desktop()->screenGeometry( screen );
    }

int Workspace::screenNumber( const QPoint &pos ) const
    {
    if( !options->xineramaEnabled )
        return 0;
    return qApp->desktop()->screenNumber( pos );
    }

void Workspace::sendClientToScreen( Client* c, int screen )
    {
    if( c->screen() == screen ) // don't use isOnScreen(), that's true even when only partially
        return;
    GeometryUpdatesBlocker blocker( c );
    QRect old_sarea = clientArea( MaximizeArea, c );
    QRect sarea = clientArea( MaximizeArea, screen, c->desktop());
    c->setGeometry( sarea.x() - old_sarea.x() + c->x(), sarea.y() - old_sarea.y() + c->y(),
        c->size().width(), c->size().height());
    c->checkWorkspacePosition();
    ClientList transients_stacking_order = ensureStackingOrder( c->transients());
    for( ClientList::ConstIterator it = transients_stacking_order.begin();
         it != transients_stacking_order.end();
         ++it )
        sendClientToScreen( *it, screen );
    if( c->isActive())
        active_screen = screen;
    }

void Workspace::updateDesktopLayout()
    {
    // rootInfo->desktopLayoutCorner(); // I don't find this worth bothering, feel free to
    layoutOrientation = ( rootInfo->desktopLayoutOrientation() == NET::OrientationHorizontal
        ? Qt::Horizontal : Qt::Vertical );
    layoutX = rootInfo->desktopLayoutColumnsRows().width();
    layoutY = rootInfo->desktopLayoutColumnsRows().height();
    if( layoutX == 0 && layoutY == 0 ) // not given, set default layout
        layoutY = 2;
    }

void Workspace::calcDesktopLayout(int* xp, int* yp, Qt::Orientation* orientation) const
    {
    int x = layoutX; // <= 0 means compute it from the other and total number of desktops
    int y = layoutY;
    if((x <= 0) && (y > 0))
       x = (numberOfDesktops()+y-1) / y;
    else if((y <=0) && (x > 0))
       y = (numberOfDesktops()+x-1) / x;

    if(x <=0)
       x = 1;
    if (y <= 0)
       y = 1;
    *xp = x;
    *yp = y;
    *orientation = layoutOrientation;
    }

void Workspace::killWindowId( Window window_to_kill )
    {
    if( window_to_kill == None )
        return;
    Window window = window_to_kill;
    Client* client = NULL;
    for(;;)
        {
        client = findClient( FrameIdMatchPredicate( window ));
        if( client != NULL ) // found the client
            break;
        Window parent, root;
        Window* children;
        unsigned int children_count;
        XQueryTree( display(), window, &root, &parent, &children, &children_count );
        if( children != NULL )
            XFree( children );
        if( window == root ) // we didn't find the client, probably an override-redirect window
            break;
        window = parent; // go up
        }
    if( client != NULL )
        client->killWindow();
    else
        XKillClient( display(), window_to_kill );
    }


void Workspace::sendPingToWindow( Window window, Time timestamp )
    {
    rootInfo->sendPing( window, timestamp );
    }

void Workspace::sendTakeActivity( Client* c, Time timestamp, long flags )
    {
    rootInfo->takeActivity( c->window(), timestamp, flags );
    pending_take_activity = c;
    }


/*!
  Takes a screenshot of the current window and puts it in the clipboard.
*/
void Workspace::slotGrabWindow()
    {
    if ( active_client )
        {
        QPixmap snapshot = QPixmap::grabWindow( active_client->frameId() );

	//No XShape - no work.
        if( Extensions::shapeAvailable())
            {
	    //As the first step, get the mask from XShape.
            int count, order;
            XRectangle* rects = XShapeGetRectangles( display(), active_client->frameId(),
                                                     ShapeBounding, &count, &order);
	    //The ShapeBounding region is the outermost shape of the window;
	    //ShapeBounding - ShapeClipping is defined to be the border.
	    //Since the border area is part of the window, we use bounding
	    // to limit our work region
            if (rects)
                {
		//Create a QRegion from the rectangles describing the bounding mask.
                QRegion contents;
                for (int pos = 0; pos < count; pos++)
                    contents += QRegion(rects[pos].x, rects[pos].y,
                                        rects[pos].width, rects[pos].height);
                XFree(rects);

		//Create the bounding box.
                QRegion bbox(0, 0, snapshot.width(), snapshot.height());

		//Get the masked away area.
                QRegion maskedAway = bbox - contents;
                QVector<QRect> maskedAwayRects = maskedAway.rects();

		//Construct a bitmap mask from the rectangles
                QBitmap mask( snapshot.width(), snapshot.height());
                QPainter p(&mask);
                p.fillRect(0, 0, mask.width(), mask.height(), Qt::color1);
                for (int pos = 0; pos < maskedAwayRects.count(); pos++)
                    p.fillRect(maskedAwayRects[pos], Qt::color0);
                p.end();
                snapshot.setMask(mask);
                }
            }

        QClipboard *cb = QApplication::clipboard();
        cb->setPixmap( snapshot );
        }
    else
        slotGrabDesktop();
    }

/*!
  Takes a screenshot of the whole desktop and puts it in the clipboard.
*/
void Workspace::slotGrabDesktop()
    {
    QPixmap p = QPixmap::grabWindow( rootWindow() );
    QClipboard *cb = QApplication::clipboard();
    cb->setPixmap( p );
    }


/*!
  Invokes keyboard mouse emulation
 */
void Workspace::slotMouseEmulation()
    {
    if ( mouse_emulation )
        {
        ungrabXKeyboard();
        mouse_emulation = false;
        return;
        }

    if( grabXKeyboard())
        {
        mouse_emulation = true;
        mouse_emulation_state = 0;
        mouse_emulation_window = 0;
        }
    }

/*!
  Returns the child window under the mouse and activates the
  respective client if necessary.

  Auxiliary function for the mouse emulation system.
 */
WId Workspace::getMouseEmulationWindow()
    {
    Window root;
    Window child = rootWindow();
    int root_x, root_y, lx, ly;
    uint state;
    Window w;
    Client * c = 0;
    do
        {
        w = child;
        if (!c)
            c = findClient( FrameIdMatchPredicate( w ));
        XQueryPointer( display(), w, &root, &child,
                       &root_x, &root_y, &lx, &ly, &state );
        } while  ( child != None && child != w );

    if ( c && !c->isActive() )
        activateClient( c );
    return (WId) w;
    }

/*!
  Sends a faked mouse event to the specified window. Returns the new button state.
 */
unsigned int Workspace::sendFakedMouseEvent( const QPoint &pos, WId w, MouseEmulation type, int button, unsigned int state )
    {
    if ( !w )
        return state;
    QWidget* widget = QWidget::find( w );
    if ( (!widget ||  qobject_cast<QToolButton*>(widget)) && !findClient( WindowMatchPredicate( w )) )
        {
        int x, y;
        Window xw;
        XTranslateCoordinates( display(), rootWindow(), w, pos.x(), pos.y(), &x, &y, &xw );
        if ( type == EmuMove )
            { // motion notify events
            XEvent e;
            e.type = MotionNotify;
            e.xmotion.window = w;
            e.xmotion.root = rootWindow();
            e.xmotion.subwindow = w;
            e.xmotion.time = xTime();
            e.xmotion.x = x;
            e.xmotion.y = y;
            e.xmotion.x_root = pos.x();
            e.xmotion.y_root = pos.y();
            e.xmotion.state = state;
            e.xmotion.is_hint = NotifyNormal;
            XSendEvent( display(), w, true, ButtonMotionMask, &e );
            }
        else
            {
            XEvent e;
            e.type = type == EmuRelease ? ButtonRelease : ButtonPress;
            e.xbutton.window = w;
            e.xbutton.root = rootWindow();
            e.xbutton.subwindow = w;
            e.xbutton.time = xTime();
            e.xbutton.x = x;
            e.xbutton.y = y;
            e.xbutton.x_root = pos.x();
            e.xbutton.y_root = pos.y();
            e.xbutton.state = state;
            e.xbutton.button = button;
            XSendEvent( display(), w, true, ButtonPressMask, &e );

            if ( type == EmuPress )
                {
                switch ( button )
                    {
                    case 2:
                        state |= Button2Mask;
                        break;
                    case 3:
                        state |= Button3Mask;
                        break;
                    default: // 1
                        state |= Button1Mask;
                        break;
                    }
                }
            else
                {
                switch ( button )
                    {
                    case 2:
                        state &= ~Button2Mask;
                        break;
                    case 3:
                        state &= ~Button3Mask;
                        break;
                    default: // 1
                        state &= ~Button1Mask;
                        break;
                    }
                }
            }
        }
    return state;
    }

/*!
  Handles keypress event during mouse emulation
 */
bool Workspace::keyPressMouseEmulation( XKeyEvent& ev )
    {
    int kc = XKeycodeToKeysym(display(), ev.keycode, 0);
    int km = ev.state & (ControlMask | Mod1Mask | ShiftMask);

    bool is_control = km & ControlMask;
    bool is_alt = km & Mod1Mask;
    bool is_shift = km & ShiftMask;
    int delta = is_control?1:is_alt?32:8;
    QPoint pos = cursorPos();

    switch ( kc )
        {
        case XK_Left:
        case XK_KP_Left:
            pos.rx() -= delta;
            break;
        case XK_Right:
        case XK_KP_Right:
            pos.rx() += delta;
            break;
        case XK_Up:
        case XK_KP_Up:
            pos.ry() -= delta;
            break;
        case XK_Down:
        case XK_KP_Down:
            pos.ry() += delta;
            break;
        case XK_F1:
            if ( !mouse_emulation_state )
                mouse_emulation_window = getMouseEmulationWindow();
            if ( (mouse_emulation_state & Button1Mask) == 0 )
                mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuPress, Button1, mouse_emulation_state );
            if ( !is_shift )
                mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuRelease, Button1, mouse_emulation_state );
            break;
        case XK_F2:
            if ( !mouse_emulation_state )
                mouse_emulation_window = getMouseEmulationWindow();
            if ( (mouse_emulation_state & Button2Mask) == 0 )
                mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuPress, Button2, mouse_emulation_state );
            if ( !is_shift )
                mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuRelease, Button2, mouse_emulation_state );
            break;
        case XK_F3:
            if ( !mouse_emulation_state )
                mouse_emulation_window = getMouseEmulationWindow();
            if ( (mouse_emulation_state & Button3Mask) == 0 )
                mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuPress, Button3, mouse_emulation_state );
            if ( !is_shift )
                mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuRelease, Button3, mouse_emulation_state );
            break;
        case XK_Return:
        case XK_space:
        case XK_KP_Enter:
        case XK_KP_Space:
            {
            if ( !mouse_emulation_state )
                {
            // nothing was pressed, fake a LMB click
                mouse_emulation_window = getMouseEmulationWindow();
                mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuPress, Button1, mouse_emulation_state );
                mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuRelease, Button1, mouse_emulation_state );
                }
            else
                { // release all
                if ( mouse_emulation_state & Button1Mask )
                    mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuRelease, Button1, mouse_emulation_state );
                if ( mouse_emulation_state & Button2Mask )
                    mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuRelease, Button2, mouse_emulation_state );
                if ( mouse_emulation_state & Button3Mask )
                    mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuRelease, Button3, mouse_emulation_state );
                }
            }
    // fall through
        case XK_Escape:
            ungrabXKeyboard();
            mouse_emulation = false;
            return true;
        default:
            return false;
        }

    QCursor::setPos( pos );
    if ( mouse_emulation_state )
        mouse_emulation_state = sendFakedMouseEvent( pos, mouse_emulation_window, EmuMove, 0, mouse_emulation_state );
    return true;

    }

//Delayed focus functions
void Workspace::delayFocus()
    {
    requestFocus( delayfocus_client );
    cancelDelayFocus();
    }

void Workspace::requestDelayFocus( Client* c )
    {
    delayfocus_client = c;
    delete delayFocusTimer;
    delayFocusTimer = new QTimer( this );
    connect( delayFocusTimer, SIGNAL( timeout() ), this, SLOT( delayFocus() ) );
    delayFocusTimer->setSingleShot( true );
    delayFocusTimer->start( options->delayFocusInterval );
    }

void Workspace::cancelDelayFocus()
    {
    delete delayFocusTimer;
    delayFocusTimer = 0;
    }

// Electric Borders
//========================================================================//
// Electric Border Window management. Electric borders allow a user
// to change the virtual desktop or activate another features
// by moving the mouse pointer to the borders or corners.
// Technically this is done with input only windows.
void Workspace::updateElectricBorders()
    {
    electric_time_first = xTime();
    electric_time_last = xTime();
    electric_current_border = ElectricNone;
    QRect r = QApplication::desktop()->geometry();
    electricTop = r.top();
    electricBottom = r.bottom();
    electricLeft = r.left();
    electricRight = r.right();

    for( int pos = 0;
         pos < ELECTRIC_COUNT;
         ++pos )
        {
        if( electric_reserved[ pos ] == 0 )
            {
            if( electric_windows[ pos ] != None )
                XDestroyWindow( display(), electric_windows[ pos ] );
            electric_windows[ pos ] = None;
            continue;
            }
        if( electric_windows[ pos ] != None )
            continue;
        XSetWindowAttributes attributes;
        attributes.override_redirect = True;
        attributes.event_mask = EnterWindowMask | LeaveWindowMask;
        unsigned long valuemask = CWOverrideRedirect | CWEventMask;
        int xywh[ ELECTRIC_COUNT ][ 4 ] =
            {
                { r.left() + 1, r.top(), r.width() - 2, 1 }, // top
                { r.right(), r.top(), 1, 1 }, // topright
                { r.right(), r.top() + 1, 1, r.height() - 2 }, // etc.
                { r.right(), r.bottom(), 1, 1 },
                { r.left() + 1, r.bottom(), r.width() - 2, 1 },
                { r.left(), r.bottom(), 1, 1 },
                { r.left(), r.top() + 1, 1, r.height() - 2 },
                { r.left(), r.top(), 1, 1 }
            };
        electric_windows[ pos ] = XCreateWindow( display(), rootWindow(),
            xywh[ pos ][ 0 ], xywh[ pos ][ 1 ], xywh[ pos ][ 2 ], xywh[ pos ][ 3 ],
            0, CopyFromParent, InputOnly, CopyFromParent, valuemask, &attributes );
        XMapWindow( display(), electric_windows[ pos ]);
        // Set XdndAware on the windows, so that DND enter events are received (#86998)
        Atom version = 4; // XDND version
        XChangeProperty( display(), electric_windows[ pos ], atoms->xdnd_aware, XA_ATOM,
            32, PropModeReplace, ( unsigned char* )&version, 1 );
        }
    }

void Workspace::destroyElectricBorders()
    {
    for( int pos = 0;
         pos < ELECTRIC_COUNT;
         ++pos )
        {
        if( electric_windows[ pos ] != None )
            XDestroyWindow( display(), electric_windows[ pos ] );
        electric_windows[ pos ] = None;
        }
    }

void Workspace::reserveElectricBorderSwitching( bool reserve )
    {
    for( int pos = 0;
         pos < ELECTRIC_COUNT;
         ++pos )
        if( reserve )
            reserveElectricBorder( static_cast< ElectricBorder >( pos ));
        else
            unreserveElectricBorder( static_cast< ElectricBorder >( pos ));
    }

void Workspace::reserveElectricBorder( ElectricBorder border )
    {
    if( border == ElectricNone )
        return;
    if( electric_reserved[ border ]++ == 0 )
        QTimer::singleShot( 0, this, SLOT( updateElectricBorders()));
    }

void Workspace::unreserveElectricBorder( ElectricBorder border )
    {
    if( border == ElectricNone )
        return;
    assert( electric_reserved[ border ] > 0 );
    if( --electric_reserved[ border ] == 0 )
        QTimer::singleShot( 0, this, SLOT( updateElectricBorders()));
    }

void Workspace::checkElectricBorder(const QPoint &pos, Time now)
    {
    if ((pos.x() != electricLeft) &&
        (pos.x() != electricRight) &&
        (pos.y() != electricTop) &&
        (pos.y() != electricBottom))
       return;

    bool have_borders = false;
    for( int i = 0;
         i < ELECTRIC_COUNT;
         ++i )
        if( electric_windows[ i ] != None )
            have_borders = true;
    if( !have_borders )
        return;

    Time treshold_set = options->electricBorderDelay(); // set timeout
    Time treshold_reset = 250; // reset timeout
    int distance_reset = 30; // Mouse should not move more than this many pixels

    ElectricBorder border;
    if( pos.x() == electricLeft && pos.y() == electricTop )
        border = ElectricTopLeft;
    else if( pos.x() == electricRight && pos.y() == electricTop )
        border = ElectricTopRight;
    else if( pos.x() == electricLeft && pos.y() == electricBottom )
        border = ElectricBottomLeft;
    else if( pos.x() == electricRight && pos.y() == electricBottom )
        border = ElectricBottomRight;
    else if( pos.x() == electricLeft )
        border = ElectricLeft;
    else if( pos.x() == electricRight )
        border = ElectricRight;
    else if( pos.y() == electricTop )
        border = ElectricTop;
    else if( pos.y() == electricBottom )
        border = ElectricBottom;
    else
        abort();

    if( electric_windows[ border ] == None )
        return;

    if ((electric_current_border == border) &&
        (timestampDiff(electric_time_last, now) < treshold_reset) &&
        ((pos-electric_push_point).manhattanLength() < distance_reset))
        {
        electric_time_last = now;

        if (timestampDiff(electric_time_first, now) > treshold_set)
            {
            electric_current_border = ElectricNone;
            if( effects && static_cast<EffectsHandlerImpl*>(effects)->borderActivated( border ))
                {} // handled by effects
            else
                electricBorderSwitchDesktop( border, pos );
            return;
            }
        }
    else
        {
        electric_current_border = border;
        electric_time_first = now;
        electric_time_last = now;
        electric_push_point = pos;
        }

    // reset the pointer to find out wether the user is really pushing
    // (the direction back from which it came, starting from top clockwise)
    const int xdiff[ ELECTRIC_COUNT ] = { 0, -1, -1, -1, 0, 1, 1, 1 };
    const int ydiff[ ELECTRIC_COUNT ] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    QCursor::setPos( pos.x() + xdiff[ border ], pos.y() + ydiff[ border ] );
    }

void Workspace::electricBorderSwitchDesktop( ElectricBorder border, const QPoint& _pos )
    {
    QPoint pos = _pos;
    int desk = currentDesktop();
    const int OFFSET = 2;
    if( border == ElectricLeft || border == ElectricTopLeft || border == ElectricBottomLeft )
        {
        desk = desktopToLeft( desk, options->rollOverDesktops );
        pos.setX( displayWidth() - 1 - OFFSET );
        }
    if( border == ElectricRight || border == ElectricTopRight || border == ElectricBottomRight )
        {
        desk = desktopToRight( desk, options->rollOverDesktops );
        pos.setX( OFFSET );
        }
    if( border == ElectricTop || border == ElectricTopLeft || border == ElectricTopRight )
        {
        desk = desktopUp( desk, options->rollOverDesktops );
        pos.setY( displayHeight() - 1 - OFFSET );
        }
    if( border == ElectricBottom || border == ElectricBottomLeft || border == ElectricBottomRight )
        {
        desk = desktopDown( desk, options->rollOverDesktops );
        pos.setY( OFFSET );
        }
    int desk_before = currentDesktop();
    setCurrentDesktop( desk );
    if( currentDesktop() != desk_before )
        QCursor::setPos( pos ); 
    }

// this function is called when the user entered an electric border
// with the mouse. It may switch to another virtual desktop
bool Workspace::electricBorderEvent(XEvent *e)
    {
    if( e->type == EnterNotify )
        {
        for( int i = 0;
             i < ELECTRIC_COUNT;
             ++i )
            if( electric_windows[ i ] != None && e->xcrossing.window == electric_windows[ i ] )
                { // the user entered an electric border
                checkElectricBorder( QPoint( e->xcrossing.x_root, e->xcrossing.y_root ), e->xcrossing.time );
                return true;
                }
        }
    if( e->type == ClientMessage )
        {
        if( e->xclient.message_type == atoms->xdnd_position )
            {
            for( int i = 0;
                 i < ELECTRIC_COUNT;
                 ++i )
                if( electric_windows[ i ] != None && e->xclient.window == electric_windows[ i ] )
                    {
                    updateXTime();
                    checkElectricBorder( QPoint( e->xclient.data.l[2]>>16, e->xclient.data.l[2]&0xffff), xTime() );
                    return true;
                    }
            }
        }
    return false;
    }

void Workspace::addTopMenu( Client* c )
    {
    assert( c->isTopMenu());
    assert( !topmenus.contains( c ));
    topmenus.append( c );
    if( managingTopMenus())
        {
        int minsize = c->minSize().height();
        if( minsize > topMenuHeight())
            {
            topmenu_height = minsize;
            updateTopMenuGeometry();
            }
        updateTopMenuGeometry( c );
        updateCurrentTopMenu();
        }
//        kDebug() << "NEW TOPMENU:" << c;
    }

void Workspace::removeTopMenu( Client* c )
    {
//    if( c->isTopMenu())
//        kDebug() << "REMOVE TOPMENU:" << c;
    assert( c->isTopMenu());
    assert( topmenus.contains( c ));
    topmenus.removeAll( c );
    updateCurrentTopMenu();
    // TODO reduce topMenuHeight() if possible?
    }

void Workspace::lostTopMenuSelection()
    {
//    kDebug() << "lost TopMenu selection";
    // make sure this signal is always set when not owning the selection
    disconnect( topmenu_watcher, SIGNAL( lostOwner()), this, SLOT( lostTopMenuOwner()));
    connect( topmenu_watcher, SIGNAL( lostOwner()), this, SLOT( lostTopMenuOwner()));
    if( !managing_topmenus )
        return;
    connect( topmenu_watcher, SIGNAL( lostOwner()), this, SLOT( lostTopMenuOwner()));
    disconnect( topmenu_selection, SIGNAL( lostOwnership()), this, SLOT( lostTopMenuSelection()));
    managing_topmenus = false;
    delete topmenu_space;
    topmenu_space = NULL;
    updateClientArea();
    for( ClientList::ConstIterator it = topmenus.begin();
         it != topmenus.end();
         ++it )
        (*it)->checkWorkspacePosition();
    }

void Workspace::lostTopMenuOwner()
    {
    if( !options->topMenuEnabled())
        return;
//    kDebug() << "TopMenu selection lost owner";
    if( !topmenu_selection->claim( false ))
        {
//        kDebug() << "Failed to claim TopMenu selection";
        return;
        }
//    kDebug() << "claimed TopMenu selection";
    setupTopMenuHandling();
    }

void Workspace::setupTopMenuHandling()
    {
    if( managing_topmenus )
        return;
    connect( topmenu_selection, SIGNAL( lostOwnership()), this, SLOT( lostTopMenuSelection()));
    disconnect( topmenu_watcher, SIGNAL( lostOwner()), this, SLOT( lostTopMenuOwner()));
    managing_topmenus = true;
    topmenu_space = new QWidget( NULL, Qt::X11BypassWindowManagerHint );
    Window stack[ 2 ];
    stack[ 0 ] = supportWindow->winId();
    stack[ 1 ] = topmenu_space->winId();
    XRestackWindows(display(), stack, 2);
    updateTopMenuGeometry();
    topmenu_space->show();
    updateClientArea();
    updateCurrentTopMenu();
    }

int Workspace::topMenuHeight() const
    {
    if( topmenu_height == 0 )
        { // simply create a dummy menubar and use its preffered height as the menu height
        KMenuBar tmpmenu;
        tmpmenu.addAction( "dummy" );
        topmenu_height = tmpmenu.sizeHint().height();
        }
    return topmenu_height;
    }

KDecoration* Workspace::createDecoration( KDecorationBridge* bridge )
    {
    return mgr->createDecoration( bridge );
    }

// returns a list of all colors (KDecorationDefines::ColorType) the current
// decoration supports
QList< int > Workspace::decorationSupportedColors() const
    {
    KDecorationFactory* factory = mgr->factory();
    QList< int > ret;
    for( Ability ab = ABILITYCOLOR_FIRST;
         ab < ABILITYCOLOR_END;
         ab = static_cast< Ability>( ab + 1 ))
        {
        if( factory->supports( ab ))
            ret << ab;
        }
    return ret;
    }

QString Workspace::desktopName( int desk ) const
    {
    return QString::fromUtf8( rootInfo->desktopName( desk ) );
    }

bool Workspace::checkStartupNotification( Window w, KStartupInfoId& id, KStartupInfoData& data )
    {
    return startup->checkStartup( w, id, data ) == KStartupInfo::Match;
    }

/*!
  Puts the focus on a dummy window
  Just using XSetInputFocus() with None would block keyboard input
 */
void Workspace::focusToNull()
    {
    XSetInputFocus(display(), null_focus_window, RevertToPointerRoot, xTime() );
    }

void Workspace::helperDialog( const QString& message, const Client* c )
    {
    QStringList args;
    QString type;
    if( message == "noborderaltf3" )
        {
        KAction* action = qobject_cast<KAction*>(keys->action( "Window Operations Menu" ));
        if (action==0) assert( false );
        QString shortcut = QString( "%1 (%2)" ).arg( action->text() )
            .arg( action->globalShortcut().primary().toString());
        args << "--msgbox" <<
              i18n( "You have selected to show a window without its border.\n"
                    "Without the border, you will not be able to enable the border "
                    "again using the mouse: use the window operations menu instead, "
                    "activated using the %1 keyboard shortcut." ,
                  shortcut );
        type = "altf3warning";
        }
    else if( message == "fullscreenaltf3" )
        {
        KAction* action = qobject_cast<KAction*>(keys->action( "Window Operations Menu" ));
        if (action==0) assert( false );
        QString shortcut = QString( "%1 (%2)" ).arg( action->text() )
            .arg( action->globalShortcut().primary().toString());
        args << "--msgbox" <<
              i18n( "You have selected to show a window in fullscreen mode.\n"
                    "If the application itself does not have an option to turn the fullscreen "
                    "mode off you will not be able to disable it "
                    "again using the mouse: use the window operations menu instead, "
                    "activated using the %1 keyboard shortcut." ,
                  shortcut );
        type = "altf3warning";
        }
    else
        assert( false );
    if( !type.isEmpty())
        {
        KConfig cfg( "kwin_dialogsrc" );
	KConfigGroup cg(&cfg, "Notification Messages" ); // this depends on KMessageBox
        if( !cg.readEntry( type, true )) // has don't show again checked
            return;                           // save launching kdialog
	args <<"--dontagain" << "kwin_dialogsrc:" + type;
        }
    if( c != NULL )
	args <<"--embed" << QString::number( c->window());
    KProcess::startDetached("kdialog",args);
    }

void Workspace::setShowingDesktop( bool showing )
    {
    rootInfo->setShowingDesktop( showing );
    showing_desktop = showing;
    ++block_showing_desktop;
    if( showing_desktop )
        {
        showing_desktop_clients.clear();
        ++block_focus;
        ClientList cls = stackingOrder();
        // find them first, then minimize, otherwise transients may get minimized with the window
        // they're transient for
        for( ClientList::ConstIterator it = cls.begin();
             it != cls.end();
             ++it )
            {
            if( (*it)->isOnCurrentDesktop() && (*it)->isShown( true ) && !(*it)->isSpecialWindow())
                showing_desktop_clients.prepend( *it ); // topmost first to reduce flicker
            }
        for( ClientList::ConstIterator it = showing_desktop_clients.begin();
             it != showing_desktop_clients.end();
             ++it )
            (*it)->minimize();
        --block_focus;
        if( Client* desk = findDesktop( true, currentDesktop()))
            requestFocus( desk );
        }
    else
        {
        for( ClientList::ConstIterator it = showing_desktop_clients.begin();
             it != showing_desktop_clients.end();
             ++it )
            (*it)->unminimize();
        if( showing_desktop_clients.count() > 0 )
            requestFocus( showing_desktop_clients.first());
        showing_desktop_clients.clear();
        }
    --block_showing_desktop;
    }

// Following Kicker's behavior:
// Changing a virtual desktop resets the state and shows the windows again.
// Unminimizing a window resets the state but keeps the windows hidden (except
// the one that was unminimized).
// A new window resets the state and shows the windows again, with the new window
// being active. Due to popular demand (#67406) by people who apparently
// don't see a difference between "show desktop" and "minimize all", this is not
// true if "showDesktopIsMinimizeAll" is set in kwinrc. In such case showing
// a new window resets the state but doesn't show windows.
void Workspace::resetShowingDesktop( bool keep_hidden )
    {
    if( block_showing_desktop > 0 )
        return;
    rootInfo->setShowingDesktop( false );
    showing_desktop = false;
    ++block_showing_desktop;
    if( !keep_hidden )
        {
        for( ClientList::ConstIterator it = showing_desktop_clients.begin();
             it != showing_desktop_clients.end();
             ++it )
            (*it)->unminimize();
        }
    showing_desktop_clients.clear();
    --block_showing_desktop;
    }

// Activating/deactivating this feature works like this:
// When nothing is active, and the shortcut is pressed, global shortcuts are disabled
//   (using global_shortcuts_disabled)
// When a window that has disabling forced is activated, global shortcuts are disabled.
//   (using global_shortcuts_disabled_for_client)
// When a shortcut is pressed and global shortcuts are disabled (either by a shortcut
// or for a client), they are enabled again.
void Workspace::slotDisableGlobalShortcuts()
    {
    if( global_shortcuts_disabled || global_shortcuts_disabled_for_client )
        disableGlobalShortcuts( false );
    else
        disableGlobalShortcuts( true );
    }

static bool pending_dfc = false;

void Workspace::disableGlobalShortcutsForClient( bool disable )
    {
    if( global_shortcuts_disabled_for_client == disable )
        return;
    if( !global_shortcuts_disabled )
        {
        if( disable )
            pending_dfc = true;
        KGlobalSettings::self()->emitChange( KGlobalSettings::BlockShortcuts, disable );
        // kwin will get the kipc message too
        }
    }

void Workspace::disableGlobalShortcuts( bool disable )
    {
    KGlobalSettings::self()->emitChange( KGlobalSettings::BlockShortcuts, disable );
    // kwin will get the kipc message too
    }

void Workspace::slotBlockShortcuts( int data )
    {
    if( pending_dfc && data )
        {
        global_shortcuts_disabled_for_client = true;
        pending_dfc = false;
        }
    else
        {
        global_shortcuts_disabled = data;
        global_shortcuts_disabled_for_client = false;
        }
    // update also Alt+LMB actions etc.
    for( ClientList::ConstIterator it = clients.begin();
         it != clients.end();
         ++it )
        (*it)->updateMouseGrab();
    }

// Optimized version of QCursor::pos() that tries to avoid X roundtrips
// by updating the value only when the X timestamp changes.
static QPoint last_cursor_pos;
static int last_buttons = 0;
static Time last_cursor_timestamp = CurrentTime;
static QTimer* last_cursor_timer;

QPoint Workspace::cursorPos() const
    {
    if( last_cursor_timestamp == CurrentTime
        || last_cursor_timestamp != QX11Info::appTime())
        {
        last_cursor_timestamp = QX11Info::appTime();
        Window root;
        Window child;
        int root_x, root_y, win_x, win_y;
        uint state;
        XQueryPointer( display(), rootWindow(), &root, &child,
            &root_x, &root_y, &win_x, &win_y, &state );
        last_cursor_pos = QPoint( root_x, root_y );
        last_buttons = state;
        if( last_cursor_timer == NULL )
            {
            Workspace* ws = const_cast< Workspace* >( this );
            last_cursor_timer = new QTimer( ws );
            last_cursor_timer->setSingleShot( true );
            connect( last_cursor_timer, SIGNAL( timeout()), ws, SLOT( resetCursorPosTime()));
            }
        last_cursor_timer->start( 0 );
        }
    return last_cursor_pos;
    }

// Because of QTimer's and the impossibility to get events for all mouse
// movements (at least I haven't figured out how) the position needs
// to be also refetched after each return to the event loop.
void Workspace::resetCursorPosTime()
    {
    last_cursor_timestamp = CurrentTime;
    }

void Workspace::checkCursorPos()
    {
    QPoint last = last_cursor_pos;
    int lastb = last_buttons;
    cursorPos(); // update if needed
    if( last != last_cursor_pos || lastb != last_buttons )
        static_cast< EffectsHandlerImpl* >( effects )->mouseChanged( cursorPos(), last,
            x11ToQtMouseButtons( last_buttons ), x11ToQtMouseButtons( lastb ),
            x11ToQtKeyboardModifiers( last_buttons ), x11ToQtKeyboardModifiers( lastb ));
    }

} // namespace

#include "workspace.moc"
