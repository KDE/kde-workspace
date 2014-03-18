#ifndef __FC_QUERY_H__
#define __FC_QUERY_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include "Misc.h"

class QProcess;

namespace KFI
{

class CFcQuery : public QObject
{
    Q_OBJECT

    public:

    CFcQuery(QObject *parent) : QObject(parent), itsProc(NULL) { }
    ~CFcQuery();

    void run(const QString &query);

    const QString & font() const { return itsFont; }
    const QString & file() const { return itsFile; }

    private Q_SLOTS:

    void procExited();
    void data();

    Q_SIGNALS:

    void finished();

    private:

    QProcess   *itsProc;
    QByteArray itsBuffer;
    QString    itsFile,
               itsFont;
};

}

#endif

