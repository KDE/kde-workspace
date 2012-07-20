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


#include "chromebookmarksfinder.h"
#include <QDir>
#include <qjson/parser.h>
#include <QVariantMap>
#include <KDebug>
#include "bookmarksrunner_defs.h"
#include <QFileInfo>

ChromeBookmarksFinder::ChromeBookmarksFinder (const QString &applicationName, const QString &homeDirectory, QObject* parent )
    : QObject(parent), m_applicationName(applicationName), m_homeDirectory(homeDirectory)
{
}

QStringList ChromeBookmarksFinder::find()
{
  QString configDirectory = QString("%1/.config/%2")
            .arg(m_homeDirectory).arg(m_applicationName);
  QString localStateFileName = QString("%1/Local State")
          .arg(configDirectory);

  QStringList profiles;
  QJson::Parser parser;
  bool ok;
  QFile localStateFile(localStateFileName);

  QVariantMap localState = parser.parse(&localStateFile, &ok).toMap();
  if(!ok) {
      kDebug(kdbg_code) << "error opening " << QFileInfo(localStateFile).absoluteFilePath();
      return profiles;
  }

  QVariantMap profilesConfig = localState.value("profile").toMap().value("info_cache").toMap();

  foreach(QString profile, profilesConfig.keys())
      profiles << QString("%1/%2/Bookmarks").arg(configDirectory).arg(profile);

  return profiles;
}
