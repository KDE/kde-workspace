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
      if (m_db.isValid()) {
        if (m_dbCacheFile.isEmpty()) {
            m_dbCacheFile = KStandardDirs::locateLocal("cache", "") + "bookmarkrunnerfirefoxdbfile.sqlite";
        }

        // ### DO NOT USE KIO FROM RUNNER THREADS!
        // ### This looks like a local copy, so use QFile::copy instead.
        KIO::Job *job = KIO::file_copy(m_dbFile, m_dbCacheFile, -1,
                                       KIO::HideProgressInfo | KIO::Overwrite);
        connect(job, SIGNAL(result(KJob*)), this, SLOT(dbCopied(KJob*)));
    }
}

void Firefox::dbCopied(KJob *)
{
    m_db.setDatabaseName(m_dbCacheFile);
    m_dbOK = m_db.open();
    kDebug(kdbg_code) << "Database was opened: " << m_dbOK;
}

QList< BookmarkMatch > Firefox::match(const QString& term, bool addEverything)
{
    QList< BookmarkMatch > matches;
    if (!m_dbOK) {
        return matches;
    }
    kDebug(kdbg_code) << "Firefox bookmark: match " << term;

    QString tmpTerm = term;
    QSqlQuery query;
    if (addEverything) {
        query = QSqlQuery("SELECT moz_bookmarks.fk, moz_bookmarks.title, moz_places.url," \
                    "moz_places.favicon_id FROM moz_bookmarks, moz_places WHERE " \
                    "moz_bookmarks.type = 1 AND moz_bookmarks.fk = moz_places.id");
    } else {
        const QString escapedTerm = tmpTerm.replace('\'', "\\'");
        query = QSqlQuery("SELECT moz_bookmarks.fk, moz_bookmarks.title, moz_places.url," \
                        "moz_places.favicon_id FROM moz_bookmarks, moz_places WHERE " \
                        "moz_bookmarks.type = 1 AND moz_bookmarks.fk = moz_places.id AND " \
                        "(moz_bookmarks.title LIKE  '%" + escapedTerm + "%' or moz_places.url LIKE '%"
                        + escapedTerm + "%')");
    }

    while (query.next() /*&& context.isValid() TODO: restore? */) {
        const QString title = query.value(1).toString();
        const QUrl url = query.value(2).toString();
        //const int favicon_id = query.value(3).toInt();
        // TODO: icon is actually stored in sqlite as blob. Fetch it?
        
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
    if (m_db.isOpen()) {
        m_db.close();
        m_dbOK = false;
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
                m_db = QSqlDatabase();
                return;
            }
	    kDebug(kdbg_code) << "Profile " << profilePath << " found";
            profilePath.prepend(QString("%1/.mozilla/firefox/").arg(QDir::homePath()));
            m_dbFile = profilePath + "/places.sqlite";
            grp.writeEntry("dbfile", m_dbFile);
        }
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        m_db.setHostName("localhost");
    } else {
        kDebug(kdbg_code) << "SQLITE driver isn't available";
        m_db = QSqlDatabase();
    }
}
