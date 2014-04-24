/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2011 Arthur Arlt <a.arlt@stud.uni-heidelberg.de>
Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>

Since the functionality provided in this class has been moved from
class Workspace, it is not clear who exactly has written the code.
The list below contains the copyright holders of the class Workspace.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2009 Lucas Murray <lmurray@undefinedfire.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "screenedge.h"

// KWin
#include "atoms.h"
#include "client.h"
#include "cursor.h"
#include "effects.h"
#include "screens.h"
#include "utils.h"
#include "workspace.h"
#include "virtualdesktops.h"
// Qt
#include <QTimer>
#include <QVector>
#include <QTextStream>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingCall>

namespace KWin {

// Mouse should not move more than this many pixels
static const int DISTANCE_RESET = 30;

Edge::Edge(ScreenEdges *parent)
    : QObject(parent)
    , m_edges(parent)
    , m_border(ElectricNone)
    , m_action(ElectricActionNone)
    , m_reserved(0)
    , m_approaching(false)
    , m_lastApproachingFactor(0)
    , m_blocked(false)
{
}

Edge::~Edge()
{
}

void Edge::reserve()
{
    m_reserved++;
    if (m_reserved == 1) {
        // got activated
        activate();
    }
}

void Edge::reserve(QObject *object, const char *slot)
{
    connect(object, SIGNAL(destroyed(QObject*)), SLOT(unreserve(QObject*)));
    m_callBacks.insert(object, QByteArray(slot));
    reserve();
}

void Edge::unreserve()
{
    m_reserved--;
    if (m_reserved == 0) {
        // got deactivated
        deactivate();
    }
}
void Edge::unreserve(QObject *object)
{
    if (m_callBacks.contains(object)) {
        m_callBacks.remove(object);
        disconnect(object, SIGNAL(destroyed(QObject*)), this, SLOT(unreserve(QObject*)));
        unreserve();
    }
}

bool Edge::triggersFor(const QPoint &cursorPos) const
{
    if (isBlocked()) {
        return false;
    }
    if (!m_geometry.contains(cursorPos)) {
        return false;
    }
    if (isLeft() && cursorPos.x() != m_geometry.x()) {
        return false;
    }
    if (isRight() && cursorPos.x() != (m_geometry.x() + m_geometry.width() -1)) {
        return false;
    }
    if (isTop() && cursorPos.y() != m_geometry.y()) {
        return false;
    }
    if (isBottom() && cursorPos.y() != (m_geometry.y() + m_geometry.height() -1)) {
        return false;
    }
    return true;
}

void Edge::check(const QPoint &cursorPos, const QDateTime &triggerTime, bool forceNoPushBack)
{
    if (!triggersFor(cursorPos)) {
        return;
    }
    // no pushback so we have to activate at once
    bool directActivate = forceNoPushBack || edges()->cursorPushBackDistance().isNull();
    if (directActivate || canActivate(cursorPos, triggerTime)) {
        m_lastTrigger = triggerTime;
        m_lastReset = QDateTime(); // invalidate
        handle(cursorPos);
    } else {
        pushCursorBack(cursorPos);
    }
    m_triggeredPoint = cursorPos;
}

bool Edge::canActivate(const QPoint &cursorPos, const QDateTime &triggerTime)
{
    // we check whether either the timer has explicitly been invalidated (successfull trigger) or is
    // bigger than the reactivation threshold (activation "aborted", usually due to moving away the cursor
    // from the corner after successfull activation)
    // either condition means that "this is the first event in a new attempt"
    if (!m_lastReset.isValid() || m_lastReset.msecsTo(triggerTime) > edges()->reActivationThreshold()) {
        m_lastReset = triggerTime;
        return false;
    }
    if (m_lastTrigger.msecsTo(triggerTime) < edges()->reActivationThreshold()) {
        return false;
    }
    if (m_lastReset.msecsTo(triggerTime) < edges()->timeThreshold()) {
        return false;
    }
    // does the check on position make any sense at all?
    if ((cursorPos - m_triggeredPoint).manhattanLength() > DISTANCE_RESET) {
        return false;
    }
    return true;
}

void Edge::handle(const QPoint &cursorPos)
{
    if ((edges()->isDesktopSwitchingMovingClients() && Workspace::self()->getMovingClient()) ||
        (edges()->isDesktopSwitching() && isScreenEdge())) {
        // always switch desktops in case:
        // moving a Client and option for switch on client move is enabled
        // or switch on screen edge is enabled
        switchDesktop(cursorPos);
        return;
    }
    if (Workspace::self()->getMovingClient()) {
        // if we are moving a window we don't want to trigger the actions. This just results in
        // problems, e.g. Desktop Grid activated or screen locker activated which just cannot
        // work as we hold a grab.
        return;
    }
    if (handleAction() || handleByCallback()) {
        pushCursorBack(cursorPos);
        return;
    }
    if (edges()->isDesktopSwitching() && isCorner()) {
        // try again desktop switching for the corner
        switchDesktop(cursorPos);
    }
}

bool Edge::handleAction()
{
    switch (m_action) {
    case ElectricActionDashboard: { // Display Plasma dashboard
        QDBusInterface plasmaApp("org.kde.plasma-desktop", "/App");
        plasmaApp.asyncCall("toggleDashboard");
        return true;
    }
    case ElectricActionShowDesktop: {
        Workspace::self()->setShowingDesktop(!Workspace::self()->showingDesktop());
        return true;
    }
    case ElectricActionLockScreen: { // Lock the screen
        QDBusInterface screenSaver("org.kde.screensaver", "/ScreenSaver");
        screenSaver.asyncCall("Lock");
        return true;
    }
    default:
        return false;
    }
}

bool Edge::handleByCallback()
{
    if (m_callBacks.isEmpty()) {
        return false;
    }
    for (QHash<QObject *, QByteArray>::iterator it = m_callBacks.begin();
        it != m_callBacks.end();
        ++it) {
        bool retVal = false;
        QMetaObject::invokeMethod(it.key(), it.value().constData(), Q_RETURN_ARG(bool, retVal), Q_ARG(ElectricBorder, m_border));
        if (retVal) {
            return true;
        }
    }
    return false;
}

void Edge::switchDesktop(const QPoint &cursorPos)
{
    QPoint pos(cursorPos);
    VirtualDesktopManager *vds = VirtualDesktopManager::self();
    const uint oldDesktop = vds->current();
    uint desktop = oldDesktop;
    const int OFFSET = 2;
    if (isLeft()) {
        const uint interimDesktop = desktop;
        desktop = vds->toLeft(desktop, vds->isNavigationWrappingAround());
        if (desktop != interimDesktop)
            pos.setX(displayWidth() - 1 - OFFSET);
    } else if (isRight()) {
        const uint interimDesktop = desktop;
        desktop = vds->toRight(desktop, vds->isNavigationWrappingAround());
        if (desktop != interimDesktop)
            pos.setX(OFFSET);
    }
    if (isTop()) {
        const uint interimDesktop = desktop;
        desktop = vds->above(desktop, vds->isNavigationWrappingAround());
        if (desktop != interimDesktop)
            pos.setY(displayHeight() - 1 - OFFSET);
    } else if (isBottom()) {
        const uint interimDesktop = desktop;
        desktop = vds->below(desktop, vds->isNavigationWrappingAround());
        if (desktop != interimDesktop)
            pos.setY(OFFSET);
    }
    if (Client *c = Workspace::self()->getMovingClient()) {
        if (c->rules()->checkDesktop(desktop) != int(desktop)) {
            // user attempts to move a client to another desktop where it is ruleforced to not be
            return;
        }
    }
    vds->setCurrent(desktop);
    if (vds->current() != oldDesktop) {
        Cursor::setPos(pos);
    }
}

void Edge::pushCursorBack(const QPoint &cursorPos)
{
    int x = cursorPos.x();
    int y = cursorPos.y();
    const QSize &distance = edges()->cursorPushBackDistance();
    if (isLeft()) {
        x += distance.width();
    }
    if (isRight()) {
        x -= distance.width();
    }
    if (isTop()) {
        y += distance.height();
    }
    if (isBottom()) {
        y -= distance.height();
    }
    Cursor::setPos(x, y);
}

void Edge::setGeometry(const QRect &geometry)
{
    if (m_geometry == geometry) {
        return;
    }
    m_geometry = geometry;
    int x = m_geometry.x();
    int y = m_geometry.y();
    int width = m_geometry.width();
    int height = m_geometry.height();
    const int size = m_edges->cornerOffset();
    if (isCorner()) {
        if (isRight()) {
            x = x - size +1;
        }
        if (isBottom()) {
            y = y - size +1;
        }
        width = size;
        height = size;
    } else {
        if (isLeft()) {
            y += size + 1;
            width = size;
            height = height - size * 2;
        } else if (isRight()) {
            x = x - size + 1;
            y += size;
            width = size;
            height = height - size * 2;
        } else if (isTop()) {
            x += size;
            width = width - size * 2;
            height = size;
        } else if (isBottom()) {
            x += size;
            y = y - size +1;
            width = width - size * 2;
            height = size;
        }
    }
    m_approachGeometry = QRect(x, y, width, height);
    doGeometryUpdate();
}

void Edge::checkBlocking()
{
    if (isCorner()) {
        return;
    }
    bool newValue = false;
    if (Client *client = Workspace::self()->activeClient()) {
        newValue = client->isFullScreen() && client->geometry().contains(m_geometry.center());
    }
    if (newValue == m_blocked) {
        return;
    }
    m_blocked = newValue;
    doUpdateBlocking();
}

void Edge::doUpdateBlocking()
{
}

void Edge::doGeometryUpdate()
{
}

void Edge::activate()
{
}

void Edge::deactivate()
{
}

void Edge::startApproaching()
{
    if (m_approaching) {
        return;
    }
    m_approaching = true;
    doStartApproaching();
    m_lastApproachingFactor = 0;
    emit approaching(border(), 0.0, m_approachGeometry);
}

void Edge::doStartApproaching()
{
}

void Edge::stopApproaching()
{
    if (!m_approaching) {
        return;
    }
    m_approaching = false;
    doStopApproaching();
    m_lastApproachingFactor = 0;
    emit approaching(border(), 0.0, m_approachGeometry);
}

void Edge::doStopApproaching()
{
}

void Edge::updateApproaching(const QPoint &point)
{
    if (approachGeometry().contains(point)) {
        int factor = 0;
        const int edgeDistance = m_edges->cornerOffset();
        // manhattan length for our edge
        const int cornerDistance = 2*edgeDistance;
        switch (border()) {
        case ElectricTopLeft:
            factor = (point.manhattanLength()<<8) / cornerDistance;
            break;
        case ElectricTopRight:
            factor = ((point - approachGeometry().topRight()).manhattanLength()<<8) / cornerDistance;
            break;
        case ElectricBottomRight:
            factor = ((point - approachGeometry().bottomRight()).manhattanLength()<<8) / cornerDistance;
            break;
        case ElectricBottomLeft:
            factor = ((point - approachGeometry().bottomLeft()).manhattanLength()<<8) / cornerDistance;
            break;
        case ElectricTop:
            factor = (qAbs(point.y() - approachGeometry().y())<<8) / edgeDistance;
            break;
        case ElectricRight:
            factor = (qAbs(point.x() - approachGeometry().right())<<8) / edgeDistance;
            break;
        case ElectricBottom:
            factor = (qAbs(point.y() - approachGeometry().bottom())<<8) / edgeDistance;
            break;
        case ElectricLeft:
            factor = (qAbs(point.x() - approachGeometry().x())<<8) / edgeDistance;
            break;
        default:
            break;
        }
        factor = 256 - factor;
        if (m_lastApproachingFactor != factor) {
            m_lastApproachingFactor = factor;
            emit approaching(border(), m_lastApproachingFactor/256.0f, m_approachGeometry);
        }
    } else {
        stopApproaching();
    }
}

/**********************************************************
 * ScreenEdges
 *********************************************************/
WindowBasedEdge::WindowBasedEdge(ScreenEdges *parent)
    : Edge(parent)
    , m_window(XCB_WINDOW_NONE)
    , m_approachWindow(XCB_WINDOW_NONE)
{
}

WindowBasedEdge::~WindowBasedEdge()
{
}

void WindowBasedEdge::activate()
{
    createWindow();
    createApproachWindow();
    doUpdateBlocking();
}

void WindowBasedEdge::deactivate()
{
    m_window.reset();
    m_approachWindow.reset();
}

void WindowBasedEdge::createWindow()
{
    if (m_window.isValid()) {
        return;
    }
    const uint32_t mask = XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
    const uint32_t values[] = {
        true,
        XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW
    };
    m_window.create(geometry(), XCB_WINDOW_CLASS_INPUT_ONLY, mask, values);
    m_window.map();
    // Set XdndAware on the windows, so that DND enter events are received (#86998)
    xcb_atom_t version = 4; // XDND version
    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_window,
                        atoms->xdnd_aware, XCB_ATOM_ATOM, 32, 1, (unsigned char*)(&version));
}

