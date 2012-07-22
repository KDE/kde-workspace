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

#include "fetchsqlite.h"
#include <QFile>
#include <KDebug>
#include "bookmarksrunner_defs.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

FetchSqlite::FetchSqlite(const QString &originalFilePath, const QString &copyTo, QObject *parent) :
    QObject(parent), m_databaseFile(copyTo)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", originalFilePath);
    m_db.setHostName("localhost");

    QFile originalFile(originalFilePath);
    QFile(copyTo).remove();
    bool couldCopy = originalFile.copy(copyTo);
    if(!couldCopy) {
        kDebug(kdbg_code) << "error copying favicon database from " << originalFile.fileName() << " to " << copyTo;
        kDebug(kdbg_code) << originalFile.errorString();
    }
}

FetchSqlite::~FetchSqlite()
{
    if(m_db.isOpen()) m_db.close();
    QFile(m_databaseFile).remove();
}

void FetchSqlite::prepare()
{
    m_db.setDatabaseName(m_databaseFile);
    bool ok = m_db.open();
    kDebug(kdbg_code) << "Chrome favicon Database " << m_databaseFile << " was opened: " << ok;
    if(!ok) {
        kDebug(kdbg_code) << "Error: " << m_db.lastError().text();
    }
}

void FetchSqlite::teardown()
{
    m_db.close();
}

QList<QVariantMap> FetchSqlite::query(const QString &sql, QMap<QString, QVariant> bindObjects)
{
    QSqlQuery query(m_db);
    query.prepare(sql);
    foreach(const QString &variableName, bindObjects.keys()) {
        query.bindValue(variableName, bindObjects.value(variableName));
        kDebug(kdbg_code) << "* Bound " << variableName << " to " << query.boundValue(variableName);
    }

    if(!query.exec()) {
        QSqlError error = m_db.lastError();
        kDebug(kdbg_code) << "query failed: " << error.text() << " (" << error.type() << ", " << error.number() << ")";
        kDebug(kdbg_code) << query.lastQuery();
    }

    QList<QVariantMap> result;
    while(query.next()) {
        QVariantMap recordValues;
        QSqlRecord record = query.record();
        for(int field=0; field<record.count(); field++) {
            QVariant value = record.value(field);
            recordValues.insert(record.fieldName(field), value  );
        }
        result << recordValues;
    }
    return result;
}
