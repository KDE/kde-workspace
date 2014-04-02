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

#include <kconfiggroup.h>
#include <ksharedconfig.h>

namespace KHotKeys {

ActionList::ActionList( const QString& comment_P )
    : QList< Action* >(), _comment( comment_P )
    {}


void ActionList::aboutToBeErased()
    {
    QListIterator<Action*> it(*this);
    while (it.hasNext())
        {
        it.next()->aboutToBeErased();
        }
    }

const QString& ActionList::comment() const
    {
    return _comment;
    }


ActionList::~ActionList()
    {
    while (!isEmpty())
        {
        delete takeFirst();
        }
    }


void ActionList::cfg_write( KConfigGroup& cfg_P ) const
    {
    QString save_cfg_group = cfg_P.name();
    int i = 0;
    for( ActionList::ConstIterator it = begin();
         it != end();
         ++it )
        {
        KConfigGroup group( cfg_P.config(), save_cfg_group + QString::number( i++ ) );
        (*it)->cfg_write( group );
        }
    cfg_P.writeEntry( "ActionsCount", i );
    }

} // namespace KHotKeys