void WindowBasedEdge::createApproachWindow()
{
    if (m_approachWindow.isValid()) {
        return;
    }
    if (!approachGeometry().isValid()) {
        return;
    }
    const uint32_t mask = XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
    const uint32_t values[] = {
        true,
        XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW
    };
    m_approachWindow.create(approachGeometry(), XCB_WINDOW_CLASS_INPUT_ONLY, mask, values);
    m_approachWindow.map();
}

void WindowBasedEdge::doGeometryUpdate()
{
    m_window.setGeometry(geometry());
    m_approachWindow.setGeometry(approachGeometry());
}

void WindowBasedEdge::doStartApproaching()
{
    m_approachWindow.unmap();
    Cursor *cursor = Cursor::self();
    connect(cursor, SIGNAL(posChanged(QPoint)), SLOT(updateApproaching(QPoint)));
    cursor->startMousePolling();
}

void WindowBasedEdge::doStopApproaching()
{
    Cursor *cursor = Cursor::self();
    disconnect(cursor, SIGNAL(posChanged(QPoint)), this, SLOT(updateApproaching(QPoint)));
    cursor->stopMousePolling();
    m_approachWindow.map();
}

void WindowBasedEdge::doUpdateBlocking()
{
    if (!isReserved()) {
        return;
    }
    if (isBlocked()) {
        m_window.unmap();
        m_approachWindow.unmap();
    } else {
        m_window.map();
        m_approachWindow.map();
    }
}

