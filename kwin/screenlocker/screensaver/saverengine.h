//===========================================================================
//
// This file is part of the KDE project
//
// Copyright 1999 Martin R. Jones <mjones@kde.org>
//

#ifndef SAVERENGINE_H
#define SAVERENGINE_H

#include <QWidget>
#include <QVector>
#include <QDBusContext>
#include <QDBusMessage>

class QDBusServiceWatcher;
class KProcess;

namespace KWin {
namespace ScreenLocker {
    class ScreenLocker;
}
}

#include "xautolock.h"
#include "xautolock_c.h"

class ScreenSaverRequest
{
public:
//     QString appname;
//     QString reasongiven;
    QString dbusid;
    uint cookie;
    enum { Inhibit,Throttle } type;
};

//===========================================================================
/**
 * Screen saver engine.  Handles screensaver window, starting screensaver
 * hacks, and password entry.
 */
class SaverEngine : public QWidget, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.ScreenSaver")

public:
    SaverEngine(KWin::ScreenLocker::ScreenLocker *locker);
    ~SaverEngine();

    bool doLock();

public Q_SLOTS:
    /**
     * Lock the screen now even if the screensaver does not lock by default.
     */
    void Lock();

    /**
     * Save the screen now. If the user has locking enabled, the screen is locked also.
     */
    bool save();

    /**
     * Start the screensaver in plasma-setup mode.
     * if plasma is not enabled, this just locks the screen.
     */
    bool setupPlasma();

    /**
     * Quit the screensaver if it is running
     */
    bool quit();

    /**
     * Simulate user activity
     */
    void SimulateUserActivity();

    /**
     * Return true if the screensaver is enabled
     */
    bool isEnabled();

    /**
     * Enable/disable the screensaver
     * @return true if the action succeeded
     */
    bool enable( bool e, bool force = false );

    /**
     * Return true if the screen is currently blanked
     */
    bool isBlanked();

    /**
     * Read and apply configuration.
     */
    void configure();

    /**
     * Called by krunner_lock when locking is in effect.
     */
    void saverLockReady();

    /**
     * Request a change in the state of the screensaver.
     * Set to TRUE to request that the screensaver activate.
     * Active means that the screensaver has blanked the
     * screen and may run a graphical theme.  This does
     * not necessary mean that the screen is locked.
     */
    bool SetActive( bool state );

    /// Returns the value of the current state of activity (See setActive)
    bool GetActive();

    /**
     * Returns the number of seconds that the screensaver has
     * been active.  Returns zero if the screensaver is not active.
     */
    uint GetActiveTime();

    /**
     * Returns the number of seconds that the session has
     * been idle.  Returns zero if the session is not idle.
     */
    uint GetSessionIdleTime();

    /**
     * Request that saving the screen due to system idleness
     * be blocked until UnInhibit is called or the
     * calling process exits.
     * The cookie is a random number used to identify the request
     */
    uint Inhibit(const QString &application_name, const QString &reason_for_inhibit);
    /// Cancel a previous call to Inhibit() identified by the cookie.
    void UnInhibit(uint cookie);

    /**
     * Request that running themes while the screensaver is active
     * be blocked until UnThrottle is called or the
     * calling process exits.
     * The cookie is a random number used to identify the request
     */
    uint Throttle(const QString &application_name, const QString &reason_for_inhibit);
    /// Cancel a previous call to Throttle() identified by the cookie.
    void UnThrottle(uint cookie);

Q_SIGNALS:
    // DBus signals
    void ActiveChanged(bool state);

protected Q_SLOTS:
    void idleTimeout();
    void lockProcessExited();
    void serviceUnregistered(const QString&);

protected:
    enum LockType { DontLock, DefaultLock, ForceLock, PlasmaSetup };
    bool startLockProcess( LockType lock_type );
    void stopLockProcess();
    bool handleKeyPress(XKeyEvent *xke);
    void processLockTransactions();
    xautolock_corner_t applyManualSettings(int);

private:
    enum State { Waiting, Preparing, Saving };

    State       mState;
    XAutoLock   *mXAutoLock;
    KProcess    *mLockProcess;

    // the original X screensaver parameters
    int         mXTimeout;
    int         mXInterval;
    int         mXBlanking;
    int         mXExposures;

    time_t      m_actived_time;
    QList<ScreenSaverRequest> m_requests;
    QDBusServiceWatcher *m_serviceWatcher;
    uint        m_next_cookie;
    
    int        m_nr_throttled;
    int        m_nr_inhibited;
 
    QList<QDBusMessage> mLockTransactions;

    KWin::ScreenLocker::ScreenLocker *m_screenLocker;
};

#endif

