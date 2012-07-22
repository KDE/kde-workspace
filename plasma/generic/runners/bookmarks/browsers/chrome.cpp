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
#include "faviconfromblob.h"
#include "browsers/findprofile.h"
#include <qjson/parser.h>
#include <QFileInfo>
#include <KDebug>
#include "bookmarksrunner_defs.h"
#include <QFileInfo>
#include <QDir>

class ProfileBookmarks {
public:
    ProfileBookmarks(Profile &profile) : m_profile(profile) {}
    inline QList<QVariantMap> bookmarks() { return m_bookmarks; }
    inline Profile profile() { return m_profile; }
    void tearDown() { m_profile.favicon()->teardown(); m_bookmarks.clear(); }
    void add(QVariantMap &bookmarkEntry) { m_bookmarks << bookmarkEntry; }
private:
    Profile m_profile;
    QList<QVariantMap> m_bookmarks;
};

Chrome::Chrome( FindProfile* findProfile, QObject* parent )
    : Browser(parent)
{
    foreach(Profile profile, findProfile->find()) {
        m_profileBookmarks << new ProfileBookmarks(profile);
    }
}

Chrome::~Chrome()
{
    foreach(ProfileBookmarks *profileBookmark, m_profileBookmarks) {
        delete profileBookmark;
    }
}

QList<BookmarkMatch> Chrome::match(const QString &term, bool addEveryThing)
{
    QList<BookmarkMatch> results;
    foreach(ProfileBookmarks *profileBookmarks, m_profileBookmarks) {
        results << match(term, addEveryThing, profileBookmarks);
    }
    return results;
}

QList<BookmarkMatch> Chrome::match(const QString &term, bool addEveryThing, ProfileBookmarks *profileBookmarks)
{
    QList<BookmarkMatch> results;
    foreach(QVariantMap bookmark, profileBookmarks->bookmarks()) {
        QString url = bookmark.value("url").toString();

        BookmarkMatch bookmarkMatch(profileBookmarks->profile().favicon(), term, bookmark.value("name").toString(), url);
        bookmarkMatch.addTo(results, addEveryThing);
    }
    return results;
}

void Chrome::prepare()
{
    QJson::Parser parser;
    bool ok;
    foreach(ProfileBookmarks *profileBookmarks, m_profileBookmarks) {
        Profile profile = profileBookmarks->profile();
        QFile bookmarksFile(profile.path());
        QVariant result = parser.parse(&bookmarksFile, &ok);
        if(!ok || !result.toMap().contains("roots")) {
            return;
        }
        QVariantMap entries = result.toMap().value("roots").toMap();
        foreach(QVariant folder, entries.values()) {
            parseFolder(folder.toMap(), profileBookmarks);
        }
        profile.favicon()->prepare();
    }
}

void Chrome::teardown()
{
    foreach(ProfileBookmarks *profileBookmarks, m_profileBookmarks) {
        profileBookmarks->tearDown();
    }
}

void Chrome::parseFolder(const QVariantMap &entry, ProfileBookmarks *profile)
{
    QVariantList children = entry.value("children").toList();
    foreach(QVariant child, children) {
        QVariantMap entry = child.toMap();
        if(entry.value("type").toString() == "folder")
            parseFolder(entry, profile);
        else {
            profile->add(entry);
        }
    }
}
