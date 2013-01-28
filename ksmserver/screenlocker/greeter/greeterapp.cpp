/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2004 Chris Howells <howells@kde.org>
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
#include "greeterapp.h"
#include "kscreensaversettings.h"
#include "greeter.h"
#include "sessions.h"
#include "screensaverwindow.h"

// workspace
#include <kworkspace/kworkspace.h>
// KDE
#include <KDE/KAuthorized>
#include <KDE/KCrash>
#include <KDE/KDebug>
#include <KDE/KStandardDirs>
#include <KDE/KUser>
#include <Solid/PowerManagement>
#include <kdeclarative.h>
//Plasma
#include <Plasma/Package>
#include <Plasma/PackageMetadata>
// Qt
#include <QtCore/QTimer>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeItem>
#include <QtDeclarative/QDeclarativeProperty>
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/qdeclarative.h>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingCall>
#include <QtGui/QKeyEvent>
#include <QDesktopWidget>
// X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <fixx11h.h>

namespace ScreenLocker
{

static const char *DEFAULT_MAIN_PACKAGE = "org.kde.passworddialog";

// App
UnlockApp::UnlockApp()
    : KApplication()
    , m_resetRequestIgnoreTimer(new QTimer(this))
    , m_testing(false)
    , m_capsLocked(false)
    , m_ignoreRequests(false)
    , m_showScreenSaver(false)
{
    initialize();
    connect (desktop(), SIGNAL(resized(int)), SLOT(desktopResized()));
    connect (desktop(), SIGNAL(screenCountChanged(int)), SLOT(desktopResized()));
    QMetaObject::invokeMethod(this, "desktopResized", Qt::QueuedConnection);
}

UnlockApp::~UnlockApp()
{
    qDeleteAll(m_views);
    qDeleteAll(m_screensaverWindows);
}

void UnlockApp::initialize()
{
    const char *uri = "org.kde.kscreenlocker";
    qmlRegisterType<GreeterItem>(uri, 1, 0, "GreeterItem");
    qmlRegisterType<KeyboardItem>(uri, 1, 0, "KeyboardItem");
    qmlRegisterType<SessionSwitching>(uri, 1, 0, "Sessions");
    qmlRegisterType<QAbstractItemModel>();

    // set up the request ignore timeout, so that multiple requests to sleep/suspend/shutdown
    // are not processed in quick (and confusing) succession)
    m_resetRequestIgnoreTimer->setSingleShot(true);
    m_resetRequestIgnoreTimer->setInterval(2000);
    connect(m_resetRequestIgnoreTimer, SIGNAL(timeout()), this, SLOT(resetRequestIgnore()));

    // disable DrKonqi as the crash dialog blocks the restart of the locker
    KCrash::setDrKonqiEnabled(false);

    KScreenSaverSettings::self()->readConfig();
    m_showScreenSaver = KScreenSaverSettings::legacySaverEnabled();

    m_structure = Plasma::PackageStructure::load("Plasma/Generic");
    m_package = new Plasma::Package(KStandardDirs::locate("data", "ksmserver/screenlocker/"), KScreenSaverSettings::greeterQML(), m_structure);
    m_mainQmlPath = m_package->filePath("mainscript");
    if (m_mainQmlPath.isEmpty()) {
        delete m_package;
        m_package = new Plasma::Package(KStandardDirs::locate("data", "ksmserver/screenlocker/"), DEFAULT_MAIN_PACKAGE, m_structure);
        m_mainQmlPath = m_package->filePath("mainscript");
    }

    installEventFilter(this);
}

void UnlockApp::viewStatusChanged(const QDeclarativeView::Status &status)
{
    // on error, if we did not load the default qml, try to do so now.
    if (status == QDeclarativeView::Error &&
        m_package->metadata().pluginName() != DEFAULT_MAIN_PACKAGE) {
        if (QDeclarativeView *view = qobject_cast<QDeclarativeView *>(sender())) {
            m_package = new Plasma::Package(KStandardDirs::locate("data", "ksmserver/screenlocker/"), DEFAULT_MAIN_PACKAGE, m_structure);
            m_mainQmlPath = m_package->filePath("mainscript");
            view->setSource(QUrl::fromLocalFile(m_mainQmlPath));
        }
    }
}

void UnlockApp::desktopResized()
{
    const int nScreens = QApplication::desktop()->screenCount();
    // remove useless views and savers
    while (m_views.count() > nScreens) {
        m_views.takeLast()->deleteLater();
    }
    while (m_screensaverWindows.count() > nScreens) {
        m_screensaverWindows.takeLast()->deleteLater();
    }

    Q_ASSERT((!m_showScreenSaver || m_views.count() == m_screensaverWindows.count()));

    // extend views and savers to current demand
    const bool canLogout = KAuthorized::authorizeKAction("logout") && KAuthorized::authorize("logout");
    const QSet<Solid::PowerManagement::SleepState> spdMethods = Solid::PowerManagement::supportedSleepStates();
    for (int i = m_views.count(); i < nScreens; ++i) {
        // create the view
        QDeclarativeView *view = new QDeclarativeView();
        connect(view, SIGNAL(statusChanged(QDeclarativeView::Status)),
                this, SLOT(viewStatusChanged(QDeclarativeView::Status)));
        view->setWindowFlags(Qt::X11BypassWindowManagerHint);
        view->setFrameStyle(QFrame::NoFrame);
        view->installEventFilter(this);

        // engine stuff
        foreach (const QString &importPath, KGlobal::dirs()->findDirs("module", "imports")) {
            view->engine()->addImportPath(importPath);
        }

        KDeclarative kdeclarative;
        kdeclarative.setDeclarativeEngine(view->engine());
        kdeclarative.initialize();
        kdeclarative.setupBindings();
        QDeclarativeContext *context = view->engine()->rootContext();
        const KUser user;
        const QString fullName = user.property(KUser::FullName).toString();
        context->setContextProperty("kscreenlocker_userName", fullName.isEmpty() ? user.loginName() : fullName);

        view->setSource(QUrl::fromLocalFile(m_mainQmlPath));
        view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

        connect(view->rootObject(), SIGNAL(unlockRequested()), SLOT(quit()));

        QDeclarativeProperty sleepProperty(view->rootObject(), "suspendToRamSupported");
        sleepProperty.write(spdMethods.contains(Solid::PowerManagement::SuspendState));
        if (spdMethods.contains(Solid::PowerManagement::SuspendState) &&
            view->rootObject()->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("suspendToRam()")) != -1) {
            connect(view->rootObject(), SIGNAL(suspendToRam()), SLOT(suspendToRam()));
        }

        QDeclarativeProperty hibernateProperty(view->rootObject(), "suspendToDiskSupported");
        hibernateProperty.write(spdMethods.contains(Solid::PowerManagement::SuspendState));
        if (spdMethods.contains(Solid::PowerManagement::SuspendState) &&
            view->rootObject()->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("suspendToDisk()")) != -1) {
            connect(view->rootObject(), SIGNAL(suspendToDisk()), SLOT(suspendToDisk()));
        }

