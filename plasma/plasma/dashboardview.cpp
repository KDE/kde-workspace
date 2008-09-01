/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007 André Duffeck <duffeck@kde.org>
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

#include "dashboardview.h"

#include <QAction>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QTimer>

#include <KWindowSystem>

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/svg.h"
#include "plasma/appletbrowser.h"
#include "plasmaapp.h"

static const int SUPPRESS_SHOW_TIMEOUT = 500; // Number of millis to prevent reshow of dashboard

DashboardView::DashboardView(Plasma::Containment *containment, QWidget *parent)
    : Plasma::View(containment, parent),
      m_appletBrowser(0),
      m_suppressShow(false),
      m_zoomIn(false),
      m_zoomOut(false)
{
    //setContextMenuPolicy(Qt::NoContextMenu);
    setWindowFlags(Qt::FramelessWindowHint);
    if (!PlasmaApp::hasComposite()) {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_NoSystemBackground);
    }

    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->screenGeometry(containment->screen()));

    setWallpaperEnabled(!PlasmaApp::hasComposite());

    connect(scene(), SIGNAL(releaseVisualFocus()), SLOT(hideView()));

    m_hideAction = new QAction(i18n("Hide Dashboard"), this);
    m_hideAction->setIcon(KIcon("preferences-desktop-display"));
    m_hideAction->setEnabled(false);
    containment->addToolBoxTool(m_hideAction);
    connect(m_hideAction, SIGNAL(triggered()), this, SLOT(hideView()));

    installEventFilter(this);
}

DashboardView::~DashboardView()
{
    delete m_appletBrowser;
}

void DashboardView::drawBackground(QPainter * painter, const QRectF & rect)
{
    if (PlasmaApp::hasComposite()) {
        setWallpaperEnabled(false);
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect, QColor(0, 0, 0, 180));
    } else {
        setWallpaperEnabled(true);
        Plasma::View::drawBackground(painter, rect);
    }
}

void DashboardView::paintEvent(QPaintEvent *event)
{
    Plasma::View::paintEvent(event);

    // now draw a little label saying "this is your friendly neighbourhood dashboard"
    const QRect r = rect();
    const QString text = i18n("Plasma Dashboard");
    QFont f = font();
    f.bold();
    const QFontMetrics fm(f);
    const int margin = 6;
    const int textWidth = fm.width(text);
    const QPoint centered(r.width() / 2 - textWidth / 2 - margin, r.y());
    const QRect boundingBox(centered, QSize(margin * 2 + textWidth, fm.height() + margin * 2));

    if (!viewport() || !event->rect().intersects(boundingBox)) {
        return;
    }

    QPainterPath box;
    box.moveTo(boundingBox.topLeft());
    box.lineTo(boundingBox.bottomLeft() + QPoint(0, -margin * 2));
    box.quadTo(boundingBox.bottomLeft(), boundingBox.bottomLeft() + QPoint(margin * 2, 0));
    box.lineTo(boundingBox.bottomRight() + QPoint(-margin * 2, 0));
    box.quadTo(boundingBox.bottomRight(), boundingBox.bottomRight() + QPoint(0, -margin * 2));
    box.lineTo(boundingBox.topRight());
    box.closeSubpath();

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(f);
    //kDebug() << "******************** painting from" << centered << boundingBox << rect() << event->rect();
    QColor highlight = palette().highlight().color();
    highlight.setAlphaF(0.7);
    painter.setPen(highlight.darker());
    painter.setBrush(highlight);
    painter.drawPath(box);
    painter.setPen(palette().highlightedText().color());
    painter.drawText(boundingBox, Qt::AlignCenter | Qt::AlignVCenter, text);
}

void DashboardView::showAppletBrowser()
{
    if (!containment()) {
        return;
    }

    if (!m_appletBrowser) {
        m_appletBrowser = new Plasma::AppletBrowser(this, Qt::FramelessWindowHint );
        m_appletBrowser->setContainment(containment());
        //TODO: make this proportional to the screen
        m_appletBrowser->setInitialSize(QSize(400, 400));
        m_appletBrowser->setApplication();
        m_appletBrowser->setWindowTitle(i18n("Add Widgets"));
        QPalette p = m_appletBrowser->palette();
        p.setBrush(QPalette::Background, QBrush(QColor(0, 0, 0, 180)));
        m_appletBrowser->setPalette(p);
        m_appletBrowser->setBackgroundRole(QPalette::Background);
        m_appletBrowser->setAutoFillBackground(true);
        KWindowSystem::setState(m_appletBrowser->winId(), NET::KeepAbove|NET::SkipTaskbar);
        m_appletBrowser->move(0, 0);
        m_appletBrowser->installEventFilter(this);
    }

    m_appletBrowser->setHidden(m_appletBrowser->isVisible());
}

