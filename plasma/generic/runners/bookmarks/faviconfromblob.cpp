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

#include "faviconfromblob.h"

#include <KDebug>
#include <QPixmap>
#include <QFile>
#include <QFileInfo>
#include <KStandardDirs>
#include <QPainter>
#include <QDir>
#include "bookmarksrunner_defs.h"
#include "fetchsqlite.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

#define dbFileName m_profileCacheDirectory + QDir::separator() + "Favicons.sqlite"

FaviconFromBlob *FaviconFromBlob::chrome(const QString &profileDirectory, QObject *parent)
{
    QString profileName = QFileInfo(profileDirectory).fileName();
    QString chromeFaviconQuery = "SELECT * FROM favicons inner join icon_mapping " \
            "on icon_mapping.icon_id = favicons.id WHERE page_url = :url LIMIT 1;";
    QString faviconCache = QString("%1/KRunner-Chrome-Favicons-%2.sqlite")
            .arg(KStandardDirs::locateLocal("cache", ""))
            .arg(profileName);
    FetchSqlite *fetchSqlite = new FetchSqlite(profileDirectory + "/Favicons", faviconCache, parent);
    return new FaviconFromBlob(profileName, chromeFaviconQuery, "image_data", fetchSqlite, parent);
}

FaviconFromBlob *FaviconFromBlob::firefox(FetchSqlite *fetchSqlite, QObject *parent)
{

    QString faviconQuery = QString("SELECT moz_favicons.data FROM moz_favicons" \
                                   " inner join moz_places ON moz_places.favicon_id = moz_favicons.id" \
                                   " WHERE moz_places.url = :url LIMIT 1;");
    return new FaviconFromBlob("firefox-default", faviconQuery, "data", fetchSqlite, parent);
}


FaviconFromBlob::FaviconFromBlob(const QString &profileName, const QString &query, const QString &blobColumn, FetchSqlite *fetchSqlite, QObject *parent)
    : Favicon(parent), m_query(query), m_blobcolumn(blobColumn), m_fetchsqlite(fetchSqlite)
{
    m_profileCacheDirectory = QString("%1/KRunner-Favicons-%2")
            .arg(KStandardDirs::locateLocal("cache", ""))
            .arg(profileName);
    kDebug(kdbg_code) << "got cache directory: " << m_profileCacheDirectory;
    cleanCacheDirectory();
    QDir().mkpath(m_profileCacheDirectory);
}

FaviconFromBlob::~FaviconFromBlob()
{
    cleanCacheDirectory();
}

void FaviconFromBlob::prepare()
{
    m_fetchsqlite->prepare();
}

void FaviconFromBlob::teardown()
{
    m_fetchsqlite->teardown();
}

void FaviconFromBlob::cleanCacheDirectory()
{
    foreach(QFileInfo file, QDir(m_profileCacheDirectory).entryInfoList(QDir::NoDotAndDotDot)) {
        kDebug(kdbg_code) << "Removing file " << file.absoluteFilePath() << ": " <<
                             QFile(file.absoluteFilePath()).remove();
    }
    QDir().rmdir(m_profileCacheDirectory);
}

QIcon FaviconFromBlob::iconFor(const QString &url)
{
    kDebug(kdbg_code) << "got url: " << url << "; querying database " << m_db.databaseName();
    QString fileChecksum = QString("%1").arg(qChecksum(url.toAscii(), url.toAscii().size()));
    QFile iconFile( m_profileCacheDirectory + QDir::separator() + fileChecksum + "_favicon" );
    if(!iconFile.exists()) {
        QMap<QString,QVariant> bindVariables;
        bindVariables.insert("url", url);
        QList<QVariantMap> faviconFound = m_fetchsqlite->query(m_query, bindVariables);
        if(!faviconFound.size()>0) return defaultIcon();

        QByteArray iconData = faviconFound.first().value(m_blobcolumn).toByteArray();
        kDebug(kdbg_code) << "Favicon found: " << iconData.size() << " bytes";

        iconFile.open(QFile::WriteOnly);
        iconFile.write(iconData);
        iconFile.close();
    }
    return QIcon(iconFile.fileName());
}

