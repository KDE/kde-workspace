/***********************************************************************************************************************
 * KDE System Tray (Plasmoid)
 *
 * Copyright (C) 2012 ROSA  <support@rosalab.ru>
 * License: GPLv2+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
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
// class WidgetItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WidgetItem::WidgetItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
    setClip(false);
}


WidgetItem::~WidgetItem()
{
    unbind();
}


QObject *WidgetItem::widget() const
{
    return m_widget.data();
}


void WidgetItem::setWidget(QObject *w)
{
    QGraphicsWidget *widget = qobject_cast<QGraphicsWidget*>(w);
    // check input
    if (!widget || widget == m_widget.data()) {
        return;
    }

    // unbind old widget
    unbind();

    // bind new widget
    m_widget = widget;
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

void WidgetItem::unbind()
{
    QGraphicsWidget *w = m_widget.data();
    if (w && w->parentItem() == this) {
        w->hide();
        w->setParentItem(0);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


} //namespace SystemTray