void DashboardView::appletBrowserDestroyed()
{
    m_appletBrowser = 0;
}

bool DashboardView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_appletBrowser) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                hideView();
            }
        }
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        m_appletBrowserDragStart = me->globalPos();
    } else if (event->type() == QEvent::MouseMove && m_appletBrowserDragStart != QPoint()) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        QPoint newPos = me->globalPos();
        QPoint curPos = m_appletBrowser->pos();
        int x = curPos.x();
        int y = curPos.y();

        if (curPos.y() == 0 || curPos.y() + m_appletBrowser->height() >= height()) {
           x = curPos.x() + (newPos.x() - m_appletBrowserDragStart.x());
           if (x < 0) {
               x = 0;
           } else if (x + m_appletBrowser->width() > width()) {
               x = width() - m_appletBrowser->width();
           }
        }

        if (x == 0 || x + m_appletBrowser->width() >= width()) {
            y = m_appletBrowser->y() + (newPos.y() - m_appletBrowserDragStart.y());

            if (y < 0) {
                y = 0;
            } else if (y + m_appletBrowser->height() > height()) {
                y = height() - m_appletBrowser->height();
            }
        }
        m_appletBrowser->move(x, y);
        m_appletBrowserDragStart = newPos;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        m_appletBrowserDragStart = QPoint();
    }

    return false;
}

void DashboardView::toggleVisibility()
{
    if (isHidden() && containment()) {
        if (m_suppressShow) {
            kDebug() << "DashboardView::toggleVisibility but show was suppressed";
            return;
        }

        setWindowState(Qt::WindowFullScreen);
        KWindowSystem::setOnAllDesktops(winId(), true);
        KWindowSystem::setState(winId(), NET::KeepAbove|NET::SkipTaskbar);

        QAction *action = containment()->action("zoom out");
        m_zoomOut = action ? action->isEnabled() : false;
        action = containment()->action("zoom in");
        m_zoomIn = action ? action->isEnabled() : false;

        m_hideAction->setEnabled(true);
        containment()->enableAction("zoom out", false);
        containment()->enableAction("zoom in", false);

        show();
        raise();

        m_suppressShow = true;
        QTimer::singleShot(SUPPRESS_SHOW_TIMEOUT, this, SLOT(suppressShowTimeout()));
        containment()->openToolBox();
    } else {
        hideView();
    }
}

void DashboardView::setContainment(Plasma::Containment *newContainment)
{
    if (!newContainment || newContainment == containment()) {
        return;
    }

    Plasma::Containment *oldContainment = containment();
    if (oldContainment) {
        oldContainment->removeToolBoxTool(m_hideAction);
    }
    newContainment->addToolBoxTool(m_hideAction);

    if (isVisible()) {
        if (oldContainment) {
            disconnect(oldContainment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));
            oldContainment->closeToolBox();
            oldContainment->enableAction("zoom out", m_zoomOut);
            oldContainment->enableAction("zoom in", m_zoomIn);
        }

        connect(newContainment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));
        QAction *action = newContainment->action("zoom out");
        m_zoomOut = action ? action->isEnabled() : false;
        action = newContainment->action("zoom in");
        m_zoomIn = action ? action->isEnabled() : false;
        newContainment->enableAction("zoom out", false);
        newContainment->enableAction("zoom in", false);
        newContainment->openToolBox();
    }

    if (m_appletBrowser) {
        m_appletBrowser->setContainment(newContainment);
    }

    View::setContainment(0); // we don't actually to mess with the screen settings
    View::setContainment(newContainment);
}

void DashboardView::hideView()
{
    if (m_appletBrowser) {
        m_appletBrowser->hide();
    }

#ifndef Q_WS_WIN
    disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(activeWindowChanged(WId)));
#endif

    if (containment()) {
        disconnect(containment(), SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));

        containment()->closeToolBox();
        containment()->enableAction("zoom out", m_zoomOut);
        containment()->enableAction("zoom in", m_zoomIn);
    }

    m_hideAction->setEnabled(false);
    hide();
}

void DashboardView::suppressShowTimeout()
{
    kDebug() << "DashboardView::suppressShowTimeout";
    m_suppressShow = false;
}

void DashboardView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideView();
        event->accept();
        return;
    }

    Plasma::View::keyPressEvent(event);
}

void DashboardView::activeWindowChanged(WId id)
{
    if (id != winId() &&
        (!m_appletBrowser || id != m_appletBrowser->winId())) {
        hideView();
    }
}

void DashboardView::showEvent(QShowEvent *event)
{
    KWindowSystem::setState(winId(), NET::SkipPager);
#ifndef Q_WS_WIN
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(activeWindowChanged(WId)));
#endif
    if (containment()) {
        connect(containment(), SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));
    }
    Plasma::View::showEvent(event);
}

#include "dashboardview.moc"

