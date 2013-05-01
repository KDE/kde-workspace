/**************************************************************************
 * Copyright (C) 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>     *
 * Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>               *
 *                                                                        *
 * This program is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU General Public License            *
 * as published by the Free Software Foundation; either version 2         *
 * of the License, or (at your option) any later version.                 *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA          *
 * 02110-1301, USA.                                                       *
***************************************************************************/

#include "PredicateModel.h"

#include "PredicateItem.h"

class PredicateModel::Private {
public:
    Private() {}

    PredicateItem * rootItem;
};

PredicateModel::PredicateModel( PredicateItem * menuRoot, QObject *parent )
    : QAbstractItemModel( parent )
    , d( new Private() )
{
    d->rootItem = menuRoot;
}

PredicateModel::~PredicateModel()
{
    delete d;
}

int PredicateModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return 1;
}

int PredicateModel::rowCount( const QModelIndex &parent ) const
{
    PredicateItem * mi;
    if ( parent.isValid() ) {
        mi = static_cast<PredicateItem*>( parent.internalPointer() );
    } else {
        mi = d->rootItem;
    }

    return mi->children().count();
}

QVariant PredicateModel::data( const QModelIndex &index, int role ) const
{
    PredicateItem * mi = 0;
    QVariant theData;
    if ( !index.isValid() ) {
        return QVariant();
    }

    mi = static_cast<PredicateItem*>( index.internalPointer() );
    switch ( role ) {
        case Qt::DisplayRole:
            theData.setValue( mi->prettyName() );
            break;
        case Qt::UserRole:
            theData.setValue( mi );
            break;
        default:
            break;
    }
    return theData;
}

Qt::ItemFlags PredicateModel::flags( const QModelIndex &index ) const
{
    if ( !index.isValid() ) {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex PredicateModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( !hasIndex(row, column, parent) ) {
        return QModelIndex();
    }

    PredicateItem *parentItem;
    if ( !parent.isValid() ) {
        parentItem = d->rootItem;
    } else {
        parentItem = static_cast<PredicateItem*>( parent.internalPointer() );
    }

    PredicateItem *childItem = parentItem->children().value(row);
    if ( childItem ) {
        return createIndex( row, column, childItem );
    } else {
        return QModelIndex();
    }
}

QModelIndex PredicateModel::parent( const QModelIndex &index ) const
{
    PredicateItem *childItem = static_cast<PredicateItem*>( index.internalPointer() );
    if( !childItem ) {
        return QModelIndex();
    }

    PredicateItem * parent = childItem->parent();
    PredicateItem * grandParent = parent->parent();

    int childRow = 0;
    if( grandParent ) {
        childRow = grandParent->children().indexOf( parent );
    }

    if ( parent == d->rootItem ) {
        return QModelIndex();
    }
    return createIndex( childRow, 0, parent );
}

PredicateItem * PredicateModel::rootItem() const
{
    return d->rootItem;
}

void PredicateModel::setRootPredicate( PredicateItem * item )
{
    d->rootItem = item;
    reset();
}

void PredicateModel::itemUpdated( const QModelIndex& item )
{
    emit dataChanged( item, item );
}

void PredicateModel::childrenChanging( const QModelIndex& item, Solid::Predicate::Type oldType )
{
    PredicateItem * currentItem = static_cast<PredicateItem*>( item.internalPointer() );
    Solid::Predicate::Type newType = currentItem->itemType;
    
    if( oldType == newType ) {
        return;
    }

    if( rowCount(item) != 0 && newType != Solid::Predicate::Conjunction && newType != Solid::Predicate::Disjunction ) {
        emit beginRemoveRows( item, 0, 1 );
        currentItem->updateChildrenStatus();
        emit endRemoveRows();
        return;
    }

    bool hasChildren = (newType == Solid::Predicate::Conjunction || newType == Solid::Predicate::Disjunction);
 
    if( rowCount(item) == 0 && hasChildren ) {
        emit beginInsertRows( item, 0, 1 );
        currentItem->updateChildrenStatus();
        emit endInsertRows();
    }
}

#include "PredicateModel.moc"
