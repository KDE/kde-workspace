/********************************************************************
 KWin - the KDE window manager
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
#ifndef KWIN_SCREENLOCKER_SCREENLOCKER_H
#define KWIN_SCREENLOCKER_SCREENLOCKER_H

#include <QtCore/QObject>

// forward declarations
class KActionCollection;
class SaverEngine;

namespace KWin
{
namespace ScreenLocker
{

/**
 * @short Class handling screen locking.
 *
 * The classic Screen Saving is handled by the @link SaverEngine.
 * This class only takes care of locking the screen. That is ensure that
 * nothing is displayed on the screen.
 *
 * The D-Bus interface to screen locking is provided by the @link SaverEngine
 * for backwards-compatibility. This class cannot implement the org.freedesktop.ScreenSaver
 * interface as it does not provide the screen saving capabilities.
 **/
class ScreenLocker : public QObject
{
    Q_OBJECT
public:
    ScreenLocker(QObject *parent = NULL);
    virtual ~ScreenLocker();

    void initShortcuts(KActionCollection *keys);

    /**
     * Unlocks the screen. Inside KWin we trust each other and assume
     * that the method will only be called when the screen got unlocked
     * by a trusted authority. E.g. a KWin Effect or the SaverEngine.
     **/
    void unlock();
    /**
     * @returns Whether the screen is locked.
     **/
    bool isLocked() const {
        return m_locked;
    }

Q_SIGNALS:
    /**
     * Emitted when the screen gets locked.
     **/
    void locked();
    /**
     * Emitted when the screen gets unlocked.
     **/
    void unlocked();

public Q_SLOTS:
    /**
     * Locks the screen, either through a KWin effect (modern) or ScreenSaver Engine (legacy).
     **/
    void lock();

private:
    // legacy screen saver engine.
    SaverEngine *m_saverEngine;
    // indicates whether the screen is locked
    bool m_locked;
};
} // namespace ScreenLocker
} // namespace KWin

#endif
