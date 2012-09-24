/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_DESKTOPTOOLBOX_P_H
#define PLASMA_DESKTOPTOOLBOX_P_H

#include <QGraphicsItem>
#include <QPropertyAnimation>
#include <QTime>

#include <KIcon>

#include <Plasma/Animator>
#include <Plasma/IconWidget>

#include "internaltoolbox.h"

class Widget;
class EmptyGraphicsItem;
class DesktopToolBoxPrivate;

class DesktopToolBox : public InternalToolBox
{
    Q_OBJECT
    Q_PROPERTY(qreal highlight READ highlight WRITE setHighlight)

public:
    explicit DesktopToolBox(Plasma::Containment *parent = 0);
    explicit DesktopToolBox(QObject *parent = 0, const QVariantList &args = QVariantList());
    ~DesktopToolBox();
    QRectF boundingRect() const;
    QPainterPath shape() const;

    void addTool(QAction *action);
    void removeTool(QAction *action);
    void showToolBox();
    void hideToolBox();

    QSize cornerSize() const;
    QSize fullWidth() const;
    QSize fullHeight() const;

    QGraphicsWidget *toolParent();

public Q_SLOTS:
    void toolTipAboutToShow();
    void toolTipHidden();
    void updateToolBox();

protected:
    void init();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void keyPressEvent(QKeyEvent *event);

protected Q_SLOTS:
    void setHighlight(qreal progress);
    qreal highlight();
    void updateTheming();
    void toolTriggered(bool);
    void hideToolBacker();
    /**
     * show/hide the toolbox
     */
    void toggle();

    // basic desktop controls
    void startLogout();
    void logout();
    void lockScreen();

private:
    void highlight(bool highlighting);
    void adjustToolBackerGeometry();
    void adjustBackgroundBorders() const;

    Plasma::Containment *m_containment;
    Plasma::FrameSvg *m_background;
    QMultiMap<Plasma::AbstractToolBox::ToolType, Plasma::IconWidget *> m_tools;
    KIcon m_icon;
    EmptyGraphicsItem *m_toolBacker;
    QWeakPointer<QPropertyAnimation> m_anim;
    qreal m_animCircleFrame;
    qreal m_animHighlightFrame;
    QRect m_shapeRect;
    QColor m_fgColor;
    QColor m_bgColor;
    bool m_hovering;
};

K_EXPORT_PLASMA_TOOLBOX(desktoptoolbox, DesktopToolBox)

#endif // multiple inclusion guard

