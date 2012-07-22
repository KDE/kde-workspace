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

#ifndef CHROMEFAVICON_H
#define CHROMEFAVICON_H

#include <QIcon>
#include <QSqlDatabase>
#include "favicon.h"

class ChromeFavicon : public Favicon
{
    Q_OBJECT
public:
    explicit ChromeFavicon(const QString &profileDirectory, QObject *parent = 0);
    ~ChromeFavicon();
    virtual QIcon iconFor(const QString &url);

public slots:
    virtual void prepare();
    virtual void teardown();

private:
    QSqlDatabase m_db;
    QString m_profileCacheDirectory;
    void cleanCacheDirectory();
};

#endif // CHROMEFAVICON_H