/**********************************************************
 * ScreenEdges
 *********************************************************/
KWIN_SINGLETON_FACTORY(ScreenEdges)

ScreenEdges::ScreenEdges(QObject *parent)
    : QObject(parent)
    , m_desktopSwitching(false)
    , m_desktopSwitchingMovingClients(false)
    , m_timeThreshold(0)
    , m_reactivateThreshold(0)
    , m_virtualDesktopLayout(0)
    , m_actionTopLeft(ElectricActionNone)
    , m_actionTop(ElectricActionNone)
    , m_actionTopRight(ElectricActionNone)
    , m_actionRight(ElectricActionNone)
    , m_actionBottomRight(ElectricActionNone)
    , m_actionBottom(ElectricActionNone)
    , m_actionBottomLeft(ElectricActionNone)
    , m_actionLeft(ElectricActionNone)
{
    QWidget w;
    m_cornerOffset = (w.physicalDpiX() + w.physicalDpiY() + 5) / 6;
}

ScreenEdges::~ScreenEdges()
{
    s_self = NULL;
}

void ScreenEdges::init()
{
    reconfigure();
    updateLayout();
    recreateEdges();
}
static ElectricBorderAction electricBorderAction(const QString& name)
{
    QString lowerName = name.toLower();
    if (lowerName == "dashboard") {
        return ElectricActionDashboard;
    } else if (lowerName == "showdesktop") {
        return ElectricActionShowDesktop;
    } else if (lowerName == "lockscreen") {
        return ElectricActionLockScreen;
    } else if (lowerName == "preventscreenlocking") {
        return ElectricActionPreventScreenLocking;
    }
    return ElectricActionNone;
}

