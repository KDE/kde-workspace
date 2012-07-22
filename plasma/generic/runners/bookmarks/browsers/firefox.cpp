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

#include "firefox.h"
#include <KJob>
#include <KStandardDirs>
#include <KDebug>
#include "bookmarksrunner_defs.h"
#include <KIO/Job>
#include <QSqlQuery>
#include <QFile>
#include <QDir>
#include "bookmarkmatch.h"
#include "favicon.h"
#include "fetchsqlite.h"
#include "faviconfromblob.h"

Firefox::Firefox(QObject *parent) :
    Browser(parent), m_favicon(new FallbackFavicon(this))
{
  reloadConfiguration();
  kDebug(kdbg_code) << "Loading Firefox Bookmarks Browser";
}


Firefox::~Firefox()
{
    if (!m_dbCacheFile.isEmpty()) {
        QFile db_CacheFile(m_dbCacheFile);
        if (db_CacheFile.exists()) {
            kDebug(kdbg_code) << "Cache file was removed: " << db_CacheFile.remove();
        }
    }
    kDebug(kdbg_code) << "Deleted Firefox Bookmarks Browser";
}

void Firefox::prepare()
{
    if (m_dbCacheFile.isEmpty()) {
        m_dbCacheFile = KStandardDirs::locateLocal("cache", "") + "bookmarkrunnerfirefoxdbfile.sqlite";
    }
      if (!m_dbFile.isEmpty()) {
          m_fetchsqlite = new FetchSqlite(m_dbFile, m_dbCacheFile);
          m_fetchsqlite->prepare();
          m_favicon = FaviconFromBlob::firefox(m_fetchsqlite, this);
    }
}

QList< BookmarkMatch > Firefox::match(const QString& term, bool addEverything)
{
    QList< BookmarkMatch > matches;
    if (!m_fetchsqlite) {
        return matches;
    }
    kDebug(kdbg_code) << "Firefox bookmark: match " << term;

    QString tmpTerm = term;
    QString query;
    if (addEverything) {
        query = QString("SELECT moz_bookmarks.fk, moz_bookmarks.title, moz_places.url," \
                    "moz_places.favicon_id FROM moz_bookmarks, moz_places WHERE " \
                    "moz_bookmarks.type = 1 AND moz_bookmarks.fk = moz_places.id");
    } else {
        const QString escapedTerm = tmpTerm.replace('\'', "\\'");
        query = QString("SELECT moz_bookmarks.fk, moz_bookmarks.title, moz_places.url," \
                        "moz_places.favicon_id FROM moz_bookmarks, moz_places WHERE " \
                        "moz_bookmarks.type = 1 AND moz_bookmarks.fk = moz_places.id AND " \
                        "(moz_bookmarks.title LIKE  '%" + escapedTerm + "%' or moz_places.url LIKE '%"
                        + escapedTerm + "%')");
    }
    QList<QVariantMap> results = m_fetchsqlite->query(query);
    foreach(QVariantMap result, results) {
        const QString title = result.value("title").toString();
        const QUrl url = result.value("url").toUrl();
        if (url.scheme().contains("place")) {
            //Don't use bookmarks with empty title, url or Firefox intern url
            kDebug(kdbg_code) << "element " << url << " was not added";
            continue;
        }

        BookmarkMatch bookmarkMatch( m_favicon, term, title, url.toString());
        bookmarkMatch.addTo(matches, addEverything);
    }

    return matches;
}


void Firefox::teardown()
{
    if(m_fetchsqlite) {
        m_fetchsqlite->teardown();
        delete m_favicon;
    }
}



void Firefox::reloadConfiguration()
{
    KConfigGroup config(KSharedConfig::openConfig("kdeglobals"), QLatin1String("General") );
    if (QSqlDatabase::isDriverAvailable("QSQLITE")) {
        KConfigGroup grp = config;
        /* This allows the user to specify a profile database */
        m_dbFile = grp.readEntry<QString>("dbfile", "");
        if (m_dbFile.isEmpty() || QFile::exists(m_dbFile)) {
            //Try to get the right database file, the default profile is used
            KConfig firefoxProfile(QDir::homePath() + "/.mozilla/firefox/profiles.ini",
                                   KConfig::SimpleConfig);
            QStringList profilesList = firefoxProfile.groupList();
            profilesList = profilesList.filter(QRegExp("^Profile\\d+$"));
            int size = profilesList.size();

            QString profilePath;
            if (size == 1) {
                // There is only 1 profile so we select it
                KConfigGroup fGrp = firefoxProfile.group(profilesList.first());
                profilePath = fGrp.readEntry("Path", "");
            } else {
                // There are multiple profiles, find the default one
                foreach(const QString & profileName, profilesList) {
                    KConfigGroup fGrp = firefoxProfile.group(profileName);
                    if (fGrp.readEntry<int>("Default", 0)) {
                        profilePath = fGrp.readEntry("Path", "");
                        break;
                    }
                }
            }

            if (profilePath.isEmpty()) {
                kDebug(kdbg_code) << "No default firefox profile found";
                return;
            }
	    kDebug(kdbg_code) << "Profile " << profilePath << " found";
            profilePath.prepend(QString("%1/.mozilla/firefox/").arg(QDir::homePath()));
            m_dbFile = profilePath + "/places.sqlite";
            grp.writeEntry("dbfile", m_dbFile);
        }
    } else {
        kDebug(kdbg_code) << "SQLITE driver isn't available";
    }
}
