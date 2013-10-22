/******************************************************************************
*   Copyright 2013 Sebastian Kügler <sebas@kde.org>                           *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "statusnotifiertest.h"

#include <QDebug>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kplugininfo.h>
#include <kplugintrader.h>

#include <qcommandlineparser.h>

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QMap>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QJsonObject>


static QTextStream cout(stdout);

class StatusNotifierTestPrivate {
public:
    QString pluginName;
    QTimer* timer;
    int interval = 500;
};

StatusNotifierTest::StatusNotifierTest() :
    QObject(0)
{
    d = new StatusNotifierTestPrivate;
    d->timer = new QTimer(this);
    connect(d->timer, &QTimer::timeout, this, &StatusNotifierTest::timeout);
    d->timer->setInterval(d->interval);
    d->timer->start();
}

StatusNotifierTest::~StatusNotifierTest()
{
    delete d;
}

int StatusNotifierTest::runMain()
{

    return 0;
}

void StatusNotifierTest::timeout()
{
    qDebug() << "Timeout";
    qApp->quit();
}



#include "moc_statusnotifiertest.cpp"

