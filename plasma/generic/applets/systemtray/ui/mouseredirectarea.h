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


#ifndef __SYSTEMTRAY__MOUSEREDIRECTAREA_H
#define __SYSTEMTRAY__MOUSEREDIRECTAREA_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Includes
#include <QtDeclarative/QDeclarativeItem>

#include <KDE/Plasma/Applet>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
class QGraphicsWidget;
class QGraphicsSceneWheelEvent;
class QGraphicsSceneContextMenuEvent;


namespace SystemTray
{
class Task;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class MouseRedirectArea
 * This helper class is intended to handle and redirect some mouse events
 */
class MouseRedirectArea: public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QObject* target READ target WRITE setTarget)
    Q_PROPERTY(QObject* applet READ applet WRITE setApplet)
public:
    explicit MouseRedirectArea(QDeclarativeItem *parent = 0);

    QObject* target() const { return m_target; }
    void setTarget(QObject* t);
    QObject *applet() const { return m_applet; }
    void setApplet(QObject *applet);

signals:
    void clickMiddle();
    void clickRight();
    void scrollVert(int delta);
    void scrollHorz(int delta);
    void changedMousePos(qreal mouseX, qreal mouseY);
    void entered();
    void exited();

private: //Events
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

    template<class T> void forwardEvent(T *event, bool is_context_menu = false);

private: // Methods
    void processTarget();

private: //Variables
    QGraphicsObject *m_widget;
    Task *m_task;
    QObject *m_target;
    Plasma::Applet *m_applet;
    bool m_isApplet; // true if target is an applet
};

} // namespace SystemTray

#endif // __SYSTEMTRAY__MOUSEREDIRECTAREA_H
