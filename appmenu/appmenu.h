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

#ifndef APPMENUMODULE_H
#define APPMENUMODULE_H

#include <kdedmodule.h>
#include "menuimporter.h"
#include "gtkicons.h"

class QDBusPendingCallWatcher;
class KDBusMenuImporter;
class AppmenuDBus;
class TopMenuBar;
class VerticalMenu;

class AppMenuModule : public KDEDModule,
                      protected QDBusContext
{
    Q_OBJECT
public:
    AppMenuModule(QObject* parent, const QList<QVariant>& list);
    virtual ~AppMenuModule();

Q_SIGNALS:
    /**
     * We do not know where is menu decoration button, so tell kwin to show menu
     */
    void showRequest(qulonglong);
    /**
     *  This signal is emitted whenever application menu becomes available
     */
    void menuAvailable(qulonglong);
    /**
     * This signal is emitted whenever menus are unavailables
     */
    void clearMenus();
    /**
     * This signal is emitted whenever popup menu/menubar is hidden
     * Useful for decorations to know if menu button should be release
     */
    void menuHidden(qulonglong);
    /**
     * This signal is emitted whenever a window register to appmenu
     */
    void WindowRegistered(qulonglong wid, const QString& service, const QDBusObjectPath&);
    /**
     * This signal is emitted whenever a window unregister from appmenu
     */
    void WindowUnregistered(qulonglong wid);

private Q_SLOTS:
    /**
     * Show menu at QPoint(x,y) for id
     * if x or y == -1, show in application window
     */
    void slotShowMenu(int x, int y, WId);
    /**
     * Send menuHidden signal over bus when menu is about to hide
     */
    void slotAboutToHide();
    /**
     * New window registered to appmenu
     * Emit WindowRegistered signal over bus
     */
    void slotWindowRegistered(WId id, const QString& service, const QDBusObjectPath& path);
    /**
     * Window unregistered from appmenu
     * Emit WindowUnregistered signal over bus
     */
    void slotWindowUnregistered(WId id);
    /**
     * Update window importer
     */
    void slotUpdateImporter(WId id);
    /**
     * Open a action in current menu
     */
    void slotActionActivationRequested(QAction* a);
    /**
     * Active window changed, update menubar for id
     * if force, menubar will be updated
     */
    void slotActiveWindowChanged(WId id, bool force = false);
    /**
     * Update menubar with current window menu
     */
    void slotShowCurrentWindowMenu();
    /**
     * Current screen changed, update menubar
     */
    void slotCurrentScreenChanged();
    /**
     * Reconfigure module
     */
    void reconfigure();

private:
    /**
     * return an importer for window id
     * if force, menu importer will no be taken from cache
     */
    KDBusMenuImporter* getImporter(WId id, bool force = false);
    /**
     * Show top menubar with menu
     */
    void showTopMenuBar(QMenu *menu);
    /**
     * Hide menubar. Delete object
     */
    void hideMenubar(TopMenuBar* menubar);
    /**
     * Return current screen
     */
    int currentScreen();

    QObject* m_parent;
    MenuImporter* m_menuImporter;
    AppmenuDBus* m_appmenuDBus;
    QHash<WId, KDBusMenuImporter*> m_importers;
    GtkIcons m_icons;
    QString m_menuStyle;
    TopMenuBar* m_menubar;
    VerticalMenu* m_menu;
    QTimer* m_menuTimer;
    QTimer* m_screenTimer;
    QAction *m_waitingAction;
    int m_currentScreen;
};

#endif
