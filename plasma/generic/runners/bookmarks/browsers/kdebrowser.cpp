/*
 *   Copyright 2007 Glenn Ergeerts <glenn.ergeerts@telenet.be>
 *   Copyright 2012 Glenn Ergeerts <marco.gulino@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kdebrowser.h"
#include <KBookmarkManager>
#include <QStack>
#include <QIcon>
#include "bookmarkmatch.h"
#include <KUrl>
#include <KMimeType>
#include "favicon.h"


QIcon KDEFavicon::iconFor(const QString &url)  {
    const QString iconFile = KMimeType::favIconForUrl(KUrl(url));
    if (iconFile.isEmpty()) {
        return defaultIcon();
    }
    return KIcon(iconFile);
}


KDEFavicon::KDEFavicon(QObject *parent)  : Favicon(parent) {}


KDEBrowser::KDEBrowser(QObject *parent) :
    QObject(parent), m_bookmarkManager(KBookmarkManager::userBookmarksManager()), m_favicon(new KDEFavicon(this))
{
}


QList< BookmarkMatch > KDEBrowser::match(const QString& term, bool addEverything)
{
    KBookmarkGroup bookmarkGroup = m_bookmarkManager->root();

    QList< BookmarkMatch > matches;
    QStack<KBookmarkGroup> groups;

    KBookmark bookmark = bookmarkGroup.first();
    while (!bookmark.isNull()) {
//         if (!context.isValid()) {
//             return;
//         } TODO: restore?

        if (bookmark.isSeparator()) {
            bookmark = bookmarkGroup.next(bookmark);
            continue;
        }

        if (bookmark.isGroup()) { // descend
            //kDebug (kdbg_code) << "descending into" << bookmark.text();
            groups.push(bookmarkGroup);
            bookmarkGroup = bookmark.toGroup();
            bookmark = bookmarkGroup.first();

            while (bookmark.isNull() && !groups.isEmpty()) {
//                 if (!context.isValid()) {
//                     return;
//                 } TODO: restore?

                bookmark = bookmarkGroup;
                bookmarkGroup = groups.pop();
                bookmark = bookmarkGroup.next(bookmark);
            }

            continue;
        }
        
        BookmarkMatch bookmarkMatch(m_favicon, term, bookmark.text(), bookmark.url().url() );
        bookmarkMatch.addTo(matches, addEverything);

        bookmark = bookmarkGroup.next(bookmark);
        while (bookmark.isNull() && !groups.isEmpty()) {
//             if (!context.isValid()) {
//                 return;
//             } // TODO: restore?

            bookmark = bookmarkGroup;
            bookmarkGroup = groups.pop();
            //kDebug(kdbg_code) << "ascending from" << bookmark.text() << "to" << bookmarkGroup.text();
            bookmark = bookmarkGroup.next(bookmark);
        }
    }
    return matches;
}

