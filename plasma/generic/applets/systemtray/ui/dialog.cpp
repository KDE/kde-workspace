/***********************************************************************************************************************
 * System Tray (KDE Plasmoid)
 * Copyright (C) 2011-2012 ROSA  <support@rosalab.ru>
 * License: GPLv3+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *          based on work made by Marco Martin <mart@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **********************************************************************************************************************/


// Includes
#include "dialog.h"

#include <netwm.h>

#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QtCore/QWeakPointer>
#include <QtCore/QTimer>
#include <QtGui/QResizeEvent>
#include <QtGui/QMoveEvent>
#include <QtGui/QGraphicsObject>
#include <QtGui/QGraphicsWidget>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsScene>
#include <QtDeclarative/QDeclarativeItem>

#include <KDE/Plasma/Dialog>
#include <KDE/Plasma/WindowEffects>
#include <KDE/KWindowSystem>



namespace SystemTray
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Dialog::_DeclarativeItemContainer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Dialog::_DeclarativeItemContainer: public QGraphicsWidget
{
public:
    _DeclarativeItemContainer(Dialog &dialog);

    void setDeclarativeItem(QDeclarativeItem *item);
    QDeclarativeItem *declarativeItem() const { return m_item.data(); }

private:
    virtual void resizeEvent(QGraphicsSceneResizeEvent *) {
        m_dialog.containerSizeChanged();
    }

    Dialog &m_dialog;
    QWeakPointer<QDeclarativeItem> m_item;
};


Dialog::_DeclarativeItemContainer::_DeclarativeItemContainer(Dialog &dialog):
    QGraphicsWidget(0),
    m_dialog(dialog)
{

}


void Dialog::_DeclarativeItemContainer::setDeclarativeItem(QDeclarativeItem *item)
{
    if (m_item) {
        disconnect(m_item.data(), 0, &m_dialog, 0);
    }
    m_item = item;
    static_cast<QGraphicsItem *>(item)->setParentItem(this);
    setMinimumWidth(item->implicitWidth());
    setMinimumHeight(item->implicitHeight());
    resize(item->width(), item->height());
    connect(item, SIGNAL(widthChanged()), &m_dialog, SLOT(itemSizeChanged()));
    connect(item, SIGNAL(heightChanged()), &m_dialog, SLOT(itemSizeChanged()));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// struct Dialog::_Private
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Dialog::_Private
{
    Plasma::Dialog *dialog;
    QWeakPointer<QDeclarativeItem> item;
    Dialog::_DeclarativeItemContainer *container;
    Dialog::Location location;

    _Private(): dialog(0), container(0), location(Dialog::Floating) {}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Dialog::Dialog(QObject *parent):
    QObject(parent),
    d(new _Private)
{
    d->dialog = new Plasma::Dialog(0, Qt::FramelessWindowHint);
    d->dialog->installEventFilter(this);
}


Dialog::~Dialog()
{
    delete d->container;
    d->dialog->deleteLater();
    delete d;
}



QDeclarativeItem *Dialog::item() const
{
    return d->item.data();
}


void Dialog::setItem(QDeclarativeItem *item)
{
    if (item == d->item.data()) {
        return;
    }

    // remove old item
    if (d->item) {
        d->item.data()->setParent(item ? item->parent() : 0);
    }

    // set new item
    d->item = item;

    // init new item
    if (item) {
        item->setParentItem(0);
        item->setParent(this);
    }

    //if this is called in Compenent.onCompleted we have to wait a loop the item is added to a scene
    QTimer::singleShot(0, this, SLOT(syncItem()));
    emit changedItem();
}


bool Dialog::visible() const
{
    return d->dialog->isVisible();
}


void Dialog::setVisible(bool vis)
{
    if (d->dialog->isVisible() != vis) {
        d->dialog->setVisible(vis);
        if (vis) {
            d->dialog->raise();
        }
    }
}


int Dialog::x() const
{
    return d->dialog->pos().x();
}


void Dialog::setX(int x)
{
    d->dialog->move(x, y());
}


int Dialog::y() const
{
    return d->dialog->pos().y();
}


void Dialog::setY(int y)
{
    d->dialog->move(x(), y);
}


int Dialog::width() const
{
    return d->dialog->size().width();
}


int Dialog::height() const
{
    return d->dialog->size().height();
}


Dialog::Location Dialog::location() const
{
    return d->location;
}

void Dialog::setLocation(Dialog::Location loc)
{
    if (loc != d->location) {
        d->location = loc;
        emit changedLocation();
    }
}


bool Dialog::active() const
{
    return d->dialog->isActiveWindow();
}


void Dialog::activate()
{
    d->dialog->activateWindow();
}


bool Dialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != d->dialog) {
        return false;
    }

    switch (event->type()) {
    case QEvent::Show:
    case QEvent::Hide:
        KWindowSystem::setState(d->dialog->winId(), NET::SkipTaskbar | NET::SkipPager);
        Plasma::WindowEffects::slideWindow(d->dialog, (Plasma::Location)d->location);
        emit changedVisible();
        return false;

    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        emit changedActive();
        return false;

    case QEvent::Move: {
        QMoveEvent *me = static_cast<QMoveEvent *>(event);
        if (me->oldPos().x() != me->pos().x()) {
            emit changedX();
        }
        if (me->oldPos().y() != me->pos().y()) {
            emit changedY();
        }
        return false;
    }

    case QEvent::Resize: {
        QResizeEvent *re = static_cast<QResizeEvent *>(event);
        if (re->oldSize().width() != re->size().width()) {
            emit changedWidth();
        }
        if (re->oldSize().height() != re->size().height()) {
            emit changedHeight();
        }
    }

    default:
        return false;
    }
    return false;
}


void Dialog::syncItem()
{
    if (!d->item) {
        return;
    }

    if ( qobject_cast<QDeclarativeItem*>(d->dialog->graphicsWidget()) == d->item.data() ) {
        return;
    }

    //not have a scene? go up in the hyerarchy until we find something with a scene
    QGraphicsScene *scene = d->item.data()->scene();
    for (QObject *p = d->item.data()->parent(); !scene && p; p = p->parent()) {
        QGraphicsObject *go = qobject_cast<QGraphicsObject *>(p);
        if (go) {
            scene = go->scene();
        }
    }

    if (!scene) {
        return;
    }

    if (!d->item.data()->scene()) {
        scene->addItem(d->item.data());
    }

    if (!d->container) {
        d->container = new _DeclarativeItemContainer(*this);
        scene->addItem(d->container);
    }
    d->container->setDeclarativeItem(d->item.data());
    d->dialog->setGraphicsWidget(d->container);
}

void Dialog::itemSizeChanged()
{
    if (d->container && d->item) {
        d->container->resize(d->item.data()->width(), d->item.data()->height());
    }
}

void Dialog::containerSizeChanged()
{
    if (d->container && d->item) {
        d->item.data()->setProperty("width", QVariant::fromValue(d->container->size().width()));
        d->item.data()->setProperty("height", QVariant::fromValue(d->container->size().height()));
    }
}

} // namespace SystemTray