void ScreenEdges::reconfigure()
{
    if (!m_config) {
        return;
    }
    // TODO: migrate settings to a group ScreenEdges
    KConfigGroup windowsConfig = m_config->group("Windows");
    setTimeThreshold(windowsConfig.readEntry("ElectricBorderDelay", 150));
    setReActivationThreshold(qMax(timeThreshold() + 50, windowsConfig.readEntry("ElectricBorderCooldown", 350)));
    int desktopSwitching = windowsConfig.readEntry("ElectricBorders", static_cast<int>(ElectricDisabled));
    if (desktopSwitching == ElectricDisabled) {
        setDesktopSwitching(false);
        setDesktopSwitchingMovingClients(false);
    } else if (desktopSwitching == ElectricMoveOnly) {
        setDesktopSwitching(false);
        setDesktopSwitchingMovingClients(true);
    } else if (desktopSwitching == ElectricAlways) {
        setDesktopSwitching(true);
        setDesktopSwitchingMovingClients(true);
    }
    const int pushBack = windowsConfig.readEntry("ElectricBorderPushbackPixels", 1);
    m_cursorPushBackDistance = QSize(pushBack, pushBack);

    KConfigGroup borderConfig = m_config->group("ElectricBorders");
    setActionForBorder(ElectricTopLeft,     &m_actionTopLeft,
                       electricBorderAction(borderConfig.readEntry("TopLeft", "None")));
    setActionForBorder(ElectricTop,         &m_actionTop,
                       electricBorderAction(borderConfig.readEntry("Top", "None")));
    setActionForBorder(ElectricTopRight,    &m_actionTopRight,
                       electricBorderAction(borderConfig.readEntry("TopRight", "None")));
    setActionForBorder(ElectricRight,       &m_actionRight,
                       electricBorderAction(borderConfig.readEntry("Right", "None")));
    setActionForBorder(ElectricBottomRight, &m_actionBottomRight,
                       electricBorderAction(borderConfig.readEntry("BottomRight", "None")));
    setActionForBorder(ElectricBottom,      &m_actionBottom,
                       electricBorderAction(borderConfig.readEntry("Bottom", "None")));
    setActionForBorder(ElectricBottomLeft,  &m_actionBottomLeft,
                       electricBorderAction(borderConfig.readEntry("BottomLeft", "None")));
    setActionForBorder(ElectricLeft,        &m_actionLeft,
                       electricBorderAction(borderConfig.readEntry("Left", "None")));
}

