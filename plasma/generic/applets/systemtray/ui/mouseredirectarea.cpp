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
#include "mouseredirectarea.h"

#include "../core/task.h"

#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneHoverEvent>
#include <QtGui/QGraphicsScene>

#include <KDE/Plasma/Containment>


namespace SystemTray
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class MouseRedirectArea
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> void MouseRedirectArea::forwardEvent(T *event, bool is_context_menu)
{
    if (!isEnabled() || !(m_task || m_widget) || !m_applet)
        return;
    QGraphicsObject *target = m_widget ? m_widget : (m_task ? m_task->widget(m_applet, false) : 0);
    if (!target)
        return;

    QPointF delta = target->sceneBoundingRect().center() - event->scenePos();
    event->setScenePos(target->sceneBoundingRect().center());
    event->setScreenPos((event->screenPos() + delta).toPoint());

    if (m_isApplet) {
        if (is_context_menu && m_applet->containment()) {
            // redirect context menu event to containment because it is responsible for items of context menu of an applet
            event->setPos(m_applet->containment()->mapFromScene(event->scenePos()));
            scene()->sendEvent(m_applet->containment(), event);
        } else {
            event->setPos(scene()->itemAt(event->scenePos())->mapFromScene(event->scenePos()));
            scene()->sendEvent(scene()->itemAt(event->scenePos()), event);
        }
    } else {
        event->setPos(target->boundingRect().center());
        scene()->sendEvent(target, event);
    }
}


MouseRedirectArea::MouseRedirectArea(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_widget(0)
    , m_task(0)
    , m_target(0)
    , m_applet(0)
    , m_isApplet(false)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
}

void MouseRedirectArea::setTarget(QObject *t)
{
    if (m_target != t) {
        m_target = t;
        processTarget();
    }
}

void MouseRedirectArea::setApplet(QObject *a)
{
    Plasma::Applet *applet = qobject_cast<Plasma::Applet*>(a);
    if (m_applet != applet) {
        m_applet = applet;
        processTarget(); // it may be that target already set so we should process it
    }
}


void MouseRedirectArea::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!m_isApplet && m_widget) {
        switch (event->orientation()) {
        case Qt::Vertical:
            emit scrollVert(event->delta());
            break;
        case Qt::Horizontal:
            emit scrollHorz(event->delta());
            break;
        default:
            break;
        }
        return;
    }
    forwardEvent(event);
}


void MouseRedirectArea::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    forwardEvent(event, true);
}


void MouseRedirectArea::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isApplet && m_widget) {
        switch (event->button()) {
        case Qt::MiddleButton: emit clickMiddle(); return;
        case Qt::RightButton: emit clickRight(); return;
        default: break;
        }
    }
    forwardEvent(event);
}


void MouseRedirectArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    forwardEvent(event);
}

void MouseRedirectArea::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    forwardEvent(event);
    emit entered();
}


void MouseRedirectArea::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    forwardEvent(event);
    emit exited();
}

void MouseRedirectArea::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF pos = event->pos();
    emit changedMousePos(pos.x(), pos.y());
    forwardEvent(event);
}

void MouseRedirectArea::processTarget()
{
    // we have target as QObject but it may be Task or Widget
    if (!m_applet || !m_target)
        return; // applet and target must be already set

    m_isApplet = false;
    m_widget = 0;
    m_task = 0;
    m_task = qobject_cast<Task*>(m_target);
    if (m_task) {
        QGraphicsWidget *widget = m_task->widget(m_applet);
        m_isApplet = (qobject_cast<Plasma::Applet*>(widget) != 0);
    } else {
        m_widget = qobject_cast<QGraphicsObject*>(m_target);
    }
}


} // namespace SystemTray
