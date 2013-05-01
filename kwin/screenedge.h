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

#ifndef KWIN_SCREENEDGE_H
#define KWIN_SCREENEDGE_H
// KWin
#include "kwinglobals.h"
#include "xcbutils.h"
// KDE includes
#include <KDE/KConfig>
// Qt
#include <QObject>
#include <QVector>
#include <QDateTime>

namespace KWin {

class ScreenEdges;

class Edge : public QObject
{
    Q_OBJECT
public:
    explicit Edge(ScreenEdges *parent);
    virtual ~Edge();
    bool isLeft() const;
    bool isTop() const;
    bool isRight() const;
    bool isBottom() const;
    bool isCorner() const;
    bool isScreenEdge() const;
    bool triggersFor(const QPoint &cursorPos) const;
    void check(const QPoint &cursorPos, const QDateTime &triggerTime, bool forceNoPushBack = false);
    bool isReserved() const;
    const QRect &approachGeometry() const;

    ElectricBorder border() const;
    void reserve(QObject *object, const char *slot);
    const QHash<QObject *, QByteArray> &callBacks() const;
    void startApproaching();
    void stopApproaching();

public Q_SLOTS:
    void reserve();
    void unreserve();
    void unreserve(QObject *object);
    void setBorder(ElectricBorder border);
    void setAction(ElectricBorderAction action);
    void setGeometry(const QRect &geometry);
    void updateApproaching(const QPoint &point);
    void checkBlocking();
Q_SIGNALS:
    void approaching(ElectricBorder border, qreal factor, const QRect &geometry);
protected:
    ScreenEdges *edges();
    const ScreenEdges *edges() const;
    const QRect &geometry() const;
    bool isBlocked() const;
    virtual void doGeometryUpdate();
    virtual void activate();
    virtual void deactivate();
    virtual void doStartApproaching();
    virtual void doStopApproaching();
    virtual void doUpdateBlocking();
private:
    bool canActivate(const QPoint &cursorPos, const QDateTime &triggerTime);
    void handle(const QPoint &cursorPos);
    bool handleAction();
    bool handleByCallback();
    void switchDesktop(const QPoint &cursorPos);
    void pushCursorBack(const QPoint &cursorPos);
    ScreenEdges *m_edges;
    ElectricBorder m_border;
    ElectricBorderAction m_action;
    int m_reserved;
    QRect m_geometry;
    QRect m_approachGeometry;
    QDateTime m_lastTrigger;
    QDateTime m_lastReset;
    QPoint m_triggeredPoint;
    QHash<QObject *, QByteArray> m_callBacks;
    bool m_approaching;
    int m_lastApproachingFactor;
    bool m_blocked;
};

class WindowBasedEdge : public Edge
{
    Q_OBJECT
public:
    explicit WindowBasedEdge(ScreenEdges *parent);
    virtual ~WindowBasedEdge();