void ScreenEdges::setActionForBorder(ElectricBorder border, ElectricBorderAction *oldValue, ElectricBorderAction newValue)
{
    if (*oldValue == newValue) {
        return;
    }
    if (*oldValue == ElectricActionNone) {
        // have to reserve
        for (QList<WindowBasedEdge*>::iterator it = m_edges.begin(); it != m_edges.end(); ++it) {
            if ((*it)->border() == border) {
                (*it)->reserve();
            }
        }
    }
    if (newValue == ElectricActionNone) {
        // have to unreserve
        for (QList<WindowBasedEdge*>::iterator it = m_edges.begin(); it != m_edges.end(); ++it) {
            if ((*it)->border() == border) {
                (*it)->unreserve();
            }
        }
    }
    *oldValue = newValue;
    // update action on all Edges for given border
    for (QList<WindowBasedEdge*>::iterator it = m_edges.begin(); it != m_edges.end(); ++it) {
        if ((*it)->border() == border) {
            (*it)->setAction(newValue);
        }
    }
}

void ScreenEdges::updateLayout()
{
    const QSize desktopMatrix = VirtualDesktopManager::self()->grid().size();
    Qt::Orientations newLayout = 0;
    if (desktopMatrix.width() > 1) {
        newLayout |= Qt::Horizontal;
    }
    if (desktopMatrix.height() > 1) {
        newLayout |= Qt::Vertical;
    }
    if (newLayout == m_virtualDesktopLayout) {
        return;
    }
    if (isDesktopSwitching()) {
        reserveDesktopSwitching(false, m_virtualDesktopLayout);
    }
    m_virtualDesktopLayout = newLayout;
    if (isDesktopSwitching()) {
        reserveDesktopSwitching(true, m_virtualDesktopLayout);
    }
}

static bool isLeftScreen(const QRect &screen, const QRect &fullArea)
{
    if (screens()->count() == 1) {
        return true;
    }
    if (screen.x() == fullArea.x()) {
        return true;
    }
    // the screen is also on the left in case of a vertical layout with a second screen
    // more to the left. In that case no screen ends left of screen's x coord
    for (int i=0; i<screens()->count(); ++i) {
        const QRect otherGeo = screens()->geometry(i);
        if (otherGeo == screen) {
            // that's our screen to test
            continue;
        }
        if (otherGeo.x() + otherGeo.width() <= screen.x()) {
            // other screen is completely in the left
            return false;
        }
    }
    // did not find a screen left of our current screen, so it is the left most
    return true;
}

