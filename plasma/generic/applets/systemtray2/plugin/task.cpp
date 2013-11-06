/***************************************************************************
 *   task.cpp                                                              *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "task.h"

#include <QtQuick/QQuickItem>
#include <QtCore/QTimer>

namespace SystemTray
{


class Task::Private
{
public:
    Private()
        : status(Task::UnknownStatus),
          category(Task::UnknownCategory),
          expanded(false)
    {
    }

    Task::Status status;
    Task::Category category;
    QString name;
    bool shown;
    bool expanded;
};


Task::Task(QObject *parent)
    : QObject(parent),
      d(new Private)
{
}

Task::~Task()
{
    emit destroyed(this);
    delete d;
}


void Task::emitChanged()
{
    emit changed(this);
}

void Task::setExpanded(bool expanded)
{
    if (d->expanded != expanded) {
        d->expanded = expanded;
        emit expandedChanged();
    }
}

bool Task::expanded() const
{
    return d->expanded;
}


void Task::setCategory(Category category)
{
    if (d->category == category) {
        return;
    }

    d->category = category;
    emit changedCategory();
    emit changed(this);
}

Task::Category Task::category() const
{
    return d->category;
}

void Task::setStatus(Status status)
{
    if (d->status == status) {
        return;
    }

    d->status = status;
    qDebug() << "ST2 status changed " << status;
    emit changedStatus();
    emit changed(this);
}

Task::Status Task::status() const
{
    return d->status;
}

QString Task::name() const
{
    return d->name;
}

bool Task::shown() const
{
    return d->shown;
}

void Task::setShown(bool show)
{
    if (d->shown != show) {
        d->shown = show;
        emit shownChanged();
    }
}


QQuickItem* Task::taskItem() const
{
    return 0;
}

QQuickItem* Task::taskItemExpanded() const
{
    return 0;
}


void Task::setName(QString name)
{
    if (d->name != name) {
        d->name = name;
        emit changedName();
    }
}

}

#include "task.moc"