    xcb_window_t window() const;
    /**
     * The approach window is a special window to notice when get close to the screen border but
     * not yet triggering the border.
     **/
    xcb_window_t approachWindow() const;

protected:
    virtual void doGeometryUpdate();
    virtual void activate();
    virtual void deactivate();
    virtual void doStartApproaching();
    virtual void doStopApproaching();
    virtual void doUpdateBlocking();

private:
    void createWindow();
    void createApproachWindow();
    Xcb::Window m_window;
    Xcb::Window m_approachWindow;
};

/**
 * @short Class for controlling screen edges.
 *
 * The screen edge functionality is split into three parts:
 * @li This manager class ScreenEdges
 * @li abstract class @link Edge
 * @li specific implementation of @link Edge, e.g. @link WindowBasedEdge
 *
 * The ScreenEdges creates an @link Edge for each screen edge which is also an edge in the
 * combination of all screens. E.g. if there are two screens, no Edge is created between the screens,
 * but at all other edges even if the screens have a different dimension.
 *
 * In addition at each corner of the overall display geometry an one-pixel large @link Edge is
 * created. No matter how many screens there are, there will only be exactly four of these corner
 * edges. This is motivated by Fitts's Law which show that it's easy to trigger such a corner, but
 * it would be very difficult to trigger a corner between two screens (one pixel target not visually
 * outlined).
 *
 * The ScreenEdges are used for one of the following functionality:
 * @li switch virtual desktop (see property @link desktopSwitching)
 * @li switch virtual desktop when moving a window (see property @link desktopSwitchingMovingClients)
 * @li trigger a pre-defined action (see properties @link actionTop and similar)
 * @li trigger an externally configured action (e.g. Effect, Script, see @link reserve, @link unreserve)
 *
 * An @link Edge is only active if there is at least one of the possible actions "reserved" for this
 * edge. The idea is to not block the screen edge if nothing could be triggered there, so that the
 * user e.g. can configure nothing on the top edge, which tends to interfere with full screen apps
 * having a hidden panel there. On X11 (currently only supported backend) the @link Edge is
 * represented by a @link WindowBasedEdge which creates an input only window for the geometry and
 * reacts on enter notify events. If the edge gets reserved for the first time a window is created
 * and mapped, once the edge gets unreserved again, the window gets destroyed.
 *
 * When the mouse enters one of the screen edges the following values are used to determine whether
 * the action should be triggered or the cursor be pushed back
 * @li Time difference between two entering events is not larger than a certain threshold
 * @li Time difference between two entering events is larger than @link timeThreshold
 * @li Time difference between two activations is larger than @link reActivateThreshold
 * @li Distance between two enter events is not larger than a defined pixel distance
 * These checks are performed in @link Edge
 *
 * @todo change way how Effects/Scripts can reserve an edge and are notified.
 */
class ScreenEdges : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool desktopSwitching READ isDesktopSwitching)
    Q_PROPERTY(bool desktopSwitchingMovingClients READ isDesktopSwitchingMovingClients)
    Q_PROPERTY(QSize cursorPushBackDistance READ cursorPushBackDistance)
    Q_PROPERTY(int timeThreshold READ timeThreshold)
    Q_PROPERTY(int reActivateThreshold READ reActivationThreshold)
    Q_PROPERTY(int actionTopLeft READ actionTopLeft)
    Q_PROPERTY(int actionTop READ actionTop)
    Q_PROPERTY(int actionTopRight READ actionTopRight)
    Q_PROPERTY(int actionRight READ actionRight)
    Q_PROPERTY(int actionBottomRight READ actionBottomRight)
    Q_PROPERTY(int actionBottom READ actionBottom)
    Q_PROPERTY(int actionBottomLeft READ actionBottomLeft)
    Q_PROPERTY(int actionLeft READ actionLeft)
public:
    virtual ~ScreenEdges();
    /**
     * @internal
     **/
    void setConfig(KSharedConfig::Ptr config);
    /**
     * Initialize the screen edges.
     * @internal
     */
    void init();
    /**
     * Check, if a screen edge is entered and trigger the appropriate action
     * if one is enabled for the current region and the timeout is satisfied
     * @param pos the position of the mouse pointer
     * @param now the time when the function is called
     * @param forceNoPushBack needs to be called to workaround some DnD clients, don't use unless you want to chek on a DnD event
     */
    void check(const QPoint& pos, const QDateTime &now, bool forceNoPushBack = false);
    /**
     * The (dpi dependent) length, reserved for the active corners of each edge - 1/3"
     */
    int cornerOffset() const;
    /**
     * Mark the specified screen edge as reserved. This method is provided for external activation
     * like effects and scripts. When the effect/script does no longer need the edge it is supposed
     * to call @link unreserve.
     * @param border the screen edge to mark as reserved
     * @param object The object on which the @p callback needs to be invoked
     * @param callback The method name to be invoked - uses QMetaObject::invokeMethod
     * @see unreserve
     * @todo: add pointer to script/effect
     */
    void reserve(ElectricBorder border, QObject *object, const char *callback);
    /**
     * Mark the specified screen edge as unreserved. This method is provided for external activation
     * like effects and scripts. This method is only allowed to be called if @link reserve had been
     * called before for the same @p border. An unbalanced calling of reserve/unreserve leads to the
     * edge never being active or never being able to deactivate again.
     * @param border the screen edge to mark as unreserved
     * @param object the object on which the callback had been invoked
     * @see reserve
     * @todo: add pointer to script/effect
     */
    void unreserve(ElectricBorder border, QObject *object);
    /**
     * Reserve desktop switching for screen edges, if @p isToReserve is @c true. Unreserve otherwise.
     * @param reserve indicated weather desktop switching should be reserved or unreseved
     */
    void reserveDesktopSwitching(bool isToReserve, Qt::Orientations o);
    /**
     * Raise electric border windows to the real top of the screen. We only need
     * to do this if an effect input window is active.
     */
    void ensureOnTop();
    /**
    * Called when the user entered an electric border with the mouse.
    * It may switch to another virtual desktop.
    * @param e the X event which is passed to this method.
    */
    bool isEntered(XEvent * e);
    bool isEntered(xcb_generic_event_t *e);

    /**
     * Returns a QVector of all existing screen edge windows
     * @return all existing screen edge windows in a QVector
     */
    QVector< xcb_window_t > windows() const;

    bool isDesktopSwitching() const;
    bool isDesktopSwitchingMovingClients() const;
    const QSize &cursorPushBackDistance() const;
    /**
     * Minimum time between the push back of the cursor and the activation by re-entering the edge.
     **/
    int timeThreshold() const;
    /**
     * Minimum time between triggers
     **/
    int reActivationThreshold() const;
    ElectricBorderAction actionTopLeft() const;
    ElectricBorderAction actionTop() const;
    ElectricBorderAction actionTopRight() const;
    ElectricBorderAction actionRight() const;
    ElectricBorderAction actionBottomRight() const;
    ElectricBorderAction actionBottom() const;
    ElectricBorderAction actionBottomLeft() const;
    ElectricBorderAction actionLeft() const;

