/*
  Copyright (c) 2009 KDAB
  Author: Frank Osterfeld <osterfeld@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "calfilterproxymodel.h"
#include "calendarmodel.h"

#include <Akonadi/Item>

#include <KCalCore/CalFilter>
#include <KCalCore/Incidence>

using namespace CalendarSupport;

class CalFilterProxyModel::Private
{
  public:
    explicit Private() : filter( 0 ) {}
    KCalCore::CalFilter *filter;
};

CalFilterProxyModel::CalFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent ), d( new Private )
{
  setSortRole( CalendarModel::SortRole );
  setFilterKeyColumn( 0 );
}

CalFilterProxyModel::~CalFilterProxyModel()
{
  delete d;
}

KCalCore::CalFilter *CalFilterProxyModel::filter() const
{
  return d->filter;
}

void CalFilterProxyModel::setFilter( KCalCore::CalFilter *filter )
{
  if ( filter == d->filter ) {
    return;
  }

  d->filter = filter;
  invalidateFilter();
}

bool CalFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( !d->filter ) {
    return true;
  }
  if ( source_row < 0 || !source_parent.isValid() ) {
    return false;
  }

  const QModelIndex idx = sourceModel()->index( source_row, 0, source_parent );
  if ( !idx.isValid() ) {
    return false;
  }

  const Akonadi::Item item = idx.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( !item.isValid() || !item.hasPayload<KCalCore::Incidence::Ptr>() ) {
    return false;
  }

  const KCalCore::Incidence::Ptr inc = item.payload<KCalCore::Incidence::Ptr>();
  if ( !inc ) {
    return false;
  }

  return d->filter->filterIncidence( inc );
}