static bool isRightScreen(const QRect &screen, const QRect &fullArea)
{
    if (screens()->count() == 1) {
        return true;
    }
    if (screen.x() + screen.width() == fullArea.x() + fullArea.width()) {
        return true;
    }
    // the screen is also on the right in case of a vertical layout with a second screen
    // more to the right. In that case no screen starts right of this screen
    for (int i=0; i<screens()->count(); ++i) {
        const QRect otherGeo = screens()->geometry(i);
        if (otherGeo == screen) {
            // that's our screen to test
            continue;
        }
        if (otherGeo.x() >= screen.x() + screen.width()) {
            // other screen is completely in the right
            return false;
        }
    }
    // did not find a screen right of our current screen, so it is the right most
    return true;
}

static bool isTopScreen(const QRect &screen, const QRect &fullArea)
{
    if (screens()->count() == 1) {
        return true;
    }
    if (screen.y() == fullArea.y()) {
        return true;
    }
    // the screen is also top most in case of a horizontal layout with a second screen
    // more to the top. In that case no screen ends above screen's y coord
    for (int i=0; i<screens()->count(); ++i) {
        const QRect otherGeo = screens()->geometry(i);
        if (otherGeo == screen) {
            // that's our screen to test
            continue;
        }
        if (otherGeo.y() + otherGeo.height() <= screen.y()) {
            // other screen is completely above
            return false;
        }
    }
    // did not find a screen above our current screen, so it is the top most
    return true;
}

static bool isBottomScreen(const QRect &screen, const QRect &fullArea)
{
    if (screens()->count() == 1) {
        return true;
    }
    if (screen.y() + screen.height() == fullArea.y() + fullArea.height()) {
        return true;
    }
    // the screen is also bottom most in case of a horizontal layout with a second screen
    // more below. In that case no screen starts below screen's y coord + height
    for (int i=0; i<screens()->count(); ++i) {
        const QRect otherGeo = screens()->geometry(i);
        if (otherGeo == screen) {
            // that's our screen to test
            continue;
        }
        if (otherGeo.y() >= screen.y() + screen.height()) {
            // other screen is completely below
            return false;
        }
    }
    // did not find a screen below our current screen, so it is the bottom most
    return true;
}

void ScreenEdges::recreateEdges()
{
    QList<WindowBasedEdge*> oldEdges(m_edges);
    m_edges.clear();
    const QRect fullArea(0, 0, displayWidth(), displayHeight());
    for (int i=0; i<screens()->count(); ++i) {
        const QRect screen = screens()->geometry(i);
        if (isLeftScreen(screen, fullArea)) {
            // left most screen
            createVerticalEdge(ElectricLeft, screen, fullArea);
        }
        if (isRightScreen(screen, fullArea)) {
            // right most screen
            createVerticalEdge(ElectricRight, screen, fullArea);
        }
        if (isTopScreen(screen, fullArea)) {
            // top most screen
            createHorizontalEdge(ElectricTop, screen, fullArea);
        }
        if (isBottomScreen(screen, fullArea)) {
            // bottom most screen
            createHorizontalEdge(ElectricBottom, screen, fullArea);
        }
    }
    // copy over the effect/script reservations from the old edges
    for (QList<WindowBasedEdge*>::iterator it = m_edges.begin(); it != m_edges.end(); ++it) {
        WindowBasedEdge *edge = *it;
        for (QList<WindowBasedEdge*>::const_iterator oldIt = oldEdges.constBegin();
                oldIt != oldEdges.constEnd();
                ++oldIt) {
            WindowBasedEdge *oldEdge = *oldIt;
            if (oldEdge->border() != edge->border()) {
                continue;
            }
            const QHash<QObject *, QByteArray> &callbacks = oldEdge->callBacks();
            for (QHash<QObject *, QByteArray>::const_iterator callback = callbacks.begin();
                    callback != callbacks.end();
                    ++callback) {
                edge->reserve(callback.key(), callback.value().constData());
            }
        }
    }
    qDeleteAll(oldEdges);
}

