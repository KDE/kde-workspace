/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 1997 to 2002 Cristian Tibirna <tibirna@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

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

#ifndef KWIN_PLACEMENT_H
#define KWIN_PLACEMENT_H
// KWin
#include <kwinglobals.h>
// Qt
#include <QPoint>
#include <QRect>
#include <QList>

class QObject;

namespace KWin
{

class Client;

class Placement
{
public:
    virtual ~Placement();

    /**
     * Placement policies. How workspace decides the way windows get positioned
     * on the screen. The better the policy, the heavier the resource use.
     * Normally you don't have to worry. What the WM adds to the startup time
     * is nil compared to the creation of the window itself in the memory
     */
    enum Policy {
        NoPlacement, // not really a placement
        Default, // special, means to use the global default
        Unknown, // special, means the function should use its default
        Random,
        Smart,
        Cascade,
        Centered,
        ZeroCornered,
        UnderMouse, // special
        OnMainWindow, // special
        Maximizing
    };

    void place(Client* c, QRect& area);

    void placeAtRandom(Client* c, const QRect& area, Policy next = Unknown);
    void placeCascaded(Client* c, QRect& area, Policy next = Unknown);
    void placeSmart(Client* c, const QRect& area, Policy next = Unknown);
    void placeMaximizing(Client* c, QRect& area, Policy next = Unknown);
    void placeCentered(Client* c, const QRect& area, Policy next = Unknown);
    void placeZeroCornered(Client* c, const QRect& area, Policy next = Unknown);
    void placeDialog(Client* c, QRect& area, Policy next = Unknown);
    void placeUtility(Client* c, QRect& area, Policy next = Unknown);

    void reinitCascading(int desktop);

    /**
     * Cascades all clients on the current desktop
     **/
    void cascadeDesktop();
    /**
     *   Unclutters the current desktop by smart-placing all clients again.
     **/
    void unclutterDesktop();

    static Policy policyFromString(const QString& policy, bool no_special);
    static const char* policyToString(Policy policy);

private:
    void place(Client* c, QRect& area, Policy policy, Policy nextPlacement = Unknown);
    void placeUnderMouse(Client* c, QRect& area, Policy next = Unknown);
    void placeOnMainWindow(Client* c, QRect& area, Policy next = Unknown);
    QRect checkArea(const Client*c, const QRect& area);

    //CT needed for cascading+
    struct DesktopCascadingInfo {
        QPoint pos;
        int col;
        int row;
    };

    QList<DesktopCascadingInfo> cci;

    KWIN_SINGLETON(Placement)
};

} // namespace

#endif