public Q_SLOTS:
    void reconfigure();
    /**
     * Updates the layout of virtual desktops and adjust the reserved borders in case of
     * virtual desktop switching on edges.
     **/
    void updateLayout();
    /**
     * Recreates all edges e.g. after the screen size changes.
     **/
    void recreateEdges();

Q_SIGNALS:
    /**
     * Signal emitted during approaching of mouse towards @p border. The @p factor indicates how
     * far away the mouse is from the approaching area. The values are clamped into [0.0,1.0] with
     * @c 0.0 meaning far away from the border, @c 1.0 in trigger distance.
     **/
    void approaching(ElectricBorder border, qreal factor, const QRect &geometry);
    void checkBlocking();

private:
    enum { ElectricDisabled = 0, ElectricMoveOnly = 1, ElectricAlways = 2 };
    void setDesktopSwitching(bool enable);
    void setDesktopSwitchingMovingClients(bool enable);
    void setCursorPushBackDistance(const QSize &distance);
    void setTimeThreshold(int threshold);
    void setReActivationThreshold(int threshold);
    void createHorizontalEdge(ElectricBorder border, const QRect &screen, const QRect &fullArea);
    void createVerticalEdge(ElectricBorder border, const QRect &screen, const QRect &fullArea);
    WindowBasedEdge *createEdge(ElectricBorder border, int x, int y, int width, int height);
    void setActionForBorder(ElectricBorder border, ElectricBorderAction *oldValue, ElectricBorderAction newValue);
    ElectricBorderAction actionForEdge(Edge *edge) const;
    bool handleEnterNotifiy(xcb_window_t window, const QPoint &point, const QDateTime &timestamp);
    bool handleDndNotify(xcb_window_t window, const QPoint &point);
    bool m_desktopSwitching;
    bool m_desktopSwitchingMovingClients;
    QSize m_cursorPushBackDistance;
    int m_timeThreshold;
    int m_reactivateThreshold;
    Qt::Orientations m_virtualDesktopLayout;
    QList<WindowBasedEdge*> m_edges;
    KSharedConfig::Ptr m_config;
    ElectricBorderAction m_actionTopLeft;
    ElectricBorderAction m_actionTop;
    ElectricBorderAction m_actionTopRight;
    ElectricBorderAction m_actionRight;
    ElectricBorderAction m_actionBottomRight;
    ElectricBorderAction m_actionBottom;
    ElectricBorderAction m_actionBottomLeft;
    ElectricBorderAction m_actionLeft;
    int m_cornerOffset;

    KWIN_SINGLETON(ScreenEdges)
};

