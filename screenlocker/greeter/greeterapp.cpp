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
// workspace
#include <kephal/screens.h>
#include <kworkspace/kworkspace.h>
// KDE
#include <KDE/KCrash>
#include <KDE/KDebug>
#include <KDE/KStandardDirs>
#include <kdeclarative.h>
// Qt
#include <QtCore/QTimer>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeItem>
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/qdeclarative.h>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingCall>
// X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <fixx11h.h>

namespace ScreenLocker
{

// App
UnlockApp::UnlockApp()
    : KApplication()
    , m_testing(false)
    , m_capsLocked(false)
{
    initialize();
    QTimer::singleShot(0, this, SLOT(prepareShow()));
}

UnlockApp::~UnlockApp()
{
    qDeleteAll(m_views);
}

void UnlockApp::initialize()
{
    // disable DrKonqi as the crash dialog blocks the restart of the locker
    KCrash::setDrKonqiEnabled(false);

    KScreenSaverSettings::self()->readConfig();

    for (int i=0; i<Kephal::Screens::self()->screens().count(); ++i) {

        // create the view
        QDeclarativeView *view = new QDeclarativeView();
        view->setWindowFlags(Qt::X11BypassWindowManagerHint);
        view->setFrameStyle(QFrame::NoFrame);
        // engine stuff
        foreach(const QString &importPath, KGlobal::dirs()->findDirs("module", "imports")) {
            view->engine()->addImportPath(importPath);
        }
        KDeclarative kdeclarative;
        kdeclarative.setDeclarativeEngine(view->engine());
        kdeclarative.initialize();
        kdeclarative.setupBindings();

        view->setSource(QUrl::fromLocalFile(KStandardDirs::locate("data", KScreenSaverSettings::greeterQML())));
        view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

        connect(view->rootObject(), SIGNAL(unlockRequested()), SLOT(quit()));
        connect(view->rootObject(), SIGNAL(suspendToRam()), SLOT(suspendToRam()));
        connect(view->rootObject(), SIGNAL(suspendToDisk()), SLOT(suspendToDisk()));
        connect(view->rootObject(), SIGNAL(shutdown()), SLOT(shutdown()));
        m_views << view;
    }
    installEventFilter(this);
}

void UnlockApp::prepareShow()
{
    // mark as our window
    Atom tag = XInternAtom(QX11Info::display(), "_KDE_SCREEN_LOCKER", False);

    for (int i=0; i<Kephal::Screens::self()->screens().count(); ++i) {
        if (i == m_views.size()) {
            kError() << "Views and screens not in sync";
            return;
        }
        QDeclarativeView *view = m_views.at(i);

        XChangeProperty(QX11Info::display(), view->winId(), tag, tag, 32, PropModeReplace, 0, 0);
        view->setGeometry(Kephal::Screens::self()->screen(i)->geom());
        view->show();
    }
    capsLocked();
}

void UnlockApp::suspendToRam()
{
    QDBusInterface iface("org.kde.Solid.PowerManagement",
                         "/org/kde/Solid/PowerManagement",
                         "org.kde.Solid.PowerManagement");
    iface.asyncCall("suspendToRam");

}

void UnlockApp::suspendToDisk()
{
    QDBusInterface iface("org.kde.Solid.PowerManagement",
                         "/org/kde/Solid/PowerManagement",
                         "org.kde.Solid.PowerManagement");
    iface.asyncCall("suspendToDisk");
}

void UnlockApp::shutdown()
{
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
    Q_UNUSED(obj)
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        capsLocked();
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
