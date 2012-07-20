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

#ifndef BROWSER_H
#define BROWSER_H
#include <QObject>
#include <QString>
#include <Plasma/AbstractRunner>
#include <KIcon>
#include <KUrl>
#include "bookmarkmatch.h"

class Browser;

class BrowserFactory : public QObject
{
    Q_OBJECT
public:
    BrowserFactory(QObject *parent =0);
    Browser *find(const QString &browserName, QObject *parent = 0);
private:
  Browser *m_previousBrowser;
  QString m_previousBrowserName;
};


class Browser: public QObject
{
    Q_OBJECT
public:
    Browser(QObject *parent = 0);
    virtual ~Browser();
    virtual QList<BookmarkMatch> match(const QString& term, bool addEveryThing) = 0;
protected:
    inline KIcon defaultIcon() const { return m_default_icon; }
private:
    KIcon const m_default_icon;

public slots:
    virtual void prepare();
    virtual void teardown();
};


#endif // BROWSER_H
