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

ChromeFavicon::ChromeFavicon(const QString &profileDirectory, QObject *parent) :
    Favicon(parent)
{
    m_profileCacheDirectory = QString("%1/KRunner-Chrome-Favicons-%2")
            .arg(KStandardDirs::locateLocal("cache", ""))
            .arg(QFileInfo(profileDirectory).fileName());
    cleanCacheDirectory();
    QDir().mkpath(m_profileCacheDirectory);
    m_fetchsqlite = new FetchSqlite(profileDirectory + "/Favicons", dbFileName, this);
}

ChromeFavicon::~ChromeFavicon()
{
    cleanCacheDirectory();
}

void ChromeFavicon::prepare()
{
    m_fetchsqlite->prepare();
}

void ChromeFavicon::teardown()
{
    m_fetchsqlite->teardown();
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
        QString query = "SELECT * FROM favicons inner join icon_mapping on icon_mapping.icon_id = favicons.id WHERE page_url = :url LIMIT 1;";
        QMap<QString,QVariant> bindVariables;
        bindVariables.insert("url", url);
        QList<QVariantMap> faviconFound = m_fetchsqlite->query(query, bindVariables);
        if(!faviconFound.size()>0) return defaultIcon();
        kDebug(kdbg_code) << "icon found in sqlite";

        QByteArray iconData = faviconFound.first().value("image_data").toByteArray();
        kDebug(kdbg_code) << "Data loaded: " << iconData.size() << " bytes";

        iconFile.open(QFile::WriteOnly);
        iconFile.write(iconData);
        iconFile.close();
    }
    return QIcon(iconFile.fileName());
}
