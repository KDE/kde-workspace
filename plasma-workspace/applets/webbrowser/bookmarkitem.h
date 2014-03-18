/*  Copyright (C) 2008 Marco Martin <notmart@gmail.com>

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

#ifndef BOOKMARKITEM_H
#define BOOKMARKITEM_H


#include <QStandardItem>
#include <QtCore/QModelIndex>
#include <kbookmark.h>


class BookmarkItem : public QStandardItem
{

public:
    enum BookmarkRoles
    {
        UrlRole = Qt::UserRole+1
    };

    BookmarkItem(KBookmark &bookmark);
    ~BookmarkItem();

    KBookmark bookmark() const;
    void setBookmark(const KBookmark &bookmark);
    QVariant data(int role) const;


private:
    KBookmark m_bookmark;
};

#endif
