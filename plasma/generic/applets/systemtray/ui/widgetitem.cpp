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

#include "../core/task.h"

#include <QtCore/QWeakPointer>
#include <QtGui/QGraphicsWidget>

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
    connect(this, SIGNAL(widthChanged()), this, SLOT(afterWidthChanged()), Qt::QueuedConnection);
    connect(this, SIGNAL(heightChanged()), this, SLOT(afterHeightChanged()), Qt::QueuedConnection);
}


WidgetItem::~WidgetItem()
{
    unbind();
}

void WidgetItem::setTask(QObject *task)
{
    Task *t = qobject_cast<Task*>(task);
    if (m_task.data() == t)
        return;
    unbind();
    m_task = t;
    bind();
    emit changedTask();
}

void WidgetItem::setApplet(QObject *a)
{
    Plasma::Applet *applet = qobject_cast<Plasma::Applet*>(a);
    if (m_applet == applet)
        return;
    unbind();
    m_applet = applet;
    bind();
}

void WidgetItem::unbind()
{
    if (m_applet && m_task) {
        QGraphicsWidget *widget = m_task.data()->widget(m_applet, false);
        if (widget && widget->parentItem() == this) {
            widget->hide();
            widget->setParentItem(0);
        }
    }
}

void WidgetItem::bind()
{
    if (m_applet && m_task) {
        QGraphicsWidget *widget = m_task.data()->widget(m_applet);
        if (widget) {
            widget->setParentItem(this);
            widget->setPos(0, 0);
            widget->setPreferredSize(width(), width());
            widget->setMinimumSize(width(), width());
            widget->setMaximumSize(width(), width());
            widget->show();
        }
    }
}


void WidgetItem::afterWidthChanged()
{
    if (!m_applet || !m_task) {
        return;
    }

    QGraphicsWidget *widget = m_task.data()->widget(m_applet);
    if (widget) {
        widget->setPreferredSize(width(), width());
        widget->setMinimumSize(width(), width());
        widget->setMaximumSize(width(), width());
        widget->show();
    }
}

void WidgetItem::afterHeightChanged()
{
    if (!m_applet || !m_task) {
        return;
    }

    QGraphicsWidget *widget = m_task.data()->widget(m_applet);
    if (widget) {
        widget->setPreferredSize(width(), width());
        widget->setMinimumSize(width(), width());
        widget->setMaximumSize(width(), width());
        widget->show();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


} //namespace SystemTray

