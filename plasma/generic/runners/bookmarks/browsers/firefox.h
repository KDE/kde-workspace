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

#ifndef FIREFOX_H
#define FIREFOX_H

#include <QSqlDatabase>
#include "browser.h"

class KJob;
class Favicon;
class FetchSqlite;
class Firefox : public QObject, public Browser
{
    Q_OBJECT
public:
    explicit Firefox(QObject *parent = 0);
    virtual ~Firefox();
    virtual QList<BookmarkMatch> match(const QString& term, bool addEverything);
public slots:
    virtual void teardown();
    virtual void prepare();
private:
    virtual void reloadConfiguration();
    QString m_dbFile;
    QString m_dbCacheFile;
    Favicon * m_favicon;
    FetchSqlite *m_fetchsqlite;
};

#endif // FIREFOX_H
