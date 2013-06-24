/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef LAUNCHER_H
#define LAUNCHER_H

// Qt
#include <QModelIndex>
#include <QWidget>

// Plasma
#include <Plasma/Applet>

namespace Kickoff
{

/**
 * The main window class for the Kickoff launcher.  This class is responsible
 * for creating the various tabs, views and models which make up the launcher's
 * user interface.
 */
class Launcher : public QWidget
{
    Q_OBJECT

public:
    /** Construct a new Launcher with the specified parent. */
    explicit Launcher(QWidget *parent = 0);
    /** Construct a new Launcher associated with the specified Plasma::Applet. */
    explicit Launcher(Plasma::Applet *applet = 0);
    ~Launcher();

    /** Specifies whether the launcher should hide itself when an item is activated. */
    void setAutoHide(bool autoHide);
    bool autoHide() const;

    /** Specifies whether the application names in the launcher should be displayed *
        before or after the description                                             */
    void setShowAppsByName(bool showAppByName);
    bool showAppsByName() const;

    /** Specifies whether hovering switches between tabs or if a click is required to switch the tabs. */
    void setSwitchTabsOnHover(bool switchOnHover);
    bool switchTabsOnHover() const;

    /** Specifies the number of visible items used to determinate the visible height. */
    void setVisibleItemCount(int count);
    int visibleItemCount() const;

    /** Specifies the plasma applet the launcher is working on. */
    void setApplet(Plasma::Applet *applet);

    /** Specifies the direction the launcher is popping up in relative to its icon */
    void setLauncherOrigin(const Plasma::PopupPlacement placement, Plasma::Location location);
    void setAppViewIsReceivingKeyEvents(bool isReceiving);
    bool appViewIsReceivingKeyEvents() const;
    // reimplemented
    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;

    /** Reset the launcher. This is called e.g. by the Kickoff-applet before shown to be sure
    we don't display old searches and switch back to the favorite-view. */
    void reset();

    /** Specifies whether 'Recently Installed' hierarchy shall be shown in application view */
    void setShowRecentlyInstalled(bool showRecentlyInstalled);
    bool showRecentlyInstalled() const;

signals:
    void aboutToHide();
    void configNeedsSaving();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);

private Q_SLOTS:
    void focusSearchView(const QString& query);
    void showViewContextMenu(const QPoint& pos);
    void focusFavoritesView();
    void resultsAvailable();
    void updateThemedPalette();
    void fillBreadcrumbs(const QModelIndex &index);
    void breadcrumbActivated();
    void moveViewToLeft();

private:
    void addBreadcrumb(const QModelIndex &index, bool isLeaf);
    void init();

    class Private;
    Private * const d;
};

}

#endif // LAUNCHER_H
