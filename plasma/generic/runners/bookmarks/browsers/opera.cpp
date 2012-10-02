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

#include "opera.h"
#include <KDebug>
#include "bookmarksrunner_defs.h"
#include <QDir>
#include <QFile>
#include "favicon.h"


Opera::Opera(QObject* parent): QObject(parent), m_favicon(new FallbackFavicon(this))
{
}

QList<BookmarkMatch> Opera::match( const QString& term, bool addEverything )
{
    QList<BookmarkMatch> matches;

    QLatin1String nameStart("\tNAME=");
    QLatin1String urlStart("\tURL=");
    QLatin1String descriptionStart("\tDESCRIPTION=");

    // search
    foreach (const QString & entry, m_operaBookmarkEntries) {
        QStringList entryLines = entry.split("\n");
        if (!entryLines.first().startsWith(QString("#URL"))) {
            continue; // skip folder entries
        }
        entryLines.pop_front();

        QString name;
        QString url;
        QString description;

        foreach (const QString & line, entryLines) {
            if (line.startsWith(nameStart)) {
                name = line.mid( QString(nameStart).length() ).simplified();
            } else if (line.startsWith(urlStart)) {
                url = line.mid( QString(urlStart).length() ).simplified();
            } else if (line.startsWith(descriptionStart)) {
                description = line.mid(QString(descriptionStart).length())
                              .simplified();
            }
        }
        
        BookmarkMatch bookmarkMatch(m_favicon, term, name, url, description);
        bookmarkMatch.addTo(matches, addEverything);
    }
    return matches;
}


void Opera::prepare()
{
          // open bookmarks file
        QString operaBookmarksFilePath = QDir::homePath() + "/.opera/bookmarks.adr";
        QFile operaBookmarksFile(operaBookmarksFilePath);
        if (!operaBookmarksFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            kDebug(kdbg_code) << "Could not open Operas Bookmark File " + operaBookmarksFilePath;
            return;
        }

        // check format
        QString firstLine = operaBookmarksFile.readLine();
        if (firstLine.compare("Opera Hotlist version 2.0\n")) {
            kDebug(kdbg_code) << "Format of Opera Bookmarks File might have changed.";
        }
        operaBookmarksFile.readLine(); // skip options line ("Options: encoding = utf8, version=3")
        operaBookmarksFile.readLine(); // skip empty line

        // load contents
        QString contents = operaBookmarksFile.readAll();
        m_operaBookmarkEntries = contents.split("\n\n", QString::SkipEmptyParts);

        // close file
        operaBookmarksFile.close();
}

void Opera::teardown()
{
  m_operaBookmarkEntries.clear();
}

