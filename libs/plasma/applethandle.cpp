/*
 *   Copyright 2007 by Kevin Ottens <ervin@kde.org>
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

#include "applethandle_p.h"

#include <QApplication>
#include <QBitmap>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>
#include <QtGui/QApplication>

#include <KColorScheme>
#include <KGlobalSettings>
#include <KIcon>
#include <KWindowSystem>

#include <cmath>
#include <math.h>

#include "applet.h"
#include "applet_p.h"
#include "containment.h"
#include "corona.h"
#include "paintutils.h"
#include "theme.h"
#include "view.h"

namespace Plasma
{

qreal _k_angleForPoints(const QPointF &center, const QPointF &pt1, const QPointF &pt2);

AppletHandle::AppletHandle(Containment *parent, Applet *applet, const QPointF &hoverPos)
    : QObject(),
      QGraphicsItem(parent),
      m_pressedButton(NoButton),
      m_containment(parent),
      m_applet(applet),
      m_opacity(0.0),
      m_anim(FadeIn),
      m_animId(0),
      m_angle(0.0),
      m_tempAngle(0.0),
      m_scaleWidth(1.0),
      m_scaleHeight(1.0),
      m_buttonsOnRight(false),
      m_pendingFade(false),
      m_topview(0),
      m_entryPos(hoverPos)
{
    KColorScheme colorScheme(QPalette::Active, KColorScheme::View, Theme::defaultTheme()->colorScheme());
    m_gradientColor = colorScheme.background(KColorScheme::NormalBackground).color();

    QTransform originalMatrix = m_applet->transform();
    m_applet->resetTransform();

    QRectF rect(m_applet->contentsRect());
    QPointF center = rect.center();
    originalMatrix.translate(center.x(), center.y());

    qreal cosine = originalMatrix.m11();
    qreal sine = originalMatrix.m12();

    m_angle = _k_angleForPoints(QPointF(0, 0),
                                QPointF(1, 0),
                                QPointF(cosine, sine));

    m_applet->setParentItem(this);

    rect = QRectF(m_applet->pos(), m_applet->size());
    center = rect.center();
    QTransform matrix;
    matrix.translate(center.x(), center.y());
    matrix.rotateRadians(m_angle);
    matrix.translate(-center.x(), -center.y());
    setTransform(matrix);

    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    m_hoverTimer->setInterval(333);

    m_leaveTimer = new QTimer(this);
    m_leaveTimer->setSingleShot(true);
    m_leaveTimer->setInterval(500);

    connect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(fadeIn()));
    connect(m_leaveTimer, SIGNAL(timeout()), this, SLOT(leaveTimeout()));
    connect(m_applet, SIGNAL(destroyed(QObject*)), this, SLOT(appletDestroyed()));

    setAcceptsHoverEvents(true);
    m_hoverTimer->start();

    //We got to be able to see the applet while dragging to to another containment,
    //so we want a high zValue.
    //FIXME: apparently this doesn't work: sometimes an applet still get's drawn behind
    //the containment it's being dragged to, sometimes it doesn't.
    m_zValue = m_applet->zValue();
    m_applet->raise();
    m_applet->installSceneEventFilter(this);
    setZValue(m_applet->zValue());
}

AppletHandle::~AppletHandle()
{
    detachApplet();
    if (m_topview) {
        delete m_topview;
    }
}

Applet *AppletHandle::applet() const
{
    return m_applet;
}

void AppletHandle::detachApplet ()
{
    if (!m_applet) {
        return;
    }

    disconnect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(fadeIn()));
    disconnect(m_leaveTimer, SIGNAL(timeout()), this, SLOT(leaveTimeout()));
    m_applet->disconnect(this);

    m_applet->removeSceneEventFilter(this);

    QRectF rect = QRectF(m_applet->pos(), m_applet->size());
    QPointF center = m_applet->mapFromParent(rect.center());

    QPointF newPos = transform().inverted().map(m_applet->pos());
    m_applet->setPos(mapToParent(newPos));

    QTransform matrix;
    matrix.translate(center.x(), center.y());
    matrix.rotateRadians(m_angle);
    matrix.translate(-center.x(), -center.y());
    m_applet->setTransform(matrix);

    m_applet->setParentItem(m_containment);

    m_applet->setZValue(m_zValue);

    m_applet->update(); // re-render the background, now we've transformed the applet

    m_applet = 0;
}

QRectF Plasma::AppletHandle::boundingRect() const
{
    return m_totalRect;
}

QPainterPath AppletHandle::shape() const
{
    //when the containment changes the applet is resetted to 0
    if (m_applet) {
        QPainterPath path = PaintUtils::roundedRectangle(m_rect, 10);
        return path.united(m_applet->mapToParent(m_applet->shape()));
    } else {
        return QGraphicsItem::shape();
    }
}

QPainterPath handleRect(const QRectF &rect, int radius, bool onRight)
{
    QPainterPath path;
    if (onRight) {
        // make the left side straight
        path.moveTo(rect.left(), rect.top());                                            // Top left
        path.lineTo(rect.right() - radius, rect.top());                                 // Top side
        path.quadTo(rect.right(), rect.top(), rect.right(), rect.top() + radius);       // Top right corner
        path.lineTo(rect.right(), rect.bottom() - radius);                              // Right side
        path.quadTo(rect.right(), rect.bottom(), rect.right() - radius, rect.bottom()); // Bottom right corner
        path.lineTo(rect.left(), rect.bottom());                                        // Bottom side
    } else {
        // make the right side straight
        path.moveTo(QPointF(rect.left(), rect.top() + radius));
        path.quadTo(rect.left(), rect.top(), rect.left() + radius, rect.top());         // Top left corner
        path.lineTo(rect.right(), rect.top());                                          // Top side
        path.lineTo(rect.right(), rect.bottom());                                       // Right side
        path.lineTo(rect.left() + radius, rect.bottom());                               // Bottom side
        path.quadTo(rect.left(), rect.bottom(), rect.left(), rect.bottom() - radius);   // Bottom left corner
    }

    path.closeSubpath();
    return path;
}

void AppletHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->save();
    painter->setOpacity(m_opacity);

    painter->save();
    painter->setOpacity(m_opacity * 0.8);
    painter->setPen(Qt::NoPen);
    painter->setRenderHints(QPainter::Antialiasing);

    QPainterPath path = handleRect(m_rect, 10, m_buttonsOnRight);
    painter->strokePath(path, m_gradientColor);
    painter->fillPath(path, m_gradientColor.lighter());
    painter->restore();

    //XXX this code is duplicated in the next function
    QPointF basePoint = m_rect.topLeft() + QPointF((HANDLE_WIDTH - ICON_SIZE) / 2, ICON_MARGIN);
    QPointF step = QPointF(0, ICON_SIZE + ICON_MARGIN);
    QPointF separator = step + QPointF(0, ICON_MARGIN);
    //end duplicate code

    QPointF shiftC;
    QPointF shiftD;
    QPointF shiftR;
    QPointF shiftM;

    switch(m_pressedButton)
    {
    case ConfigureButton:
        shiftC = QPointF(2, 2);
        break;
    case RemoveButton:
        shiftD = QPointF(2, 2);
        break;
    case RotateButton:
        shiftR = QPointF(2, 2);
        break;
    case ResizeButton:
        shiftM = QPointF(2, 2);
        break;
    default:
        break;
    }

    painter->drawPixmap(basePoint + shiftM, KIcon("transform-scale").pixmap(ICON_SIZE, ICON_SIZE)); //FIXME no transform-resize icon

    basePoint += step;
    painter->drawPixmap(basePoint + shiftR, KIcon("transform-rotate").pixmap(ICON_SIZE, ICON_SIZE));

    if (m_applet && m_applet->hasConfigurationInterface()) {
        basePoint += step;
        painter->drawPixmap(basePoint + shiftC, KIcon("configure").pixmap(ICON_SIZE, ICON_SIZE));
    }

    basePoint = m_rect.bottomLeft() + QPointF((HANDLE_WIDTH - ICON_SIZE) / 2, 0) - step;
    painter->drawPixmap(basePoint + shiftD, KIcon("edit-delete").pixmap(ICON_SIZE, ICON_SIZE));

    painter->restore();
}

AppletHandle::ButtonType AppletHandle::mapToButton(const QPointF &point) const
{
    //XXX this code is duplicated in the prev. function
    QPointF basePoint = m_rect.topLeft() + QPointF((HANDLE_WIDTH - ICON_SIZE) / 2, ICON_MARGIN);
    QPointF step = QPointF(0, ICON_SIZE + ICON_MARGIN);
    QPointF separator = step + QPointF(0, ICON_MARGIN);
   //end duplicate code

    QRectF activeArea = QRectF(basePoint, QSizeF(ICON_SIZE, ICON_SIZE));

    if (activeArea.contains(point)) {
        return ResizeButton;
    }

    activeArea.translate(step);
    if (activeArea.contains(point)) {
        return RotateButton;
    }

    if (m_applet && m_applet->hasConfigurationInterface()) {
        activeArea.translate(step);
        if (activeArea.contains(point)) {
            return ConfigureButton;
        }
    }

    activeArea.moveTop(m_rect.bottom() - activeArea.height() - ICON_MARGIN);
    if (activeArea.contains(point)) {
        return RemoveButton;
    }

    return MoveButton;
    //return m_applet->mapToParent(m_applet->shape()).contains(point) ? NoButton : MoveButton;
}


void AppletHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //containment recently switched?
    if (!m_applet) {
        QGraphicsItem::mousePressEvent(event);
        return;
    }

    if (m_pendingFade) {
        //m_pendingFade = false;
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_pressedButton = mapToButton(event->pos());
        //kDebug() << "button pressed:" << m_pressedButton;
        if (m_pressedButton != NoButton) {
            m_applet->raise();
            m_zValue = m_applet->zValue();
            setZValue(m_zValue);
        }

        if (m_pressedButton == MoveButton) {
            m_pos = pos();
        }
        event->accept();

        update();

        //set mousePos to the position in the applet, in screencoords, so it becomes easy
        //to reposition the toplevel view to the correct position.
        QPoint localpos = m_containment->view()->mapFromScene(m_applet->scenePos());
        m_mousePos = event->screenPos() - m_containment->view()->mapToGlobal(localpos);

        return;
    }

    QGraphicsItem::mousePressEvent(event);
}

bool AppletHandle::leaveCurrentView(const QPoint &pos) const
{
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        if (widget->geometry().contains(pos)) {
            //is this widget a plasma view, a different view then our current one,
            //AND not a dashboardview?
            Plasma::View *v = qobject_cast<Plasma::View *>(widget);
            if (v && v != m_applet->containment()->view()
                  && v != m_topview
                  && v->containment() != m_containment) {
                return true;
            }
        }
    }
    return false;
}


void AppletHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //kDebug() << "button pressed:" << m_pressedButton << ", fade pending?" << m_pendingFade;

    if (m_pendingFade) {
        startFading(FadeOut, m_entryPos);
        m_pendingFade = false;
    }

    ButtonType releasedAtButton = mapToButton(event->pos());

    if (m_applet && event->button() == Qt::LeftButton) {
        switch (m_pressedButton) {
            case ResizeButton:
            case RotateButton: {
                if (m_scaleWidth > 0 && m_scaleHeight > 0) {
                    QRectF rect(m_applet->boundingRect());
                    const qreal newWidth = rect.width() * m_scaleWidth;
                    const qreal newHeight = rect.height() * m_scaleHeight;
                    m_applet->resetTransform();
                    m_applet->resize(newWidth, newHeight);
                    scale(1.0/m_scaleWidth, 1.0/m_scaleHeight);
                    moveBy((rect.width() - newWidth) / 2, (rect.height() - newHeight) / 2);
                    m_scaleWidth = m_scaleHeight = 0;
                }
                QRectF rect = QRectF(m_applet->pos(), m_applet->size());
                QPointF center = rect.center();

                m_angle += m_tempAngle;
                m_tempAngle = 0;

                QTransform matrix;
                matrix.translate(center.x(), center.y());
                matrix.rotateRadians(m_angle);
                matrix.translate(-center.x(), -center.y());

                setTransform(matrix);
                m_applet->update();
                break;
            }
            case ConfigureButton:
                //FIXME: Remove this call once the configuration management change was done
                if (m_pressedButton == releasedAtButton) {
                    m_applet->showConfigurationInterface();
                }
                break;
            case RemoveButton:
                if (m_pressedButton == releasedAtButton) {
                    forceDisappear();
                    m_applet->destroy();
                }
                break;
            case MoveButton: {
                if (m_topview) {
                    m_topview->hide();
                    delete m_topview;
                    m_topview = 0;
                    m_applet->d->ghostView = 0;
                    m_applet->update();
                }

                //find out if we were dropped on a panel or something
                if (leaveCurrentView(event->screenPos())) {
                    startFading(FadeOut, m_entryPos);
                    Plasma::View *v = Plasma::View::topLevelViewAt(event->screenPos());
                    if (v && v != m_containment->view()) {
                        Containment *c = v->containment();
                        QPoint pos = v->mapFromGlobal(event->screenPos());
                        //we actually have been dropped on another containment, so
                        //move there: we have a screenpos, we need a scenepos
                        //FIXME how reliable is this transform?
                        switchContainment(c, v->mapToScene(pos));
                    }
                } else {
                    // test for containment change
                    //kDebug() << "testing for containment change, sceneBoundingRect = " << m_containment->sceneBoundingRect();
                    if (!m_containment->sceneBoundingRect().contains(m_applet->scenePos())) {
                        // see which containment it belongs to
                        Corona * corona = qobject_cast<Corona*>(scene());
                        if (corona) {
                            QList<Containment*> containments = corona->containments();
                            for (int i = 0; i < containments.size(); ++i) {
                                QPointF pos;
                                QGraphicsView *v;
                                v = containments[i]->view();
                                if (v) {
                                    pos = v->mapToScene(v->mapFromGlobal(event->screenPos() - m_mousePos));

                                    if (containments[i]->sceneBoundingRect().contains(pos)) {
                                        //kDebug() << "new containment = " << containments[i];
                                        //kDebug() << "rect = " << containments[i]->sceneBoundingRect();
                                        // add the applet to the new containment and take it from the old one
                                        //kDebug() << "moving to other containment with position" << pos;;
                                        switchContainment(containments[i], pos);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    m_pressedButton = NoButton;
    update();
}

qreal _k_distanceForPoint(QPointF point)
{
    return std::sqrt(point.x()*point.x()+point.y()*point.y());
}

qreal _k_angleForPoints(const QPointF &center, const QPointF &pt1, const QPointF &pt2)
{
    QPointF vec1 = pt1 - center;
    QPointF vec2 = pt2 - center;

    qreal alpha = std::atan2(vec1.y(), vec1.x());
    qreal beta = std::atan2(vec2.y(), vec2.x());

    return beta - alpha;
}

void AppletHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    static const qreal snapAngle = M_PI_2 /* $i 3.14159 / 2.0 */;

    if (!m_applet) {
        QGraphicsItem::mouseMoveEvent(event);
        return;
    }

    //Track how much the mouse has moved.
    QPointF deltaScene  = event->scenePos() - event->lastScenePos();

    if (m_pressedButton == MoveButton) {
        m_pos += deltaScene;

        //Are we moving out of the current view?
        bool toTopLevel = leaveCurrentView(event->screenPos());

        if (!toTopLevel) {
            setPos(m_pos);
            if (m_topview) {
                //We were on a toplevel view, but are moving back on the scene
                //again. destroy the toplevel view:
                m_topview->hide();
                delete m_topview;
                m_topview = 0;
                m_applet->d->ghostView = 0;
            }
        } else {
            //set the screenRect correctly. the screenRect contains the bounding
            //rect of the applet in screen coordinates. m_mousePos contains the
            //position of the mouse relative to the applet, in screen coords.
            QRect screenRect = QRect(event->screenPos() - m_mousePos,
                                     m_applet->screenRect().size());

            //kDebug() << "screenRect = " << screenRect;

            if (!m_topview) { //create a new toplevel view
                m_topview = new View(m_containment, -1, 0);

                m_topview->setTrackContainmentChanges(false);
                m_topview->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint
                                                      | Qt::WindowStaysOnTopHint);
                m_topview->setWallpaperEnabled(false);
                m_topview->resize(screenRect.size());
                m_topview->setSceneRect(m_applet->sceneBoundingRect());
                m_topview->centerOn(m_applet);

                //We might have to scale the view, because we might be zoomed out.
                qreal scale = screenRect.width() / m_applet->boundingRect().width();
                m_topview->scale(scale, scale);

                //Paint a mask based on the applets shape.
                //TODO: I think it's nicer to have this functionality in Applet.
                //TODO: When the corona tiled background is disabled, disable the
                //mask when compositing is enabled.
                //FIXME: the mask doesn't function correctly when zoomed out.
                QBitmap bitmap(screenRect.size());
                QPainter * shapePainter = new QPainter();
                shapePainter->begin(&bitmap);
                shapePainter->fillRect(0, 0, screenRect.width(),
                                             screenRect.height(),
                                             Qt::white);
                shapePainter->setBrush(Qt::black);
                shapePainter->drawPath(m_applet->shape());
                shapePainter->end();
                delete shapePainter;
                m_topview->setMask(bitmap);

                m_topview->show();

                m_applet->d->ghostView = m_containment->view();

                //TODO: non compositing users are screwed: masking looks terrible.
                //Consider always enabling the applet background. Stuff like the analog clock
                //looks absolutely terrible when masked, while the minor rounded corners of most
                //themes should look quite ok. I said should, since shape() doesn't really
                //function correctly right now for applets drawing standard backgrounds.
            }

            m_topview->setGeometry(screenRect);
        }

    } else if (m_pressedButton == RotateButton ||
               m_pressedButton == ResizeButton) {
        if (_k_distanceForPoint(deltaScene) <= 1.0) {
            return;
        }

        QPointF pressPos = mapFromScene(event->buttonDownScenePos(Qt::LeftButton));

        QRectF rect = QRectF(m_applet->pos(), m_applet->size());
        QPointF center = rect.center();

        if (m_pressedButton == RotateButton) {
            m_tempAngle = _k_angleForPoints(center, pressPos, event->pos());

            if (fabs(remainder(m_angle+m_tempAngle, snapAngle)) < 0.15) {
                m_tempAngle = m_tempAngle - remainder(m_angle+m_tempAngle, snapAngle);
            }

            m_scaleWidth = m_scaleHeight = 1.0;
        } else {
            qreal w = m_applet->size().width();
            qreal h = m_applet->size().height();
            QSizeF min = m_applet->minimumSize();
            QSizeF max = m_applet->maximumSize();

            // If the applet doesn't have a minimum size, calculate based on a
            // minimum content area size of 16x16
            if (min.isEmpty()) {
                min = m_applet->boundingRect().size() - m_applet->boundingRect().size();
                min += QSizeF(16, 16);
            }

            bool ignoreAspectRatio = m_applet->aspectRatioMode() == Plasma::IgnoreAspectRatio;

            if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
                ignoreAspectRatio = !ignoreAspectRatio;
            }

            if (ignoreAspectRatio) {
                // free resizing
                qreal newScaleWidth = 0;
                qreal newScaleHeight = 0;

                QPointF startDistance(pressPos - center);
                QPointF currentDistance(event->pos() - center);
                newScaleWidth = currentDistance.x() / startDistance.x();
                newScaleHeight = currentDistance.y() / startDistance.y();

                if (qAbs(w - (newScaleWidth * w)) <= KGlobalSettings::dndEventDelay()) {
                    newScaleWidth = 1.0;
                }
                if (qAbs(h - (newScaleHeight * h)) <= KGlobalSettings::dndEventDelay()) {
                    newScaleHeight = 1.0;
                }

                if (newScaleHeight * h < min.height()) {
                    m_scaleHeight = min.height() / h;
                } else if (newScaleHeight * h > max.height()) {
                    m_scaleHeight = max.height() / h;
                } else {
                    m_scaleHeight = newScaleHeight;
                }
                if (newScaleWidth * w < min.width()) {
                    m_scaleWidth = min.width() / w;
                } else if (newScaleWidth * w > max.width()) {
                    m_scaleWidth = max.width() / w;
                } else {
                    m_scaleWidth = newScaleWidth;
                }
            } else {
                // maintain aspect ratio
                qreal newScale = 0;

                newScale = _k_distanceForPoint(event->pos()-center) / _k_distanceForPoint(pressPos-center);
                if (qAbs(h - (newScale * h)) <= KGlobalSettings::dndEventDelay()) {
                    newScale = 1.0;
                }

                if (newScale * w < min.width() || newScale * h < min.height()) {
                    m_scaleWidth = m_scaleHeight = qMax(min.width() / w, min.height() / h);
                } else if (newScale * w > max.width() && newScale * h > max.height()) {
                    m_scaleWidth = m_scaleHeight = qMin(max.width() / w, max.height() / h);
                } else {
                    m_scaleHeight = m_scaleWidth = newScale;
                }
            }
        }

        QTransform matrix;
        matrix.translate(center.x(), center.y());
        matrix.rotateRadians(m_angle+m_tempAngle);
        matrix.scale(m_scaleWidth, m_scaleHeight);
        matrix.translate(-center.x(), -center.y());
        setTransform(matrix);
    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}