/**********************************************************
 * Inlines Edge
 *********************************************************/

inline bool Edge::isBottom() const
{
    return m_border == ElectricBottom || m_border == ElectricBottomLeft || m_border == ElectricBottomRight;
}

inline bool Edge::isLeft() const
{
    return m_border == ElectricLeft || m_border == ElectricTopLeft || m_border == ElectricBottomLeft;
}

inline bool Edge::isRight() const
{
    return m_border == ElectricRight || m_border == ElectricTopRight || m_border == ElectricBottomRight;
}

inline bool Edge::isTop() const
{
    return m_border == ElectricTop || m_border == ElectricTopLeft || m_border == ElectricTopRight;
}

inline bool Edge::isCorner() const
{
    return m_border == ElectricTopLeft
        || m_border == ElectricTopRight
        || m_border == ElectricBottomRight
        || m_border == ElectricBottomLeft;
}

inline bool Edge::isScreenEdge() const
{
    return m_border == ElectricLeft
        || m_border == ElectricRight
        || m_border == ElectricTop
        || m_border == ElectricBottom;
}

inline bool Edge::isReserved() const
{
    return m_reserved != 0;
}

inline void Edge::setAction(ElectricBorderAction action)
{
    m_action = action;
}

inline void Edge::setBorder(ElectricBorder border)
{
    m_border = border;
}

inline ScreenEdges *Edge::edges()
{
    return m_edges;
}

inline const ScreenEdges *Edge::edges() const
{
    return m_edges;
}

inline const QRect &Edge::geometry() const
{
    return m_geometry;
}

inline const QRect &Edge::approachGeometry() const
{
    return m_approachGeometry;
}

inline ElectricBorder Edge::border() const
{
    return m_border;
}

inline const QHash< QObject *, QByteArray > &Edge::callBacks() const
{
    return m_callBacks;
}

inline bool Edge::isBlocked() const
{
    return m_blocked;
}

/**********************************************************
 * Inlines WindowBasedEdge
 *********************************************************/

inline xcb_window_t WindowBasedEdge::window() const
{
    return m_window;
}

inline xcb_window_t WindowBasedEdge::approachWindow() const
{
    return m_approachWindow;
}

/**********************************************************
 * Inlines ScreenEdges
 *********************************************************/
inline void ScreenEdges::setConfig(KSharedConfig::Ptr config)
{
    m_config = config;
}

inline int ScreenEdges::cornerOffset() const {
    return m_cornerOffset;
}

inline const QSize &ScreenEdges::cursorPushBackDistance() const
{
    return m_cursorPushBackDistance;
}

inline bool ScreenEdges::isDesktopSwitching() const
{
    return m_desktopSwitching;
}

inline bool ScreenEdges::isDesktopSwitchingMovingClients() const
{
    return m_desktopSwitchingMovingClients;
}

inline int ScreenEdges::reActivationThreshold() const
{
    return m_reactivateThreshold;
}

inline int ScreenEdges::timeThreshold() const
{
    return m_timeThreshold;
}

inline void ScreenEdges::setCursorPushBackDistance(const QSize &distance)
{
    m_cursorPushBackDistance = distance;
}

inline void ScreenEdges::setDesktopSwitching(bool enable)
{
    if (enable == m_desktopSwitching) {
        return;
    }
    m_desktopSwitching = enable;
    reserveDesktopSwitching(enable, m_virtualDesktopLayout);
}

inline void ScreenEdges::setDesktopSwitchingMovingClients(bool enable)
{
    m_desktopSwitchingMovingClients = enable;
}

inline void ScreenEdges::setReActivationThreshold(int threshold)
{
    m_reactivateThreshold = threshold;
}

inline void ScreenEdges::setTimeThreshold(int threshold)
{
    m_timeThreshold = threshold;
}

#define ACTION( name ) \
inline ElectricBorderAction ScreenEdges::name() const \
{ \
    return m_##name; \
}

ACTION(actionTopLeft)
ACTION(actionTop)
ACTION(actionTopRight)
ACTION(actionRight)
ACTION(actionBottomRight)
ACTION(actionBottom)
ACTION(actionBottomLeft)
ACTION(actionLeft)

#undef ACTION

}
#endif // KWIN_SCREENEDGE_H
