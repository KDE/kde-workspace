/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "desktopview.h"

#include <QAction>
#include <QDesktopWidget>
#include <QFile>
#include <QWheelEvent>
#include <QCoreApplication>

#include <KAuthorized>
#include <KMenu>
#include <KRun>
#include <KToggleAction>
#include <KWindowSystem>

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/svg.h"

#include "dashboardview.h"
#include "plasmaapp.h"

#ifdef Q_WS_WIN
#include "windows.h"
#include "windef.h"
#include "wingdi.h"
#include "winuser.h"
#endif

DesktopView::DesktopView(Plasma::Containment *containment, int id, QWidget *parent)
    : Plasma::View(containment, id, parent),
      m_zoomLevel(Plasma::DesktopZoom),
      m_dashboard(0),
      m_dashboardFollowsDesktop(true)
{
    setFocusPolicy(Qt::NoFocus);

    if (containment) {
        connectContainment(containment);
        containment->enableAction("zoom in", false);
        containment->enableAction("add sibling containment", false);
    }
    //FIXME should we have next/prev or up/down/left/right or what?
    QAction *action = new QAction(i18n("Next Activity"), this);
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    action->setShortcut(QKeySequence("ctrl+shift+n"));
    connect(action, SIGNAL(triggered()), this, SLOT(nextContainment()));
    addAction(action);
    action = new QAction(i18n("Previous Activity"), this);
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    action->setShortcut(QKeySequence("ctrl+shift+p"));
    connect(action, SIGNAL(triggered()), this, SLOT(previousContainment()));
    addAction(action);

    const int w = 25;
    QPixmap tile(w * 2, w * 2);
    tile.fill(palette().base().color());
    QPainter pt(&tile);
    QColor color = palette().mid().color();
    color.setAlphaF(.6);
    pt.fillRect(0, 0, w, w, color);
    pt.fillRect(w, w, w, w, color);
    pt.end();
    QBrush b(tile);
    setBackgroundBrush(tile);
}

DesktopView::~DesktopView()
{
    delete m_dashboard;
}

void DesktopView::connectContainment(Plasma::Containment *containment)
{
    if (containment) {
        connect(containment, SIGNAL(zoomRequested(Plasma::Containment*,Plasma::ZoomDirection)),
                this, SLOT(zoom(Plasma::Containment*,Plasma::ZoomDirection)));
        connect(containment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));
        connect(containment, SIGNAL(addSiblingContainment(Plasma::Containment *)), this, SLOT(addContainment(Plasma::Containment *)));
        connect(containment, SIGNAL(focusRequested(Plasma::Containment *)), this, SLOT(setContainment(Plasma::Containment *)));
    }
}

void DesktopView::toggleDashboard()
{
    if (!m_dashboard) {
        if (!containment()) {
            return;
        }

        m_dashboardFollowsDesktop = true;
        KConfigGroup cg = config();
        Plasma::Containment *dc = containment();
        int containmentId = cg.readEntry("DashboardContainment", 0);
        if (containmentId > 0) {
            foreach (Plasma::Containment *c, containment()->corona()->containments()) {
                if (c->id() == containmentId) {
                    dc = c;
                    m_dashboardFollowsDesktop = false;
                    break;
                }
            }
        }
        m_dashboard = new DashboardView(dc, 0);
        m_dashboard->addActions(actions());
    }

    m_dashboard->toggleVisibility();
    kDebug() << "toggling dashboard for screen" << screen() << m_dashboard->isVisible();
}

void DesktopView::adjustSize()
{
    // adapt to screen resolution changes
    QDesktopWidget *desktop = QApplication::desktop();
    QRect geom = desktop->screenGeometry(screen());
    setGeometry(geom);
    containment()->resize(geom.size());

    if (m_dashboard) {
        m_dashboard->setGeometry(geom);
    }
}