//pos relative to scene
void AppletHandle::switchContainment(Containment *containment, const QPointF &pos)
{
    if (containment->containmentType() != Containment::PanelContainment) {
        //FIXME assuming everything else behaves like desktop?
        kDebug() << "desktop";
        m_containment = containment;
    }

    Applet *applet = m_applet;
    m_applet = 0; //make sure we don't try to act on the applet again
    applet->removeSceneEventFilter(this);
    forceDisappear(); //takes care of event filter and killing handle
    applet->disconnect(this); //make sure the applet doesn't tell us to do anything
    applet->setZValue(m_zValue);
    containment->addApplet(applet, containment->mapFromScene(pos));
    update();
}

QVariant AppletHandle::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && m_applet) {
        m_applet->updateConstraints(Plasma::LocationConstraint);
    }
    return QGraphicsItem::itemChange(change, value);
}

void AppletHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    //kDebug() << "hover enter";

    //if a disappear was scheduled stop the timer
    m_leaveTimer->stop();

    // if we're already fading out, fade back in
    if (m_animId != 0 && m_anim == FadeOut) {
        startFading(FadeIn, m_entryPos);
    } else {
        //schedule appear
        m_hoverTimer->start();
    }
}

void AppletHandle::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_leaveTimer->stop();
}

void AppletHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_hoverTimer->stop();

    if (m_pressedButton != NoButton) {
        m_pendingFade = true;
    } else {
        //wait a moment to hide the handle in order to recheck the mouse position
        m_leaveTimer->start();
    }
}

bool AppletHandle::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched == m_applet && event->type() == QEvent::GraphicsSceneHoverLeave) {
        hoverLeaveEvent(static_cast<QGraphicsSceneHoverEvent*>(event));
    }

    return false;
}

void AppletHandle::fadeAnimation(qreal progress)
{
    qreal endOpacity = (m_anim == FadeIn) ? 1.0 : 0.0;
    m_opacity += (endOpacity - m_opacity) * progress;
    //kDebug() << "progress" << progress << "m_opacity" << m_opacity << endOpacity;
    if (progress >= 1.0) {
        m_animId = 0;
    }
    if (progress >= 1.0 && m_anim == FadeOut) {
        emit disappearDone(this);
    }

    update();
}

void AppletHandle::fadeIn()
{
    startFading(FadeIn, m_entryPos);
}

void AppletHandle::leaveTimeout()
{
    startFading(FadeOut, m_entryPos);
}

void AppletHandle::appletDestroyed()
{
    m_applet = 0;
}

void AppletHandle::appletResized()
{
    prepareGeometryChange();
    calculateSize();
    update();
}

