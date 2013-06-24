/*
  This file is part of the KDE project.

  Copyright (c) 2011 Lionel Chauvin <megabigbug@yahoo.fr>
  Copyright (c) 2011,2012 Cédric Bellegarde <gnumdk@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#include "appmenu.h"
#include "kdbusimporter.h"
#include "menuimporteradaptor.h"
#include "appmenuadaptor.h"
#include "appmenu_dbus.h"
#include "topmenubar.h"
#include "verticalmenu.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusPendingCallWatcher>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>

#include <KDebug>
#include <KWindowSystem>
#include <KWindowInfo>
#include <KConfig>
#include <KConfigGroup>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <netwm.h>

K_PLUGIN_FACTORY(AppMenuFactory,
                 registerPlugin<AppMenuModule>();
    )
K_EXPORT_PLUGIN(AppMenuFactory("appmenu"))

AppMenuModule::AppMenuModule(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent),
    m_parent(parent),
    m_menuImporter(0),
    m_appmenuDBus(new AppmenuDBus(parent)),
    m_menubar(0),
    m_menu(0),
    m_screenTimer(new QTimer(this)),
    m_waitingAction(0),
    m_currentScreen(-1)
{
    reconfigure();

    m_appmenuDBus->connectToBus();

    m_currentScreen = currentScreen();

    connect(m_appmenuDBus, SIGNAL(appShowMenu(int, int, WId)), SLOT(slotShowMenu(int, int, WId)));
    connect(m_appmenuDBus, SIGNAL(moduleReconfigure()), SLOT(reconfigure()));

    // transfer our signals to dbus
    connect(this, SIGNAL(showRequest(qulonglong)), m_appmenuDBus, SIGNAL(showRequest(qulonglong)));
    connect(this, SIGNAL(menuAvailable(qulonglong)), m_appmenuDBus, SIGNAL(menuAvailable(qulonglong)));
    connect(this, SIGNAL(clearMenus()), m_appmenuDBus, SIGNAL(clearMenus()));
    connect(this, SIGNAL(menuHidden(qulonglong)), m_appmenuDBus, SIGNAL(menuHidden(qulonglong)));
    connect(this, SIGNAL(WindowRegistered(qulonglong, const QString&, const QDBusObjectPath&)),
            m_appmenuDBus, SIGNAL(WindowRegistered(qulonglong, const QString&, const QDBusObjectPath&)));
    connect(this, SIGNAL(WindowUnregistered(qulonglong)), m_appmenuDBus, SIGNAL(WindowUnregistered(qulonglong)));
}

AppMenuModule::~AppMenuModule()
{
    emit clearMenus();
    hideMenubar();
    if (m_menubar) {
        delete m_menubar;
    }
    delete m_menuImporter;
    delete m_appmenuDBus;
}

void AppMenuModule::slotShowMenu(int x, int y, WId id)
{
    static KDBusMenuImporter *importer = 0;

    if (!m_menuImporter) {
        return;
    }

    // If menu visible, hide it
    if (m_menu && m_menu->isVisible()) {
        m_menu->hide();
        return;
    }

    //dbus call by user (for khotkey shortcut)
    if (x == -1 || y == -1) {
        // We do not know kwin button position, so tell kwin to show menu
        emit showRequest(KWindowSystem::self()->activeWindow());
        return;
    }

    importer = getImporter(id);

    if (!importer) {
        return;
    }

    QMenu *menu = importer->menu();

    // Window do not have menu
    if (!menu) {
        return;
    }

    m_menu = new VerticalMenu();
    m_menu->setParentWid(id);
    // Populate menu
    foreach (QAction *action, menu->actions()) {
        m_menu->addAction(action);
    }
    m_menu->popup(QPoint(x, y));
    // Activate waiting action if exist
    if (m_waitingAction) {
        m_menu->setActiveAction(m_waitingAction);
        m_waitingAction = 0;
    }
    connect(m_menu, SIGNAL(aboutToHide()), this, SLOT(slotAboutToHide()));
}

void AppMenuModule::slotAboutToHide()
{
    if (m_menu) {
        emit menuHidden(m_menu->parentWid());
        m_menu->deleteLater();
        m_menu = 0;
    }
}

// New window registered
void AppMenuModule::slotWindowRegistered(WId id, const QString& service, const QDBusObjectPath& path)
{
    KDBusMenuImporter* importer = m_importers.take(id);
     if (importer) {
        delete importer;
    }

    // Application already active so check if we need create menubar
    if ( m_menuStyle == "TopMenuBar" && id == KWindowSystem::self()->activeWindow()) {
        slotActiveWindowChanged(id);
    } else if (m_menuStyle == "ButtonVertical") {
        // Tell Kwin menu is available
        emit menuAvailable(id);
        getImporter(id);
    }

    // Send a signal on bus for others dbus interface registrars
    emit WindowRegistered(id, service, path);
}

// Window unregistered
void AppMenuModule::slotWindowUnregistered(WId id)
{
    KDBusMenuImporter* importer = m_importers.take(id);

    // Send a signal on bus for others dbus interface registrars
    emit WindowUnregistered(id);

    if (importer) {
        delete importer;
    }

    if (m_menubar && m_menubar->parentWid() == id) {
        hideMenubar();
    }
}

// Keyboard activation requested, transmit it to menu
void AppMenuModule::slotActionActivationRequested(QAction* a)
{
    // If we have a topmenubar, activate action
    if (m_menubar) {
        m_menubar->setActiveAction(a);
        m_menubar->show();
    } else { // else send request to kwin or others dbus interface registrars
        m_waitingAction = a;
        emit showRequest(KWindowSystem::self()->activeWindow());
    }
}

// Current window change, update menubar
// See comments in slotWindowRegistered() for why we get importers here
void AppMenuModule::slotActiveWindowChanged(WId id)
{
    KWindowInfo info = KWindowSystem::windowInfo(id, NET::WMWindowType);
    unsigned long mask = NET::AllTypesMask;

    m_currentScreen = currentScreen();

    if (id == 0) {// Ignore root window
        return;
    } else if (info.windowType(mask) & NET::Dock) { // Hide immediatly menubar for docks (krunner)
        hideMenubar();
        return;
    }

    if (!m_menuImporter->serviceExist(id)) { // No menu exist, check for another menu for application
        WId recursiveId = m_menuImporter->recursiveMenuId(id);
        if (recursiveId) {
           id = recursiveId;
        }
    }

    KDBusMenuImporter *importer = getImporter(id);
    if (!importer) {
        hideMenubar();
        return;
    }

    QMenu *menu = importer->menu();

    if(menu) {
        showMenuBar(menu);
        m_menubar->setParentWid(id);
    } else {
        hideMenubar();
    }
}

void AppMenuModule::slotShowCurrentWindowMenu()
{
    slotActiveWindowChanged(KWindowSystem::self()->activeWindow());
}

void AppMenuModule::slotCurrentScreenChanged()
{
    if (m_currentScreen != currentScreen()) {
        if (m_menubar) {
            m_menubar->setParentWid(0);
        }
        slotActiveWindowChanged(KWindowSystem::self()->activeWindow());
    }
}

void AppMenuModule::slotBarNeedResize()
{
    if (m_menubar) {
        m_menubar->updateSize();
        m_menubar->move(centeredMenubarPos());
    }
}

// reload settings
void AppMenuModule::reconfigure()
{
    KConfig config( "kdeglobals", KConfig::FullConfig );
    KConfigGroup configGroup = config.group("Appmenu Style");
    m_menuStyle = configGroup.readEntry("Style", "InApplication");

    m_waitingAction = 0;

    // hide menubar if exist
    hideMenubar();
    if (m_menubar) {
        delete m_menubar;
        m_menubar = 0;
    }

    slotAboutToHide(); // hide vertical menu if exist

    // Disconnect all options specifics signals
    disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(slotActiveWindowChanged(WId)));
    disconnect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(slotShowCurrentWindowMenu()));
    disconnect(m_screenTimer, SIGNAL(timeout()), this, SLOT(slotCurrentScreenChanged()));

    m_screenTimer->stop();

    // Tell kwin to clean its titlebar
    emit clearMenus();

    if (m_menuStyle == "InApplication") {
        if (m_menuImporter) {
            delete m_menuImporter;
            m_menuImporter = 0;
        }
        return;
    }

    // Setup a menu importer if needed
    if (!m_menuImporter) {
        m_menuImporter = new MenuImporter(m_parent);
        connect(m_menuImporter, SIGNAL(WindowRegistered(WId, const QString&, const QDBusObjectPath&)),
            SLOT(slotWindowRegistered(WId, const QString&, const QDBusObjectPath&)));
        connect(m_menuImporter, SIGNAL(WindowUnregistered(WId)),
            SLOT(slotWindowUnregistered(WId)));
        m_menuImporter->connectToBus();
    }

    if( m_menuStyle == "ButtonVertical" ) {
        foreach(WId id, m_menuImporter->ids()) {
            emit menuAvailable(id);
        }
    }

    // Setup top menubar if needed
    if (m_menuStyle == "TopMenuBar") {
        m_menubar = new TopMenuBar();
        connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(slotActiveWindowChanged(WId)));
        connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(slotShowCurrentWindowMenu()));
        connect(m_screenTimer, SIGNAL(timeout()), this, SLOT(slotCurrentScreenChanged()));
        connect(m_menubar, SIGNAL(needResize()), SLOT(slotBarNeedResize()));
        m_screenTimer->start(1000);
        slotShowCurrentWindowMenu();
    }
}

KDBusMenuImporter* AppMenuModule::getImporter(WId id)
{
    KDBusMenuImporter* importer = 0;
    if (m_importers.contains(id)) { // importer already exist
        importer = m_importers.value(id);
    } else if (m_menuImporter->serviceExist(id)) { // get importer
        importer = new KDBusMenuImporter(id, m_menuImporter->serviceForWindow(id), &m_icons,
                                             m_menuImporter->pathForWindow(id), this);
        if (importer) {
            QMetaObject::invokeMethod(importer, "updateMenu", Qt::DirectConnection);
            connect(importer, SIGNAL(actionActivationRequested(QAction*)),
                    SLOT(slotActionActivationRequested(QAction*)));
            m_importers.insert(id, importer);
        }
    }
    return importer;
}

void AppMenuModule::showMenuBar(QMenu *menu)
{
    if (!menu) {
        return;
    }

    m_menubar->setMenu(menu);
    if (menu->actions().length()) {
        m_menubar->enableMouseTracking();
    }
}

void AppMenuModule::hideMenubar()
{
    if (!m_menubar) {
        return;
    }

    m_menubar->enableMouseTracking(false);
    if (m_menubar->isVisible()) {
        m_menubar->hide();
    }
}

int AppMenuModule::currentScreen()
{
    KWindowInfo info = KWindowSystem::windowInfo(KWindowSystem::self()->activeWindow(),
                                                 NET::WMGeometry);
    int x = info.geometry().x();
    int y = info.geometry().y();

    QDesktopWidget *desktop = QApplication::desktop();
    return desktop->screenNumber(QPoint(x,y));
}


QPoint AppMenuModule::centeredMenubarPos()
{
    QDesktopWidget *desktop = QApplication::desktop();
    m_currentScreen = currentScreen();
    QRect screen = desktop->availableGeometry(m_currentScreen);
    int x = screen.center().x() - m_menubar->sizeHint().width()/2;
    return QPoint(x, screen.topLeft().y());
}


#include "appmenu.moc"
