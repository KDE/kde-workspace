/*
    Copyright 2008 Marco Martin <notmart@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "bookmarksdelegate.h"

#include <cmath>
#include <math.h>

// Qt
#include <QModelIndex>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionViewItem>

// KDE
#include <QDebug>
#include <KColorScheme>
#include <KIcon>
#include <KIconLoader>

// plasma
#include <Plasma/PaintUtils>
#include <Plasma/Theme>


class BookmarksDelegatePrivate
{
public:
    BookmarksDelegatePrivate() {
    }

    ~BookmarksDelegatePrivate() {
    }
};




BookmarksDelegate::BookmarksDelegate(QObject *parent)
        : QStyledItemDelegate(parent),
        d(/*new BookmarksDelegatePrivate*/0)
{
}

BookmarksDelegate::~BookmarksDelegate()
{
    delete d;
}

void BookmarksDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    if (option.state & (QStyle::State_MouseOver | QStyle::State_Selected)) {
        QRect destroyIconRect = QStyle::alignedRect(option.direction,
                                            option.decorationPosition == QStyleOptionViewItem::Left ?
                                            Qt::AlignRight : Qt::AlignLeft,
                                            QSize(option.rect.height(), option.rect.height()),
                                            option.rect);
        painter->drawPixmap(destroyIconRect, KIcon("list-remove").pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    }
}

bool BookmarksDelegate::editorEvent(QEvent *event,
                               QAbstractItemModel *model,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index)
{
    QRect destroyIconRect = QStyle::alignedRect(option.direction,
                                          option.decorationPosition == QStyleOptionViewItem::Left ?
                                          Qt::AlignRight : Qt::AlignLeft,
                                          QSize(option.rect.height(), option.rect.height()),
                                          option.rect);

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (destroyIconRect.contains(mouseEvent->pos())) {
            emit destroyBookmark(index);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

