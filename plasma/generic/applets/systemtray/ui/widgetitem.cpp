/***********************************************************************************************************************
 * ROSA System Tray (KDE Plasmoid)
 * Copyright ⓒ 2011-2012 ROSA  <support@rosalab.ru>
 * License: GPLv3+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include "widgetitem.h"

#include <QtCore/QWeakPointer>
#include <QtGui/QGraphicsWidget>

#include <KDE/Plasma/Applet>
#include <KDE/Plasma/Containment>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace SystemTray
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// struct WidgetItem::_Private
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct WidgetItem::_Private
{
    QWeakPointer<QGraphicsWidget> widget;
    WidgetItem &owner;

    _Private(WidgetItem &owner): owner(owner) {}
    void unbind();
};


void WidgetItem::_Private::unbind()
{
    if (widget) {
        QGraphicsWidget *w = widget.data();
        if (w->parentItem() == &owner) {
            w->hide();
            w->setParentItem(0);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class WidgetItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WidgetItem::WidgetItem(QDeclarativeItem *parent): QDeclarativeItem(parent),
    d(new _Private(*this))
{
    setClip(false);
}


WidgetItem::~WidgetItem()
{
    d->unbind();
    delete d;
}


QVariant WidgetItem::widget() const
{
    return QVariant::fromValue(static_cast<QObject*>(d->widget.data()));
}


void WidgetItem::setWidget(QVariant w)
{
    QGraphicsWidget *widget = qobject_cast<QGraphicsWidget*>(w.value<QObject*>());
    // check input
    if ( widget == d->widget.data() ) {
        return;
    }

    // unbind old widget
    d->unbind();

    // bind new widget
    d->widget = widget;
    if (widget) {
        widget->setParentItem(this);
        widget->setPos(0, 0);
        widget->setPreferredSize(width(), width());
        widget->setMinimumSize(width(), width());
        widget->setMaximumSize(width(), width());
        widget->show();
    }
    emit changedWidget();
}


} //namespace SystemTray

