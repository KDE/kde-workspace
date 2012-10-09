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
    if (!isEnabled() || !m_target || !m_applet)
        return;

    QPointF delta = m_target->sceneBoundingRect().center() - event->scenePos();
    event->setScenePos(m_target->sceneBoundingRect().center());
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
        event->setPos(m_target->boundingRect().center());
        scene()->sendEvent(m_target, event);
    }
}


MouseRedirectArea::MouseRedirectArea(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_target(0)
    , m_applet(0)
    , m_isApplet(false)
{
    setAcceptsHoverEvents(true);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
}

void MouseRedirectArea::setTarget(QVariant t)
{
    QGraphicsItem *target = qobject_cast<QGraphicsItem*>(t.value<QObject*>());
    m_target = target;
    m_isApplet = (qobject_cast<Plasma::Applet*>(target) != 0);
}

void MouseRedirectArea::setApplet(QVariant t)
{
    Plasma::Applet *applet = qobject_cast<Plasma::Applet*>(t.value<QObject*>());
    m_applet = applet;
}

void MouseRedirectArea::setIsWidget(bool is_widget)
{
    m_isWidget = is_widget;
}


void MouseRedirectArea::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!m_isApplet && !m_isWidget) {
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
    if (!m_isApplet && !m_isWidget) {
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
}


void MouseRedirectArea::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    forwardEvent(event);
}

void MouseRedirectArea::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF pos = event->pos();
    emit changedMousePos(pos.x(), pos.y());
    forwardEvent(event);
}


} // namespace SystemTray
