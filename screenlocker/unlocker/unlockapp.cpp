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
#include "unlockapp.h"
#include "unlocker.h"
// Qt
#include <QtCore/QTimer>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeItem>
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/qdeclarative.h>
#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QLineEdit>
// KDE
#include <KDE/KCrash>
#include <KDE/KDebug>
#include <KDE/KStandardDirs>
#include <KDE/KUser>
#include <kdeclarative.h>
// workspace
#include <kephal/screens.h>
// X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <fixx11h.h>

namespace ScreenLocker
{

// App
UnlockApp::UnlockApp()
    : KApplication()
    , m_view(NULL)
{
    initialize();
    QTimer::singleShot(0, this, SLOT(prepareShow()));
}

UnlockApp::~UnlockApp()
{
    delete m_view;
}

void UnlockApp::initialize()
{
    // disable DrKonqi as the crash dialog blocks the restart of the locker
    KCrash::setDrKonqiEnabled(false);

    // create the view
    m_view = new QDeclarativeView();
    m_view->setWindowFlags(Qt::X11BypassWindowManagerHint);
    m_view->setFrameStyle(QFrame::NoFrame);
    // engine stuff
    foreach(const QString &importPath, KGlobal::dirs()->findDirs("module", "imports")) {
        m_view->engine()->addImportPath(importPath);
    }
    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(m_view->engine());
    kdeclarative.initialize();
    kdeclarative.setupBindings();
    qmlRegisterType<GreeterItem>("org.kde.screenlocker", 1, 0, "GreeterItem");
    qmlRegisterType<KeyboardItem>("org.kde.screenlocker", 1, 0, "KeyboardItem");

#if 0
    m_view->rootContext()->setContextProperty("sessionModel", m_unlocker->sessionModel());
#endif
    m_view->setSource(QUrl::fromLocalFile(KStandardDirs::locate("data", "ksld/lockscreen.qml")));
    m_view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    connect(m_view->rootObject(), SIGNAL(unlockRequested()), SLOT(quit()));
#if 0
    connect(m_view->rootObject(), SIGNAL(startNewSession()), m_unlocker, SLOT(startNewSession()));
    connect(m_view->rootObject(), SIGNAL(activateSession(int)), m_unlocker, SLOT(activateSession(int)));
#endif
    KUser user;
    m_view->rootObject()->setProperty("userName", user.property(KUser::FullName).toString());
#if 0
    m_view->rootObject()->setProperty("switchUserSupported", m_unlocker->isSwitchUserSupported());
    m_view->rootObject()->setProperty("startNewSessionSupported", m_unlocker->isStartNewSessionSupported());
#endif

    // TODO: connect Kephal screens
}

void UnlockApp::prepareShow()
{
    // mark as our window
    Atom tag = XInternAtom(QX11Info::display(), "_KDE_SCREEN_LOCKER", False);
    XChangeProperty(QX11Info::display(), m_view->winId(), tag, tag, 32, PropModeReplace, 0, 0);

    m_view->setGeometry(Kephal::Screens::self()->primaryScreen()->geom());
    m_view->show();

    // HACK: set focus on password field
    if (GreeterItem *unlocker = m_view->rootObject()->findChild<GreeterItem*>("greeter")) {
        if (QLineEdit *lineEdit = unlocker->proxy()->widget()->findChild<QLineEdit *>()) {
            lineEdit->setFocus(Qt::OtherFocusReason);
        }
    }
}

} // namespace

#include "unlockapp.moc"