void DesktopView::setIsDesktop(bool isDesktop)
{
    if (isDesktop) {
#ifdef Q_WS_WIN
        setWindowFlags(Qt::FramelessWindowHint);
        SetWindowPos(winId(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        HWND hwndDesktop = ::FindWindowW(L"Progman", NULL);
        SetParent(winId(),hwndDesktop);
#else
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#endif

        KWindowSystem::setOnAllDesktops(winId(), true);
        KWindowSystem::setType(winId(), NET::Desktop);
        lower();

        adjustSize();
    } else {
        setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint);

        KWindowSystem::setOnAllDesktops(winId(), false);
        KWindowSystem::setType(winId(), NET::Normal); 
    }
}

bool DesktopView::isDesktop() const
{
#ifndef Q_WS_WIN
    return KWindowInfo(winId(), NET::WMWindowType).windowType(NET::Desktop);
#else
    return true;
#endif
}

void DesktopView::setContainment(Plasma::Containment *containment)
{
    Plasma::Containment *oldContainment = this->containment();
    if (containment == oldContainment) {
        return;
    }

    if (m_zoomLevel == Plasma::DesktopZoom) {
        //make sure actions are up-to-date
        //this is icky but necessary to have the toolbox show the right actions for the zoom level
        containment->enableAction("zoom in", false);
        containment->enableAction("add sibling containment", false);
    }

    if (m_dashboard && m_dashboardFollowsDesktop) {
        m_dashboard->setContainment(containment);
    }

    if (m_zoomLevel == Plasma::DesktopZoom) {
        //switch connections
        disconnect(oldContainment, 0, this, 0);
        connectContainment(containment);
        //make sure actions are up-to-date
        oldContainment->enableAction("zoom in", true);
        oldContainment->enableAction("add sibling containment", true);
    }

    View::setContainment(containment);
}

void DesktopView::addContainment(Plasma::Containment *fromContainment)
{
    if (fromContainment) {
        Plasma::Corona *corona = fromContainment->corona();
        if (corona) {
            //make it the same type of containment
            Plasma::Containment *c = corona->addContainment(fromContainment->pluginName());
            if (m_zoomLevel != Plasma::DesktopZoom) {
                connectContainment(c);
            }
            //note: this will set a sane size too, assuming we have a screen
            setContainment(c);
            kDebug() << "containment added at" << c->geometry();
        }
    }
}

void DesktopView::zoom(Plasma::Containment *containment, Plasma::ZoomDirection direction)
{
    if (direction == Plasma::ZoomIn) {
        zoomIn(containment);
    } else if (direction == Plasma::ZoomOut) {
        zoomOut(containment);
    }
}

void DesktopView::zoomIn(Plasma::Containment *toContainment)
{
    if (toContainment && containment() != toContainment) {
        setContainment(toContainment);
    }

    if (m_zoomLevel == Plasma::GroupZoom) {
        setDragMode(NoDrag);
        m_zoomLevel = Plasma::DesktopZoom;
        qreal factor = Plasma::scalingFactor(m_zoomLevel) / matrix().m11();
        scale(factor, factor);
        if (containment()) {
            //disconnect from other containments
            Plasma::Corona *corona = containment()->corona();
            if (corona) {
                QList<Plasma::Containment*> containments = corona->containments();
                foreach (Plasma::Containment *c, containments) {
                    if (c == containment() || c->containmentType() == Plasma::Containment::PanelContainment) {
                        continue;
                    }
                    disconnect(c, 0, this, 0);
                }
            }
            setSceneRect(containment()->geometry());
            containment()->closeToolBox();
            containment()->enableAction("zoom in", false);
            containment()->enableAction("add sibling containment", false);
        }
    } else if (m_zoomLevel == Plasma::OverviewZoom) {
        m_zoomLevel = Plasma::GroupZoom;
        qreal factor = Plasma::scalingFactor(m_zoomLevel);
        factor = factor / matrix().m11();
        scale(factor, factor);
        setSceneRect(QRectF(0, 0, scene()->sceneRect().right(), scene()->sceneRect().bottom()));

        if (containment()) {
            //make sure everyone can zoom out again
            Plasma::Corona *corona = containment()->corona();
            if (corona) {
                QList<Plasma::Containment*> containments = corona->containments();
                foreach (Plasma::Containment *c, containments) {
                    if (c->containmentType() == Plasma::Containment::PanelContainment) {
                        continue;
                    }
                    c->enableAction("zoom out", true);
                }
            }

            ensureVisible(containment()->sceneBoundingRect());
        }
    } else {
        setDragMode(NoDrag);
        if (containment()) {
            containment()->closeToolBox();
            containment()->enableAction("zoom in", false);
            containment()->enableAction("add sibling containment", false);
        }
    }
}

void DesktopView::zoomOut(Plasma::Containment *fromContainment)
{
    if (m_zoomLevel == Plasma::DesktopZoom) {
        fromContainment->enableAction("zoom in", true);
        fromContainment->enableAction("add sibling containment", true);
        m_zoomLevel = Plasma::GroupZoom;
        //connect to other containments
        //FIXME if some other view is zoomed out, a little madness will ensue
        Plasma::Corona *corona = containment()->corona();
        if (corona) {
            QList<Plasma::Containment*> containments = corona->containments();
            foreach (Plasma::Containment *c, containments) {
                if (c == fromContainment ||
                    c->containmentType() == Plasma::Containment::PanelContainment) {
                    continue;
                }
                connectContainment(c);
            }
        }
    } else if (m_zoomLevel == Plasma::GroupZoom) {
        //make sure nobody can zoom out
        Plasma::Corona *corona = fromContainment->corona();
        if (corona) {
            QList<Plasma::Containment*> containments = corona->containments();
            foreach (Plasma::Containment *c, containments) {
                if (c->containmentType() == Plasma::Containment::PanelContainment) {
                    continue;
                }
                c->enableAction("zoom out", false);
            }
        }
        m_zoomLevel = Plasma::OverviewZoom;
    } else {
        fromContainment->enableAction("zoom out", false);
        return;
    }

    setDragMode(ScrollHandDrag);
    qreal factor = Plasma::scalingFactor(m_zoomLevel);
    qreal s = factor / matrix().m11();
    scale(s, s);
    setSceneRect(QRectF(0, 0, scene()->sceneRect().right(), scene()->sceneRect().bottom()));

    ensureVisible(fromContainment->sceneBoundingRect());
}

void DesktopView::showAppletBrowser()
{
    if (m_dashboard && m_dashboard->isVisible()) {
        return;
    }

    PlasmaApp::self()->showAppletBrowser(containment());
}

void DesktopView::wheelEvent(QWheelEvent* event)
{
    if (scene() && scene()->itemAt(event->pos())) {
        QGraphicsView::wheelEvent(event);
        return;
    }

    if (event->modifiers() & Qt::ControlModifier) {
        if (event->delta() < 0) {
            zoomOut(containment());
        } else {
            zoomIn(containment());
        }
    }
}

// This function is reimplemented from QGraphicsView to work around the problem
// that QPainter::fillRect(QRectF/QRect, QBrush), which QGraphicsView uses, is
// potentially slow when the anti-aliasing hint is set and as implemented won't
// hit accelerated code at all when it isn't set.  This implementation avoids
// the problem by using integer coordinates and by using drawTiledPixmap() in
// the case of a texture brush, and fillRect(QRect, QColor) in the case of a
// solid pattern.  As an additional optimization it draws the background with
// CompositionMode_Source.
void DesktopView::drawBackground(QPainter *painter, const QRectF &rect)
{
    const QPainter::CompositionMode savedMode = painter->compositionMode();
    const QBrush brush = backgroundBrush();

    switch (brush.style())
    {
    case Qt::TexturePattern:
    {
        // Note: this assumes that the brush origin is (0, 0), and that
        //       the brush has an identity transformation matrix.
        const QPixmap texture = brush.texture();
        QRect r = rect.toAlignedRect();
        r.setLeft(r.left() - (r.left() % texture.width()));
        r.setTop(r.top() - (r.top() % texture.height()));
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->drawTiledPixmap(r, texture);
        painter->setCompositionMode(savedMode);
        return;
    }

    case Qt::SolidPattern:
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect.toAlignedRect(), brush.color());
        painter->setCompositionMode(savedMode);
        return;

    default:
        QGraphicsView::drawBackground(painter, rect);
    }
}