void AppletHandle::startFading(FadeType anim, const QPointF &hoverPos)
{
    if (m_animId != 0) {
        Animator::self()->stopCustomAnimation(m_animId);
    }

    m_hoverTimer->stop();
    m_leaveTimer->stop();

    m_entryPos = hoverPos;
    qreal time = 250;

    if (!m_applet || (anim == FadeOut && m_hoverTimer->isActive())) {
        // fading out before we've started fading in
        fadeAnimation(1.0);
        return;
    }

    if (anim == FadeIn) {
        //kDebug() << m_entryPos.x() << m_applet->pos().x();
        prepareGeometryChange();
        bool wasOnRight = m_buttonsOnRight;
        m_buttonsOnRight = m_entryPos.x() > (m_applet->size().width() / 2);
        calculateSize();
        QPolygonF region = mapToParent(m_rect).intersected(parentWidget()->boundingRect());
        //kDebug() << region << m_rect << mapToParent(m_rect) << parentWidget()->boundingRect();
        if (region != mapToParent(m_rect)) {
            // switch sides
            //kDebug() << "switch sides";
            m_buttonsOnRight = !m_buttonsOnRight;
            calculateSize();
            QPolygonF region2 = mapToParent(m_rect).intersected(parentWidget()->boundingRect());
            if (region2 != mapToParent(m_rect)) {
                // ok, both sides failed to be perfect... which one is more perfect?
                QRectF f1 = region.boundingRect();
                QRectF f2 = region2.boundingRect();
                //kDebug() << "still not a perfect world" << f2.width() << f2.height() << f1.width() << f1.height();
                if ((f2.width() * f2.height()) < (f1.width() * f1.height())) {
                    //kDebug() << "we did better the first time";
                    m_buttonsOnRight = !m_buttonsOnRight;
                    calculateSize();
                }
            }
        }

        if (wasOnRight != m_buttonsOnRight && m_anim == FadeIn && anim == FadeIn && m_opacity <= 1) {
            m_opacity = 0.0;
        }

        time *= 1.0 - m_opacity;
    } else {
        time *= m_opacity;
    }

    m_anim = anim;
    m_animId = Animator::self()->customAnimation(40, (int)time, Animator::EaseInOutCurve, this, "fadeAnimation");
}


