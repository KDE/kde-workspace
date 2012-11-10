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

#include <QtGui/QGraphicsWidget>
#include <QtCore/QTimer>

#include "../ui/applet.h"

namespace SystemTray
{


class Task::Private
{
public:
    Private()
        : status(Task::UnknownStatus),
          category(Task::UnknownCategory)
    {
    }

    QHash<Plasma::Applet *, QGraphicsWidget *> widgetsByHost;
    Task::Status status;
    Task::Category category;
    QString name;
};


Task::Task(QObject *parent)
    : QObject(parent),
      d(new Private)
{
}

Task::~Task()
{
    emit destroyed(this);
    foreach (QGraphicsWidget * widget, d->widgetsByHost) {
        disconnect(widget, 0, this, 0);
        // sometimes it appears that the widget will get scheduled for a repaint
        // then it gets deleted here and QGraphicsScene doesn't get that straight
        // in its bookkeeping and crashes occur; work around this by giving it a
        // chance to schedule after the next paintfun
        widget->deleteLater();
    }
    delete d;
}

QGraphicsWidget *Task::widget(Plasma::Applet *host, bool createIfNecessary)
{
    Q_ASSERT(host);

    QGraphicsWidget *widget = d->widgetsByHost.value(host);

    if (!widget && createIfNecessary) {
        widget = createWidget(host);

        if (widget) {
            d->widgetsByHost.insert(host, widget);
            connect(widget, SIGNAL(destroyed()), this, SLOT(widgetDeleted()));
        }
    }

    return widget;
}

bool Task::isEmbeddable(SystemTray::Applet *host)
{
    if (!host) {
        return false;
    }

    return (d->widgetsByHost.value(host) || isEmbeddable()) && host->shownCategories().contains(category());
}

QHash<Plasma::Applet *, QGraphicsWidget *> Task::widgetsByHost() const
{
    return d->widgetsByHost;
}

void Task::abandon(Plasma::Applet *host)
{
    QGraphicsWidget *widget = d->widgetsByHost.take(host);
    if (widget) {
        widget->deleteLater();
    }
}

QGraphicsWidget *Task::forget(Plasma::Applet *host)
{
    return d->widgetsByHost.take(host);
}

void Task::widgetDeleted()
{
    bool wasEmbeddable = isEmbeddable();

    QGraphicsWidget *w = static_cast<QGraphicsWidget*>(sender());
    QMutableHashIterator<Plasma::Applet *, QGraphicsWidget *> it(d->widgetsByHost);
    while (it.hasNext()) {
        it.next();
        if (it.value() == w) {
            it.remove();
        }
    }

    if (!wasEmbeddable && isEmbeddable()) {
        // we have to delay this call because some Task subclasses have a single widget that
        // becomes embedabble at this point (e.g. FdoTaskWidget). if the signal is emitted
        // immediately, another system tray will attempt to immediately embed it, and
        // part of that process involves removing the item from any previous layouts. now,
        // if that happens because a system tray is being deleted (removed, app exit, logout, etcS)
        // then the previous parent layout will be a dangling pointer at this point and
        // that will not get fixed up until everything is finished... so.. we delay the signal
        QTimer::singleShot(0, this, SLOT(emitChanged()));
    }
}

void Task::emitChanged()
{
    emit changed(this);
}

bool Task::isUsed() const
{
    return !d->widgetsByHost.isEmpty();
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


void Task::setName(QString name)
{
    if (d->name != name) {
        d->name = name;
        emit changedName();
    }
}


}

#include "task.moc"