void DesktopView::screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment* containment)
{
    kDebug() << "was, is, containment:" << wasScreen << isScreen << (QObject*)containment;
    if (containment->containmentType() == Plasma::Containment::PanelContainment) {
        // we don't care about panel containments changing screens on us
        return;
    }

    if (wasScreen == screen()) {
        if (this->containment() == containment) {
            setContainment(0);
        }
    }

    if (isScreen == screen()) {
        setContainment(containment);
    }
}

void DesktopView::nextContainment()
{
    QList<Plasma::Containment*> containments = containment()->corona()->containments();
    int start = containments.indexOf(containment());
    int i = (start + 1) % containments.size();
    //FIXME this is a *horrible* way of choosing a "next" containment.
    while (i != start) {
        if (containments.at(i)->containmentType() != Plasma::Containment::PanelContainment &&
            containments.at(i)->screen() == -1) {
            break;
        }
        i = (i + 1) % containments.size();
    }

    Plasma::Containment *c = containments.at(i);
    setContainment(c);
}

void DesktopView::previousContainment()
{
    QList<Plasma::Containment*> containments = containment()->corona()->containments();
    int start = containments.indexOf(containment());
    //fun fact: in c++, (-1 % foo) == -1
    int i = start - 1;
    if (i < 0) {
        i += containments.size();
    }
    //FIXME this is a *horrible* way of choosing a "previous" containment.
    while (i != start) {
        if (containments.at(i)->containmentType() != Plasma::Containment::PanelContainment &&
            containments.at(i)->screen() == -1) {
            break;
        }
        if (--i < 0) {
            i += containments.size();
        }
    }

    Plasma::Containment *c = containments.at(i);
    setContainment(c);
}

#include "desktopview.moc"