        QDeclarativeProperty shutdownProperty(view->rootObject(), "shutdownSupported");
        shutdownProperty.write(canLogout);
        if (canLogout &&
            view->rootObject()->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("shutdown()")) != -1) {
            connect(view->rootObject(), SIGNAL(shutdown()), SLOT(shutdown()));
        }

        m_views << view;

        if (m_showScreenSaver) {
            ScreenSaverWindow *screensaverWindow = new ScreenSaverWindow;
            screensaverWindow->setWindowFlags(Qt::X11BypassWindowManagerHint);
            m_screensaverWindows << screensaverWindow;
        }
    }

    // update geometry of all views and savers
    for (int i = 0; i < nScreens; ++i) {
        QDeclarativeView *view = m_views.at(i);

        view->setGeometry(QApplication::desktop()->screenGeometry(i));
        view->show();
        view->activateWindow();

        if (m_showScreenSaver) {
            ScreenSaverWindow *screensaverWindow = m_screensaverWindows.at(i);
            screensaverWindow->setGeometry(view->geometry());

            QPixmap backgroundPix(screensaverWindow->size());
            QPainter p(&backgroundPix);
            view->render(&p);
            p.end();
            screensaverWindow->setBackground(backgroundPix);
            screensaverWindow->show();
            screensaverWindow->activateWindow();
            connect(screensaverWindow, SIGNAL(hidden()), this, SLOT(getFocus()));
        }
    }
    // random state update, actually rather required on init only
    QMetaObject::invokeMethod(this, "getFocus", Qt::QueuedConnection);
    capsLocked();
}

void UnlockApp::getFocus()
{
    if (!m_views.isEmpty()) {
        m_views.first()->activateWindow();
    }
}

void UnlockApp::resetRequestIgnore()
{
    m_ignoreRequests = false;
}

