/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

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
#include "ksldapp.h"
#include "interface.h"
#include "lockwindow.h"
#include "kscreensaversettings.h"
// Qt
#include <QtCore/QTimer>
#include <QtGui/QDesktopWidget>
#include <QtGui/QX11Info>
// KDE
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KAuthorized>
#include <KDE/KCrash>
#include <KDE/KDebug>
#include <KDE/KIdleTime>
#include <KDE/KLocalizedString>
#include <KDE/KStandardDirs>
// workspace
#include <kdisplaymanager.h>
// X11
#include <X11/Xlib.h>
// other
#include <unistd.h>

namespace ScreenLocker
{

KSldApp* KSldApp::self()
{
    if (!kapp) {
        return new KSldApp();
    }

    return qobject_cast<KSldApp*>(kapp);
}

KSldApp::KSldApp()
    : KUniqueApplication()
    , m_actionCollection(NULL)
    , m_locked(false)
    , m_lockProcess(NULL)
    , m_lockWindow(NULL)
    , m_lockedTimer(QElapsedTimer())
    , m_idleId(0)
    , m_lockGrace(0)
    , m_graceTimer(new QTimer(this))
    , m_inhibitCounter(0)
{
    initialize();
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

KSldApp::~KSldApp()
{
}

void KSldApp::cleanUp()
{
    if (m_lockProcess && m_lockProcess->state() != QProcess::NotRunning) {
        m_lockProcess->terminate();
    }
    delete m_actionCollection;
    delete m_lockProcess;
    delete m_lockWindow;
}

void KSldApp::initialize()
{
    KCrash::setFlags(KCrash::AutoRestart);

    // Global keys
    m_actionCollection = new KActionCollection(this);

    if (KAuthorized::authorize(QLatin1String("lock_screen"))) {
        kDebug() << "Configuring Lock Action";
        KAction *a = m_actionCollection->addAction(QLatin1String("Lock Session"));
        a->setText(i18n("Lock Session"));
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_L));
        connect(a, SIGNAL(triggered(bool)), this, SLOT(lock()));
    }
    m_actionCollection->readSettings();

    // idle support
    connect(KIdleTime::instance(), SIGNAL(timeoutReached(int)), SLOT(idleTimeout(int)));

    m_lockProcess = new QProcess();
    connect(m_lockProcess, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(lockProcessFinished(int,QProcess::ExitStatus)));
    m_lockedTimer.invalidate();
    m_graceTimer->setSingleShot(true);
    // create our D-Bus interface
    new Interface(this);

    configure();
}

void KSldApp::configure()
{
    KScreenSaverSettings::self()->readConfig();
    // idle support
    if (m_idleId) {
        KIdleTime::instance()->removeIdleTimeout(m_idleId);
        m_idleId = 0;
    }
    const int timeout = KScreenSaverSettings::timeout();
    if (timeout > 0) {
        // timeout stored in seconds
        m_idleId = KIdleTime::instance()->addIdleTimeout(timeout*1000);
    }
    m_lockGrace = KScreenSaverSettings::lockGrace();
    if (m_lockGrace < 0) {
        m_lockGrace = 0;
    } else if (m_lockGrace > 300000) {
        m_lockGrace = 300000; // 5 minutes, keep the value sane
    }
}

void KSldApp::lock()
{
    if (m_locked) {
        // already locked, no need to lock again
        return;
    }
    kDebug() << "lock called";
    if (!establishGrab()) {
        kError() << "Could not establish screen lock";
        return;
    }
    KDisplayManager().setLock(true);
    //KNotification::event(QLatin1String( "locked" ));

    // blank the screen
    showLockWindow();

    // start unlock screen process
    startLockProcess();
    m_locked  = true;
    m_lockedTimer.restart();
    emit unlocked();
}

KActionCollection *KSldApp::actionCollection()
{
    return m_actionCollection;
}

bool KSldApp::establishGrab()
{
    XSync(QX11Info::display(), False);

    if (!grabKeyboard()) {
        sleep(1);
        if (!grabKeyboard()) {
            return false;
        }
    }

    if (!grabMouse()) {
        sleep(1);
        if (!grabMouse()) {
            XUngrabKeyboard(QX11Info::display(), CurrentTime);
            return false;
        }
    }

    return true;
}

bool KSldApp::grabKeyboard()
{
    int rv = XGrabKeyboard( QX11Info::display(), QApplication::desktop()->winId(),
        True, GrabModeAsync, GrabModeAsync, CurrentTime );

    return (rv == GrabSuccess);
}

bool KSldApp::grabMouse()
{
#define GRABEVENTS ButtonPressMask | ButtonReleaseMask | PointerMotionMask | \
                   EnterWindowMask | LeaveWindowMask
    int rv = XGrabPointer( QX11Info::display(), QApplication::desktop()->winId(),
            True, GRABEVENTS, GrabModeAsync, GrabModeAsync, None,
            QCursor(Qt::ArrowCursor).handle(), CurrentTime );
#undef GRABEVENTS

    return (rv == GrabSuccess);
}

void KSldApp::releaseGrab()
{
    kDebug() << "Grab Released";
    XUngrabKeyboard(QX11Info::display(), CurrentTime);
    XUngrabPointer(QX11Info::display(), CurrentTime);
    hideLockWindow();
    // delete the window again, to get rid of event filter
    delete m_lockWindow;
    m_lockWindow = NULL;
    m_locked = false;
    m_lockedTimer.invalidate();
    KDisplayManager().setLock(false);
    emit unlocked();
    //KNotification *u = new KNotification( QLatin1String("unlocked"));
    //u->sendEvent();
}

static bool s_graceTimeKill = false;

void KSldApp::lockProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if ((!exitCode && exitStatus == QProcess::NormalExit) || s_graceTimeKill) {
        // unlock process finished successfully - we can remove the lock grab
        s_graceTimeKill = false;
        releaseGrab();
        return;
    }
    // failure, restart lock process
    startLockProcess();
}

void KSldApp::startLockProcess()
{
    m_lockProcess->start(KStandardDirs::findExe(QLatin1String("kscreenunlocker")));
}

void KSldApp::showLockWindow()
{
    if (!m_lockWindow) {
        m_lockWindow = new LockWindow();
    }
    m_lockWindow->showLockWindow();
    XSync(QX11Info::display(), False);
}

void KSldApp::hideLockWindow()
{
    if (!m_lockWindow) {
        return;
    }
    m_lockWindow->hideLockWindow();
}

uint KSldApp::activeTime() const
{
    if (m_lockedTimer.isValid()) {
        return m_lockedTimer.elapsed();
    }
    return 0;
}

void KSldApp::idleTimeout(int identifier)
{
    if (identifier != m_idleId) {
        // not our identifier
        return;
    }
    if (isLocked()) {
        return;
    }
    if (m_inhibitCounter) {
        // there is at least one process blocking the auto lock of screen locker
        return;
    }
    if (m_lockGrace) {
        m_graceTimer->start(m_lockGrace);
    }
    lock();
}

bool KSldApp::isGraceTime() const
{
    return m_graceTimer->isActive();
}

void KSldApp::unlock()
{
    if (!m_graceTimer->isActive()) {
        return;
    }
    s_graceTimeKill = true;
    m_lockProcess->kill();
}

void KSldApp::inhibit()
{
    ++m_inhibitCounter;
}

void KSldApp::uninhibit()
{
    --m_inhibitCounter;
}

} // namespace
#include "ksldapp.moc"
