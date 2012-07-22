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

#include "chromefavicon.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <KDebug>
#include <QPixmap>
#include <QFile>
#include <QFileInfo>
#include <KStandardDirs>
#include <QPainter>
#include <QDir>
#include "bookmarksrunner_defs.h"

#define dbFileName m_profileCacheDirectory + QDir::separator() + "Favicons.sqlite"

ChromeFavicon::ChromeFavicon(const QString &profileDirectory, QObject *parent) :
    Favicon(parent)
{
    kDebug(kdbg_code) << "Received profile directory: " << profileDirectory;
    m_db = QSqlDatabase::addDatabase("QSQLITE", profileDirectory);
    m_db.setHostName("localhost");

    m_profileCacheDirectory = QString("%1/KRunner-Chrome-Favicons-%2")
            .arg(KStandardDirs::locateLocal("cache", ""))
            .arg(QFileInfo(profileDirectory).fileName());
    cleanCacheDirectory();
    QDir().mkpath(m_profileCacheDirectory);
    QFile profileFaviconSqlite(profileDirectory + "/Favicons");
    QFile(dbFileName).remove();
    bool couldCopy = profileFaviconSqlite.copy(dbFileName);
    if(!couldCopy) {
        kDebug(kdbg_code) << "error copying favicon database from " << profileFaviconSqlite.fileName() << " to " << dbFileName;
        kDebug(kdbg_code) << profileFaviconSqlite.errorString();
    }
}

ChromeFavicon::~ChromeFavicon()
{
    cleanCacheDirectory();
}

void ChromeFavicon::prepare()
{
    m_db.setDatabaseName(dbFileName);
    bool ok = m_db.open();
    kDebug(kdbg_code) << "Chrome favicon Database " << dbFileName << " was opened: " << ok;
    if(!ok) {
        kDebug(kdbg_code) << "Error: " << m_db.lastError().text();
    }
}

void ChromeFavicon::teardown()
{
    m_db.close();
}

void ChromeFavicon::cleanCacheDirectory()
{
    foreach(QFileInfo file, QDir(m_profileCacheDirectory).entryInfoList()) {
        kDebug(kdbg_code) << "Removing file " << file.absoluteFilePath() << ": " <<
                             QFile(file.absoluteFilePath()).remove();
    }
    QDir().rmdir(m_profileCacheDirectory);
}

QIcon ChromeFavicon::iconFor(const QString &url)
{
    kDebug(kdbg_code) << "got url: " << url << "; querying database " << m_db.databaseName();
    QString fileChecksum = QString("%1").arg(qChecksum(url.toAscii(), url.toAscii().size()));
    QFile iconFile( m_profileCacheDirectory + QDir::separator() + fileChecksum + "_favicon" );

    kDebug(kdbg_code) << "Looking for " << iconFile.fileName() << ", url: " << url;
    if(!iconFile.exists()) {
        kDebug(kdbg_code) << "icon is not cached, querying to sqlite";
        QSqlQuery query(m_db);
        query.prepare("SELECT * FROM favicons inner join icon_mapping on icon_mapping.icon_id = favicons.id WHERE page_url = :url;");
        query.bindValue("url", url);
        if(!query.exec()) {
            QSqlError error = m_db.lastError();
            kDebug(kdbg_code) << "query failed: " << error.text() << " (" << error.type() << ", " << error.number() << ")";
            kDebug(kdbg_code) << query.lastQuery();
        }
        if(!query.next()) return defaultIcon();
        kDebug(kdbg_code) << "icon found in sqlite";

        QSqlRecord record = query.record();
        kDebug(kdbg_code) << "Found favicon: " << record.value("page_url");
        QByteArray iconData = record.value("image_data").toByteArray();
        kDebug(kdbg_code) << "Data loaded: " << iconData.size() << " bytes";

        iconFile.open(QFile::WriteOnly);
        iconFile.write(iconData);
        iconFile.close();
    }
    return QIcon(iconFile.fileName());
}
