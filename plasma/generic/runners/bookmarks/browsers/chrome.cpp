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


#include "chrome.h"
#include "browsers/bookmarksfinder.h"
#include <qjson/parser.h>
#include <QFileInfo>
#include <KDebug>
#include "bookmarksrunner_defs.h"

Chrome::Chrome( BookmarksFinder* bookmarksFinder, QObject* parent ): Browser(parent), m_bookmarksFiles(bookmarksFinder->find())
{
}

QList<BookmarkMatch> Chrome::match(const QString &term, bool addEveryThing)
{
    QList<BookmarkMatch> results;
    foreach(QVariantMap bookmark, m_bookmarks) {
        BookmarkMatch bookmarkMatch(defaultIcon(), term, bookmark.value("name").toString(), bookmark.value("url").toString());
        bookmarkMatch.addTo(results, addEveryThing);
    }
    return results;
}

void Chrome::prepare()
{
    QJson::Parser parser;
    bool ok;
    foreach(QString filename, m_bookmarksFiles) {
        QFile bookmarksFile(filename);
        QVariant result = parser.parse(&bookmarksFile, &ok);
        if(!ok || !result.toMap().contains("roots")) {
            return;
        }
        QVariantMap entries = result.toMap().value("roots").toMap();
        foreach(QVariant folder, entries.values()) {
            parseFolder(folder.toMap());
        }
    }
    kDebug(kdbg_code) << "Found " << m_bookmarks.count() << " bookmarks on profiles " << m_bookmarksFiles;
}

void Chrome::teardown()
{
    m_bookmarks.clear();
}

void Chrome::parseFolder(const QVariantMap &entry)
{
    QVariantList children = entry.value("children").toList();
    foreach(QVariant child, children) {
        QVariantMap entry = child.toMap();
        if(entry.value("type").toString() == "folder")
            parseFolder(entry);
        else
            m_bookmarks << entry;
    }
}
