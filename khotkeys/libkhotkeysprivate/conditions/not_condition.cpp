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

#include "conditions.h"

#include <KConfigGroup>

namespace KHotKeys {

Not_condition::Not_condition( KConfigGroup& cfg_P, Condition_list_base* parent_P )
    : Condition_list_base( cfg_P, parent_P )
    {}


Not_condition::Not_condition( Condition_list_base* parent_P )
    : Condition_list_base( parent_P )
    {}


void Not_condition::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "NOT" ); // overwrites value set in base::cfg_write()
    }


const Condition* Not_condition::condition() const
    {
    return first();
    }


Not_condition* Not_condition::copy() const
    {
    Not_condition* ret = new Not_condition();
    if (!isEmpty())
        {
        ret->append(condition()->copy());
        }

    return ret;
    }


const QString Not_condition::description() const
    {
    return i18nc( "Not_condition", "Not" );
    }


bool Not_condition::accepts_children() const
    {
    return count() == 0;
    }


bool Not_condition::match() const
    {
    return condition() ? !condition()->match() : false;
    }

} // namespace KHotKeys
