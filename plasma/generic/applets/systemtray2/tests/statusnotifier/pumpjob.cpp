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

#include "pumpjob.h"

#include <QDebug>

#include <QStringList>
#include <QTimer>

static QTextStream cout(stdout);

class PumpJobPrivate {
public:
    QString name;
    QString error;

    QTimer* timer;
    int interval = 50;

    int counter = 0;
};

PumpJob::PumpJob(QObject* parent) :
    KJob(parent)
{
    d = new PumpJobPrivate;

    d->timer = new QTimer(this);
    d->timer->setInterval(d->interval);

    connect(d->timer, &QTimer::timeout, this, &PumpJob::timeout);

    init();
}

void PumpJob::init()
{
    d->timer->start();

}

PumpJob::~PumpJob()
{
    qDebug() << "Bye bye";
    delete d;
}

void PumpJob::start()
{
    qDebug() << "Starting job / timer";
    d->timer->start();
}

bool PumpJob::doKill()
{
    d->timer->stop();
    return KJob::doKill();
}

bool PumpJob::doResume()
{
    d->timer->start();
    return KJob::doResume();
}

bool PumpJob::doSuspend()
{
    d->timer->stop();
    return KJob::doSuspend();
}



void PumpJob::timeout()
{
    d->counter++;
    setPercent(d->counter);
    if (d->counter % 10 == 0) {
        qDebug() << "percent: " << percent();
    }

    if (d->counter >= 100) {
        qDebug() << "Job done";
        emitResult();
    }

}

#include "moc_pumpjob.cpp"

