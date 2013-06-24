/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef KWIN_CURSOR_H
#define KWIN_CURSOR_H
// kwin
#include <kwinglobals.h>
// Qt
#include <QHash>
#include <QObject>
#include <QPoint>
// xcb
#include <xcb/xcb.h>

class QTimer;

namespace KWin
{

/**
 * @short Replacement for QCursor.
 *
 * This class provides a similar API to QCursor and should be preferred inside KWin. It allows to
 * get the position and warp the mouse cursor with static methods just like QCursor. It also provides
 * the possibility to get an X11 cursor for a Qt::CursorShape - a functionality lost in Qt 5's QCursor
 * implementation.
 *
 * In addition the class provides a mouse polling facility as required by e.g. Effects and ScreenEdges
 * and emits signals when the mouse position changes. In opposite to QCursor this class is a QObject
 * and cannot be constructed. Instead it provides a singleton getter, though the most important
 * methods are wrapped in a static method, just like QCursor.
 *
 * The actual implementation is split into two parts: a system independent interface and a windowing
 * system specific subclass. So far only an X11 backend is implemented which uses query pointer to
 * fetch the position and warp pointer to set the position. It uses a timer based mouse polling and
 * can provide X11 cursors through the XCursor library.
 **/
class Cursor : public QObject
{
    Q_OBJECT
public:
    virtual ~Cursor();
    void startMousePolling();
    void stopMousePolling();
    /**
     * @brief Enables tracking changes of cursor images.
     *
     * After enabling cursor change tracking the signal @link cursorChanged will be emitted
     * whenever a change to the cursor image is recognized.
     *
     * Use @link stopCursorTracking to no longer emit this signal. Note: the signal will be
     * emitted until each call of this method has been matched with a call to stopCursorTracking.
     *
     * This tracking is not about pointer position tracking.
     * @see stopCursorTracking
     * @see cursorChanged
     */
    void startCursorTracking();
    /**
     * @brief Disables tracking changes of cursor images.
     *
     * Only call after using @link startCursorTracking.
     *
     * @see startCursorTracking
     */
    void stopCursorTracking();
    /**
     * @internal
     *
     * Called from X11 event handler.
     */
    void notifyCursorChanged(uint32_t serial);

    /**
     * Returns the current cursor position. This method does an update of the mouse position if
     * needed. It's save to call it multiple times.
     *
     * Implementing subclasses should prefer to use @link currentPos which is not performing a check
     * for update.
     **/
    static QPoint pos();
    /**
     * Warps the mouse cursor to new @p pos.
     **/
    static void setPos(const QPoint &pos);
    static void setPos(int x, int y);
    static xcb_cursor_t x11Cursor(Qt::CursorShape shape);

Q_SIGNALS:
    void posChanged(QPoint pos);
    void mouseChanged(const QPoint& pos, const QPoint& oldpos,
                      Qt::MouseButtons buttons, Qt::MouseButtons oldbuttons,
                      Qt::KeyboardModifiers modifiers, Qt::KeyboardModifiers oldmodifiers);
    /**
     * @brief Signal emitted when the cursor image changes.
     *
     * To enable these signals use @link startCursorTracking.
     *
     * @param serial The serial number of the new selected cursor.
     * @see startCursorTracking
     * @see stopCursorTracking
     */
    void cursorChanged(uint32_t serial);

protected:
    /**
     * Called from @link x11Cursor to actually retrieve the X11 cursor. Base implementation returns
     * a null cursor, an implementing subclass should implement this method if it can provide X11
     * mouse cursors.
     **/
    virtual xcb_cursor_t getX11Cursor(Qt::CursorShape shape);
    /**
     * Performs the actual warping of the cursor.
     **/
    virtual void doSetPos();
    /**
     * Called from @link pos() to allow syncing the internal position with the underlying
     * system's cursor position.
     **/
    virtual void doGetPos();
    /**
     * Called from @link startMousePolling when the mouse polling gets activated. Base implementation
     * does nothing, inheriting classes can overwrite to e.g. start a timer.
     **/
    virtual void doStartMousePolling();
    /**
     * Called from @link stopMousePolling when the mouse polling gets deactivated. Base implementation
     * does nothing, inheriting classes can overwrite to e.g. stop a timer.
     **/
    virtual void doStopMousePolling();
    /**
     * Called from @link startCursorTracking when cursor image tracking gets activated. Inheriting class needs
     * to overwrite to enable platform specific code for the tracking.
     */
    virtual void doStartCursorTracking();
    /**
     * Called from @link stopCursorTracking when cursor image tracking gets deactivated. Inheriting class needs
     * to overwrite to disable platform specific code for the tracking.
     */
    virtual void doStopCursorTracking();
    /**
     * Provides the actual internal cursor position to inheriting classes. If an inheriting class needs
     * access to the cursor position this method should be used instead of the static @link pos, as
     * the static method syncs with the underlying system's cursor.
     **/
    const QPoint &currentPos() const;
    /**
     * Updates the internal position to @p pos without warping the pointer as
     * @link setPos does.
     **/
    void updatePos(const QPoint &pos);
    void updatePos(int x, int y);

private:
    QPoint m_pos;
    int m_mousePollingCounter;
    int m_cursorTrackingCounter;

    KWIN_SINGLETON(Cursor)
};

class X11Cursor : public Cursor
{
    Q_OBJECT
public:
    virtual ~X11Cursor();
protected:
    virtual xcb_cursor_t getX11Cursor(Qt::CursorShape shape);
    virtual void doSetPos();
    virtual void doGetPos();
    virtual void doStartMousePolling();
    virtual void doStopMousePolling();
    virtual void doStartCursorTracking();
    virtual void doStopCursorTracking();

private slots:
    /**
    * Because of QTimer's and the impossibility to get events for all mouse
    * movements (at least I haven't figured out how) the position needs
    * to be also refetched after each return to the event loop.
    */
    void resetTimeStamp();
    void mousePolled();
private:
    X11Cursor(QObject *parent);
    xcb_cursor_t createCursor(Qt::CursorShape shape);
    QByteArray cursorName(Qt::CursorShape shape) const;
    QHash<Qt::CursorShape, xcb_cursor_t > m_cursors;
    xcb_timestamp_t m_timeStamp;
    uint16_t m_buttonMask;
    QTimer *m_resetTimeStampTimer;
    QTimer *m_mousePollingTimer;
    friend class Cursor;
};

inline const QPoint &Cursor::currentPos() const
{
    return m_pos;
}

inline void Cursor::updatePos(int x, int y)
{
    updatePos(QPoint(x, y));
}

}

#endif // KWIN_CURSOR_H