void ScreenEdges::createVerticalEdge(ElectricBorder border, const QRect &screen, const QRect &fullArea)
{
    if (border != ElectricRight && border != KWin::ElectricLeft) {
        return;
    }
    int y = screen.y();
    int height = screen.height();
    const int x = (border == ElectricLeft) ? screen.x() : screen.x() + screen.width() -1;
    if (isTopScreen(screen, fullArea)) {
        // also top most screen
        height -= m_cornerOffset;
        y += m_cornerOffset;
        // create top left/right edge
        const ElectricBorder edge = (border == ElectricLeft) ? ElectricTopLeft : ElectricTopRight;
        m_edges << createEdge(edge, x, screen.y(), 1, 1);
    }
    if (isBottomScreen(screen, fullArea)) {
        // also bottom most screen
        height -= m_cornerOffset;
        // create bottom left/right edge
        const ElectricBorder edge = (border == ElectricLeft) ? ElectricBottomLeft : ElectricBottomRight;
        m_edges << createEdge(edge, x, screen.y() + screen.height() -1, 1, 1);
    }
    // create border
    m_edges << createEdge(border, x, y, 1, height);
}

void ScreenEdges::createHorizontalEdge(ElectricBorder border, const QRect &screen, const QRect &fullArea)
{
    if (border != ElectricTop && border != ElectricBottom) {
        return;
    }
    int x = screen.x();
    int width = screen.width();
    if (isLeftScreen(screen, fullArea)) {
        // also left most - adjust only x and width
        x += m_cornerOffset;
        width -= m_cornerOffset;
    }
    if (isRightScreen(screen, fullArea)) {
        // also right most edge
        width -= m_cornerOffset;
    }
    const int y = (border == ElectricTop) ? screen.y() : screen.y() + screen.height() - 1;
    m_edges << createEdge(border, x, y, width, 1);
}

WindowBasedEdge *ScreenEdges::createEdge(ElectricBorder border, int x, int y, int width, int height)
{
    WindowBasedEdge *edge = new WindowBasedEdge(this);
    edge->setBorder(border);
    edge->setGeometry(QRect(x, y, width, height));
    const ElectricBorderAction action = actionForEdge(edge);
    if (action != KWin::ElectricActionNone) {
        edge->reserve();
        edge->setAction(action);
    }
    if (isDesktopSwitching()) {
        if (edge->isCorner()) {
            edge->reserve();
        } else {
            if ((m_virtualDesktopLayout & Qt::Horizontal) && (edge->isLeft() || edge->isRight())) {
                edge->reserve();
            }
            if ((m_virtualDesktopLayout & Qt::Vertical) && (edge->isTop() || edge->isBottom())) {
                edge->reserve();
            }
        }
    }
    connect(edge, SIGNAL(approaching(ElectricBorder,qreal,QRect)), SIGNAL(approaching(ElectricBorder,qreal,QRect)));
    if (edge->isScreenEdge()) {
        connect(this, SIGNAL(checkBlocking()), edge, SLOT(checkBlocking()));
    }
    return edge;
}

ElectricBorderAction ScreenEdges::actionForEdge(Edge *edge) const
{
    switch (edge->border()) {
    case ElectricTopLeft:
        return m_actionTopLeft;
    case ElectricTop:
        return m_actionTop;
    case ElectricTopRight:
        return m_actionTopRight;
    case ElectricRight:
        return m_actionRight;
    case ElectricBottomRight:
        return m_actionBottomRight;
    case ElectricBottom:
        return m_actionBottom;
    case ElectricBottomLeft:
        return m_actionBottomLeft;
    case ElectricLeft:
        return m_actionLeft;
    default:
        // fall through
        break;
    }
    return ElectricActionNone;
}

void ScreenEdges::reserveDesktopSwitching(bool isToReserve, Qt::Orientations o)
{
    if (!o)
        return;
    for (QList<WindowBasedEdge*>::iterator it = m_edges.begin(); it != m_edges.end(); ++it) {
        WindowBasedEdge *edge = *it;
        if (edge->isCorner()) {
            isToReserve ? edge->reserve() : edge->unreserve();
        } else {
            if ((m_virtualDesktopLayout & Qt::Horizontal) && (edge->isLeft() || edge->isRight())) {
                isToReserve ? edge->reserve() : edge->unreserve();
            }
            if ((m_virtualDesktopLayout & Qt::Vertical) && (edge->isTop() || edge->isBottom())) {
                isToReserve ? edge->reserve() : edge->unreserve();
            }
        }
    }
}

