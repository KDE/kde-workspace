/***********************************************************************************************************************
 * System Tray (KDE Plasmoid)
 * Copyright (C) 2011-2012 ROSA  <support@rosalab.ru>
 * License: GPLv3+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *          based on work made by Marco Martin <mart@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **********************************************************************************************************************/

#ifndef __SYSTEMTRAY__DIALOG_H
#define __SYSTEMTRAY__DIALOG_H


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include <QtCore/QObject>

#include <KDE/Plasma/Plasma>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declaration
class QDeclarativeItem;


namespace SystemTray
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Dialog
/**
 * This class has been introduced to hide icon of dialog in taskbar. PlasmaCore.Dialog doesn't provides such feature.
 * An implementation is based on PlasmaCore.Dialog.
 */
class Dialog: public QObject
{
    Q_OBJECT

    Q_ENUMS(Location)

    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QDeclarativeItem *item READ item WRITE setItem NOTIFY changedItem)

    /**
     * Visibility of the Dialog window. Doesn't have anything to do with the visibility of the mainItem.
     */
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY changedVisible)

    /**
     * True if the dialog window is the active one in the window manager.
     */
    Q_PROPERTY(bool active READ active NOTIFY changedActive)

    /**
     * X position of the dialog window in screen coordinates.
     */
    Q_PROPERTY(int x READ x WRITE setX NOTIFY changedX)

    /**
     * Y position of the dialog window in screen coordinates.
     */
    Q_PROPERTY(int y READ y WRITE setY NOTIFY changedY)

    /**
     * Read only width of the dialog window. It depends from the width of the mainItem
     */
    Q_PROPERTY(int width READ width NOTIFY changedWidth)

    /**
     * Read only height of the dialog window. It depends from the height of the mainItem
     */
    Q_PROPERTY(int height READ height NOTIFY changedHeight)

    /**
     * Plasma Location of the dialog window. Useful if this dialog is apopup for a panel
     */
    Q_PROPERTY(Location location READ location WRITE setLocation NOTIFY changedLocation)

public:

    // Location
    enum Location
    {
        Floating    = Plasma::Floating,
        Desktop     = Plasma::Desktop,
        FullScreen  = Plasma::FullScreen,
        TopEdge     = Plasma::TopEdge,
        BottomEdge  = Plasma::BottomEdge,
        LeftEdge    = Plasma::LeftEdge,
        RightEdge   = Plasma::RightEdge
    };

    explicit Dialog(QObject *parent = 0);
    virtual ~Dialog();


    QDeclarativeItem *item() const;
    void setItem(QDeclarativeItem *item);

    bool visible() const;
    void setVisible(bool vis);

    int x() const;
    void setX(int x);

    int y() const;
    void setY(int y);

    int width() const;
    int height() const;

    Location location() const;
    void setLocation(Location loc);

    bool active() const;

    Q_INVOKABLE void activate();

signals:
    void changedItem();
    void changedVisible();
    void changedActive();
    void changedX();
    void changedY();
    void changedHeight();
    void changedWidth();
    void changedLocation();

private slots:
    void syncItem();
    void itemSizeChanged();
    void containerSizeChanged();

private:
    class _DeclarativeItemContainer;

    bool eventFilter(QObject *watched, QEvent *event);

    struct _Private;
    _Private * const d;
};


} //namespace SystemTray

#endif // __SYSTEMTRAY__DIALOG_H