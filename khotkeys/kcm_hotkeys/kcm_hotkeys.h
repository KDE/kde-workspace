/*
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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
#ifndef KCMHOTKEYS_H
#define KCMHOTKEYS_H

#include <KCModule>

#include <QModelIndex>

class KCMHotkeysPrivate;
class QWidget;

namespace KHotKeys {
    class ActionDataBase;
}

/**
 * @brief KCMHotkeys KDE KCM Hotkeys Configuration Module
 * @author Michael Jansen <kde@michael-jansen.biz>
 * @date 2008-03-07
 */
class KCMHotkeys : public KCModule
    {
    Q_OBJECT

public:

    /**
     * Create the module.
     *
     * @param parent Parent widget
     */
    KCMHotkeys( QWidget *parent, const QVariantList &arg );

    /**
     * Destroy the module
     */
    virtual ~KCMHotkeys();

    /**
     * Set all settings back to defaults.
     */
    void defaults();

    /**
     * Load all settings. 
     */
    void load();

    /**
     * Save the settings
     */
    void save();



public Q_SLOTS:

    void slotChanged();

    void slotReset();

    /**
     * Call when the current item has changed
     */
    void currentChanged( const QModelIndex &current, const QModelIndex &previous );

    /**
     * Show global settings dialog
     */
    void showGlobalSettings();

private:

    KCMHotkeysPrivate *d;
};

#endif /* #ifndef KCMHOTKEYS_HPP */