void ScreenEdges::reserve(ElectricBorder border, QObject *object, const char *slot)
{
    for (QList<WindowBasedEdge*>::iterator it = m_edges.begin(); it != m_edges.end(); ++it) {
        if ((*it)->border() == border) {
            (*it)->reserve(object, slot);
        }
    }
}

void ScreenEdges::unreserve(ElectricBorder border, QObject *object)
{
    for (QList<WindowBasedEdge*>::iterator it = m_edges.begin(); it != m_edges.end(); ++it) {
        if ((*it)->border() == border) {
            (*it)->unreserve(object);
        }
    }
}

void ScreenEdges::check(const QPoint &pos, const QDateTime &now, bool forceNoPushBack)
{
    for (QList<WindowBasedEdge*>::iterator it = m_edges.begin(); it != m_edges.end(); ++it) {
        (*it)->check(pos, now, forceNoPushBack);
    }
}

bool ScreenEdges::isEntered(XEvent* e)
{
    if (e->type == EnterNotify) {
        return handleEnterNotifiy(e->xcrossing.window,
                                  QPoint(e->xcrossing.x_root, e->xcrossing.y_root),
                                  QDateTime::fromMSecsSinceEpoch(e->xcrossing.time));
    }
    if (e->type == ClientMessage) {
        if (e->xclient.message_type == atoms->xdnd_position) {
            return handleDndNotify(e->xclient.window,
                                   QPoint(e->xclient.data.l[2] >> 16, e->xclient.data.l[2] & 0xffff));
        }
    }
    return false;
}

bool ScreenEdges::isEntered(xcb_generic_event_t *e)
{
    if (e->response_type == XCB_ENTER_NOTIFY) {
        xcb_enter_notify_event_t *event = reinterpret_cast<xcb_enter_notify_event_t*>(e);
        return handleEnterNotifiy(event->event,
                                  QPoint(event->root_x, event->root_y),
                                  QDateTime::fromMSecsSinceEpoch(event->time));
    }
    if (e->response_type == XCB_CLIENT_MESSAGE) {
        xcb_client_message_event_t *event = reinterpret_cast<xcb_client_message_event_t*>(e);
        return handleDndNotify(event->window,
                              QPoint(event->data.data32[2] >> 16, event->data.data32[2] & 0xffff));
    }
    return false;
}

bool ScreenEdges::handleEnterNotifiy(xcb_window_t window, const QPoint &point, const QDateTime &timestamp)
{
    for (QList<WindowBasedEdge*>::iterator it = m_edges.begin(); it != m_edges.end(); ++it) {
        WindowBasedEdge *edge = *it;
        if (!edge->isReserved()) {
            continue;
        }
        if (edge->window() == window) {
            edge->check(point, timestamp);
            return true;
        }
        if (edge->approachWindow() == window) {
            edge->startApproaching();
            // TODO: if it's a corner, it should also trigger for other windows
            return true;
        }
    }
    return false;
}

bool ScreenEdges::handleDndNotify(xcb_window_t window, const QPoint &point)
{
    for (QList<WindowBasedEdge*>::iterator it = m_edges.begin(); it != m_edges.end(); ++it) {
        WindowBasedEdge *edge = *it;
        if (edge->isReserved() && edge->window() == window) {
            updateXTime();
            edge->check(point, QDateTime::fromMSecsSinceEpoch(xTime()), true);
            return true;
        }
    }
    return false;
}

void ScreenEdges::ensureOnTop()
{
    Xcb::restackWindowsWithRaise(windows());
}

QVector< xcb_window_t > ScreenEdges::windows() const
{
    QVector<xcb_window_t> wins;
    for (QList<WindowBasedEdge*>::const_iterator it = m_edges.constBegin();
            it != m_edges.constEnd();
            ++it) {
        xcb_window_t w = (*it)->window();
        if (w != XCB_WINDOW_NONE) {
            wins.append(w);
        }
        // TODO:  lambda
        w = (*it)->approachWindow();
        if (w != XCB_WINDOW_NONE) {
            wins.append(w);
        }
    }
    return wins;
}

} //namespace
