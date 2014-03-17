/*
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "actions.h"

#include <KConfigGroup>
#include <KDebug>
#include <KProcess>

namespace KHotKeys {

DBusActionVisitor::~DBusActionVisitor()
    {}


DBusAction::DBusAction( ActionData* data_P, const QString& app_P, const QString& obj_P,
    const QString& call_P, const QString& args_P )
    : Action( data_P ), _application( app_P ), _object( obj_P ), _function( call_P ), _arguments( args_P )
    {
    }


void DBusAction::accept(ActionVisitor& visitor)
    {
    if (DBusActionVisitor *v = dynamic_cast<DBusActionVisitor*>(&visitor))
        {
        v->visit(*this);
        }
    else
        {
        kDebug() << "Visitor error";
        }
    }


const QString DBusAction::remote_application() const
    {
    return _application;
    }


const QString DBusAction::remote_object() const
    {
    return _object;
    }


const QString DBusAction::called_function() const
    {
    return _function;
    }


const QString DBusAction::arguments() const
    {
    return _arguments;
    }


void DBusAction::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "DBUS" ); // overwrites value set in base::cfg_write()
    cfg_P.writeEntry( "RemoteApp", _application );
    cfg_P.writeEntry( "RemoteObj", _object );
    cfg_P.writeEntry( "Call", _function );
    cfg_P.writeEntry( "Arguments", _arguments );
    }


void DBusAction::execute()
    {
    if( _application.isEmpty() || _object.isEmpty() || _function.isEmpty())
        return;
    QStringList args_list;
    QString args_str = _arguments;
    while( !args_str.isEmpty())
        {
        int pos = 0;
        while( args_str[ pos ] == ' ' )
            ++pos;
        if( args_str[ pos ] == '\"' || args_str[ pos ] == '\'' )
            {
            QString val = "";
            QChar sep = args_str[ pos ];
            bool skip = false;
            ++pos;
            for(;
                 pos < args_str.length();
                 ++pos )
                {
                if( args_str[ pos ] == '\\' )
                    {
                    skip = true;
                    continue;
                    }
                if( !skip && args_str[ pos ] == sep )
                    break;
                skip = false;
                val += args_str[ pos ];
                }
            if( pos >= args_str.length())
                return;
            ++pos;
            args_str = args_str.mid( pos );
            args_list.append( val );
            }
        else
            {
            // one word
            if( pos != 0 )
                args_str = args_str.mid( pos );
            int nxt_pos = args_str.indexOf( ' ' );
            args_list.append( args_str.left( nxt_pos )); // should be ok if nxt_pos is -1
            args_str = nxt_pos >= 0 ? args_str.mid( nxt_pos ) : "";
            }
        }
    kDebug() << "D-Bus call:" << _application << ":" << _object << ":" << _function << ":" << args_list;
    KProcess proc;
    proc << "qdbus" << _application << _object << _function << args_list;
    proc.startDetached();
    }


const QString DBusAction::description() const
    {
    return i18n( "D-Bus: " ) + remote_application() + "::" + remote_object() + "::"
        + called_function();
    }


Action* DBusAction::copy( ActionData* data_P ) const
    {
    return new DBusAction( data_P, remote_application(), remote_object(),
        called_function(), arguments());
    }

void DBusAction::set_arguments( const QString &arguments )
    {
    _arguments = arguments;
    }


void DBusAction::set_called_function( const QString &function )
    {
    _function = function;
    }


void DBusAction::set_remote_application( const QString &application )
    {
    _application = application;
    }


void DBusAction::set_remote_object( const QString &object )
    {
    _object = object;
    }


} // namespace KHotKeys