void AppletHandle::forceDisappear()
{
    setAcceptsHoverEvents(false);
    startFading(FadeOut, m_entryPos);
}

void AppletHandle::calculateSize()
{
    int requiredHeight =  ICON_MARGIN + //first margin
                          (ICON_SIZE + ICON_MARGIN) * 4 + //XXX remember to update this if the number of buttons changes
                          ICON_MARGIN;  //blank space before the close button

    if (m_applet->hasConfigurationInterface()) {
        requiredHeight += (ICON_SIZE + ICON_MARGIN);
    }

    int top = m_applet->contentsRect().top();

    if (requiredHeight > m_applet->contentsRect().height()) {
        top += (m_applet->contentsRect().height() - requiredHeight) / 2.0;
    } else {
        requiredHeight = m_applet->contentsRect().height();
    }

    if (m_buttonsOnRight) {
        //put the rect on the right of the applet
        m_rect = QRectF(m_applet->size().width(), top, HANDLE_WIDTH, requiredHeight);
    } else {
        //put the rect on the left of the applet
        m_rect = QRectF(-HANDLE_WIDTH, top, HANDLE_WIDTH, requiredHeight);
    }

    m_rect = m_applet->mapToParent(m_rect).boundingRect();
    m_totalRect = m_rect.united(m_applet->geometry());
}

} // Plasma Namespace

#include "applethandle_p.moc"
