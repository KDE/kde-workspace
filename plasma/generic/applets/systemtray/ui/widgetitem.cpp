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
#include <QtQuick/QQuickItem>

#include <Plasma/Containment>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace SystemTray
{
// class WidgetItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WidgetItem::WidgetItem(QQuickItem *parent)
    : QQuickItem(parent),
      m_applet(0)
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
        QQuickItem *widget = m_task.data()->widget(m_applet, false);
        if (widget && widget->parentItem() == this) {
            widget->setVisible(false);
            widget->setParentItem(0);
        }
    }
}

void WidgetItem::bind()
{
    if (m_applet && m_task) {
        QQuickItem *widget = m_task.data()->widget(m_applet);
        if (widget) {
            widget->setParentItem(this);
            widget->setX(0);
            widget->setY(0);
            //widget->setPreferredSize(width(), width());
            widget->setImplicitWidth(width());
            widget->setImplicitHeight(width());
            //widget->setMaximumSize(width(), width());
            widget->setVisible(true);
        }
    }
}


void WidgetItem::afterWidthChanged()
{
    if (!m_applet || !m_task) {
        return;
    }

    QQuickItem *widget = m_task.data()->widget(m_applet);
    if (widget) {
        //widget->setPreferredSize(width(), width());
        widget->setImplicitHeight(width());
        widget->setImplicitWidth(width());
        widget->setVisible(true);
    }
}

void WidgetItem::afterHeightChanged()
{
    if (!m_applet || !m_task) {
        return;
    }

    QQuickItem *widget = m_task.data()->widget(m_applet);
    if (widget) {
        //widget->setPreferredSize(width(), width());
        widget->setImplicitWidth(width());
        widget->setImplicitHeight(width());
        widget->setVisible(true);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


} //namespace SystemTray

