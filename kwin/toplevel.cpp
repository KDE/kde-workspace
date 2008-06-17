/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>

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

#include "toplevel.h"

#include <kxerrorhandler.h>

#include "atoms.h"
#include "client.h"
#include "effects.h"

namespace KWin
{

Toplevel::Toplevel( Workspace* ws )
    : vis( NULL )
    , info( NULL )
    , ready_for_painting( true )
    , client( None )
    , frame( None )
    , wspace( ws )
    , window_pix( None )
#ifdef HAVE_XDAMAGE
    , damage_handle( None )
#endif
    , is_shape( false )
    , effect_window( NULL )
    , wmClientLeaderWin( 0 )
    {
    }

Toplevel::~Toplevel()
    {
#ifdef HAVE_XDAMAGE
    assert( damage_handle == None );
#endif
    discardWindowPixmap();
    delete info;
    }

kdbgstream& operator<<( kdbgstream& stream, const Toplevel* cl )
    {
    if( cl == NULL )
        return stream << "\'NULL\'";
    cl->debug( stream );
    return stream;
    }

kdbgstream& operator<<( kdbgstream& stream, const ToplevelList& list )
    {
    stream << "LIST:(";
    bool first = true;
    for( ToplevelList::ConstIterator it = list.begin();
         it != list.end();
         ++it )
        {
        if( !first )
            stream << ":";
        first = false;
        stream << *it;
        }
    stream << ")";
    return stream;
    }

kdbgstream& operator<<( kdbgstream& stream, const ConstToplevelList& list )
    {
    stream << "LIST:(";
    bool first = true;
    for( ConstToplevelList::ConstIterator it = list.begin();
         it != list.end();
         ++it )
        {
        if( !first )
            stream << ":";
        first = false;
        stream << *it;
        }
    stream << ")";
    return stream;
    }

void Toplevel::detectShape( Window id )
    {
    is_shape = Extensions::hasShape( id );
    }

// used only by Deleted::copy()
void Toplevel::copyToDeleted( Toplevel* c )
    {
    geom = c->geom;
    vis = c->vis;
    bit_depth = c->bit_depth;
    info = c->info;
    client = c->client;
    frame = c->frame;
    wspace = c->wspace;
    window_pix = c->window_pix;
    ready_for_painting = c->ready_for_painting;
#ifdef HAVE_XDAMAGE
    damage_handle = None;
#endif
    damage_region = c->damage_region;
    repaints_region = c->repaints_region;
    is_shape = c->is_shape;
    effect_window = c->effect_window;
    if( effect_window != NULL )
        effect_window->setWindow( this );
    resource_name = c->resourceName();
    resource_class = c->resourceClass();
    client_machine = c->wmClientMachine( false );
    wmClientLeaderWin = c->wmClientLeader();
    window_role = c->windowRole();
    // this needs to be done already here, otherwise 'c' could very likely
    // call discardWindowPixmap() in something called during cleanup
    c->window_pix = None;
    }

// before being deleted, remove references to everything that's now
// owner by Deleted
void Toplevel::disownDataPassedToDeleted()
    {
    info = NULL;
    }

NET::WindowType Toplevel::windowType( bool direct, int supported_types ) const
    {
    if( supported_types == 0 )
        supported_types = dynamic_cast< const Client* >( this ) != NULL
            ? SUPPORTED_MANAGED_WINDOW_TYPES_MASK : SUPPORTED_UNMANAGED_WINDOW_TYPES_MASK;
    NET::WindowType wt = info->windowType( supported_types );
    if( direct )
        return wt;
    const Client* cl = dynamic_cast< const Client* >( this );
#warning TODO
//    NET::WindowType wt2 = rules()->checkType( wt );
    NET::WindowType wt2 = wt;
    if( wt != wt2 )
        {
        wt = wt2;
        info->setWindowType( wt ); // force hint change
        }
    // hacks here
    if( wt == NET::Menu && cl != NULL )
        {
        // ugly hack to support the times when NET::Menu meant NET::TopMenu
        // if it's as wide as the screen, not very high and has its upper-left
        // corner a bit above the screen's upper-left cornet, it's a topmenu
        if( x() == 0 && y() < 0 && y() > -10 && height() < 100
            && abs( width() - workspace()->clientArea( FullArea, cl ).width()) < 10 )
            wt = NET::TopMenu;
        }
    // TODO change this to rule
    const char* const oo_prefix = "openoffice.org"; // QByteArray has no startsWith()
    // oo_prefix is lowercase, because resourceClass() is forced to be lowercase
    if( qstrncmp( resourceClass(), oo_prefix, strlen( oo_prefix )) == 0 && wt == NET::Dialog )
        wt = NET::Normal; // see bug #66065
    if( wt == NET::Unknown && cl != NULL ) // this is more or less suggested in NETWM spec
        wt = cl->isTransient() ? NET::Dialog : NET::Normal;
    return wt;
    }

void Toplevel::getWindowRole()
    {
    window_role = getStringProperty( window(), atoms->wm_window_role).toLower();
    }

/*!
  Returns SM_CLIENT_ID property for a given window.
 */
QByteArray Toplevel::staticSessionId(WId w)
    {
    return getStringProperty(w, atoms->sm_client_id);
    }

/*!
  Returns WM_COMMAND property for a given window.
 */
QByteArray Toplevel::staticWmCommand(WId w)
    {
    return getStringProperty(w, XA_WM_COMMAND, ' ');
    }

/*!
  Returns WM_CLIENT_LEADER property for a given window.
 */
Window Toplevel::staticWmClientLeader(WId w)
    {
    Atom type;
    int format, status;
    unsigned long nitems = 0;
    unsigned long extra = 0;
    unsigned char *data = 0;
    Window result = w;
    KXErrorHandler err;
    status = XGetWindowProperty( display(), w, atoms->wm_client_leader, 0, 10000,
                                 false, XA_WINDOW, &type, &format,
                                 &nitems, &extra, &data );
    if (status == Success && !err.error( false )) 
        {
        if (data && nitems > 0)
            result = *((Window*) data);
        XFree(data);
        }
    return result;
    }


void Toplevel::getWmClientLeader()
    {
    wmClientLeaderWin = staticWmClientLeader(window());
    }

/*!
  Returns sessionId for this client,
  taken either from its window or from the leader window.
 */
QByteArray Toplevel::sessionId()
    {
    QByteArray result = staticSessionId(window());
    if (result.isEmpty() && wmClientLeaderWin && wmClientLeaderWin!=window())
        result = staticSessionId(wmClientLeaderWin);
    return result;
    }

/*!
  Returns command property for this client,
  taken either from its window or from the leader window.
 */
QByteArray Toplevel::wmCommand()
    {
    QByteArray result = staticWmCommand(window());
    if (result.isEmpty() && wmClientLeaderWin && wmClientLeaderWin!=window())
        result = staticWmCommand(wmClientLeaderWin);
    return result;
    }

void Toplevel::getWmClientMachine()
    {
    client_machine = getStringProperty(window(), XA_WM_CLIENT_MACHINE);
    if( client_machine.isEmpty() && wmClientLeaderWin && wmClientLeaderWin!=window())
        client_machine = getStringProperty(wmClientLeaderWin, XA_WM_CLIENT_MACHINE);
    if( client_machine.isEmpty())
        client_machine = "localhost";
    }

/*!
  Returns client machine for this client,
  taken either from its window or from the leader window.
*/
QByteArray Toplevel::wmClientMachine( bool use_localhost ) const
    {
    QByteArray result = client_machine;
    if( use_localhost )
        { // special name for the local machine (localhost)
        if( result != "localhost" && isLocalMachine( result ))
            result = "localhost";
        }
    return result;
    }

/*!
  Returns client leader window for this client.
  Returns the client window itself if no leader window is defined.
*/
Window Toplevel::wmClientLeader() const
    {
    if (wmClientLeaderWin)
        return wmClientLeaderWin;
    return window();
    }

void Toplevel::getResourceClass()
    {
    XClassHint classHint;
    if( XGetClassHint( display(), window(), &classHint ) ) 
        {
        // Qt3.2 and older had this all lowercase, Qt3.3 capitalized resource class.
        // Force lowercase, so that workarounds listing resource classes still work.
        resource_name = QByteArray( classHint.res_name ).toLower();
        resource_class = QByteArray( classHint.res_class ).toLower();
        XFree( classHint.res_name );
        XFree( classHint.res_class );
        }
    else
        {
        resource_name = resource_class = QByteArray();
        }
    }

double Toplevel::opacity() const
    {
    if( info->opacity() == 0xffffffff )
        return 1.0;
    return info->opacity() * 1.0 / 0xffffffff;
    }

void Toplevel::setOpacity( double new_opacity )
    {
    double old_opacity = opacity();
    new_opacity = qBound( 0.0, new_opacity, 1.0 );
    if( old_opacity == new_opacity )
        return;
    info->setOpacity( static_cast< unsigned long >( new_opacity * 0xffffffff ));
    if( compositing())
        {
        addRepaintFull();
        scene->windowOpacityChanged( this );
        if( effects )
            static_cast<EffectsHandlerImpl*>(effects)->windowOpacityChanged( effectWindow(), old_opacity );
        }
    }

void Toplevel::deleteEffectWindow()
    {
    delete effect_window;
    effect_window = NULL;
    }

} // namespace

#include "toplevel.moc"