void UnlockApp::suspendToRam()
{
    if (m_ignoreRequests) {
        return;
    }

    m_ignoreRequests = true;
    m_resetRequestIgnoreTimer->start();

    QDBusInterface iface("org.kde.Solid.PowerManagement",
            "/org/kde/Solid/PowerManagement",
            "org.kde.Solid.PowerManagement");
    iface.asyncCall("suspendToRam");

}

void UnlockApp::suspendToDisk()
{
    if (m_ignoreRequests) {
        return;
    }

    m_ignoreRequests = true;
    m_resetRequestIgnoreTimer->start();

    QDBusInterface iface("org.kde.Solid.PowerManagement",
            "/org/kde/Solid/PowerManagement",
            "org.kde.Solid.PowerManagement");
    iface.asyncCall("suspendToDisk");
}

void UnlockApp::shutdown()
{
    if (m_ignoreRequests) {
        return;
    }

    m_ignoreRequests = true;
    m_resetRequestIgnoreTimer->start();

    const KWorkSpace::ShutdownConfirm confirm = KWorkSpace::ShutdownConfirmNo;
    const KWorkSpace::ShutdownType type = KWorkSpace::ShutdownTypeHalt;

    KWorkSpace::requestShutDown(confirm, type);
}

void UnlockApp::setTesting(bool enable)
{
    m_testing  = enable;
    if (m_views.isEmpty()) {
        return;
    }
    if (enable) {
        // remove bypass window manager hint
        foreach (QDeclarativeView * view, m_views) {
            view->setWindowFlags(view->windowFlags() & ~Qt::X11BypassWindowManagerHint);
        }
    } else {
        foreach (QDeclarativeView * view, m_views) {
            view->setWindowFlags(view->windowFlags() | Qt::X11BypassWindowManagerHint);
        }
    }
}

bool UnlockApp::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != this && event->type() == QEvent::Show) {
        QDeclarativeView *view(0);
        foreach (QDeclarativeView *v, m_views) {
            if (v == obj) {
                view = v;
                break;
            }
        }
        if (view && view->testAttribute(Qt::WA_WState_Created) && view->internalWinId()) {
            // showing greeter view window, set property
            static Atom tag = XInternAtom(QX11Info::display(), "_KDE_SCREEN_LOCKER", False);
            XChangeProperty(QX11Info::display(), view->winId(), tag, tag, 32, PropModeReplace, 0, 0);
        }
        // no further processing
        return false;
    }

    static bool ignoreNextEscape = false;
    if (event->type() == QEvent::KeyPress) { // react if saver is visible
        bool saverVisible = false;
        foreach (ScreenSaverWindow *screensaverWindow, m_screensaverWindows) {
            if (screensaverWindow->isVisible()) {
                saverVisible = true;
                break;
            }
        }
        if (!saverVisible) {
            return false; // we don't care
        }
        ignoreNextEscape = bool(static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape);
        capsLocked();
        foreach (ScreenSaverWindow *screensaverWindow, m_screensaverWindows) {
            screensaverWindow->hide();
        }
        getFocus();
        return true; // do not pass the key
    } else if (event->type() == QEvent::KeyRelease) { // conditionally reshow the saver
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_CapsLock) {
            capsLocked();
            return false;
        }
        if (ke->key() != Qt::Key_Escape) {
            return false; // irrelevant
        }
        if (ignoreNextEscape) {
            ignoreNextEscape = false;
            return true; // it's Qt::Key_Escape;
        }
        bool saverVisible = true;
        foreach (ScreenSaverWindow *screensaverWindow, m_screensaverWindows) {
            if (!screensaverWindow->isVisible()) {
                saverVisible = false;
                break;
            }
        }
        if (saverVisible) {
            return false; // we don't care
        }
        foreach (ScreenSaverWindow *screensaverWindow, m_screensaverWindows) {
            screensaverWindow->show();
        }
        return true; // don't pass
    } else if (event->type() == QEvent::GraphicsSceneMousePress) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);

        foreach (QDeclarativeView *view, m_views) {
            if (view->geometry().contains(me->screenPos())) {
                view->activateWindow();
                view->grabKeyboard();
                break;
            }
        }
    }

    return false;
}

void UnlockApp::capsLocked()
{
    unsigned int lmask;
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    XQueryPointer(QX11Info::display(), DefaultRootWindow( QX11Info::display() ), &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6, &lmask);
    const bool before = m_capsLocked;
    m_capsLocked = lmask & LockMask;
    if (before != m_capsLocked) {
        foreach (QDeclarativeView *view, m_views) {
            view->rootObject()->setProperty("capsLockOn", m_capsLocked);
        }
    }
}

} // namespace

#include "greeterapp.moc"
