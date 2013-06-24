/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2010 Rohan Prabhu <rohan@rohanprabhu.com>
Copyright (C) 2011, 2012 Martin Gräßlin <mgraesslin@kde.org>

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

#include "workspace_wrapper.h"
#include "../client.h"
#include "../outline.h"
#include "../screens.h"
#include "../virtualdesktops.h"
#include "../workspace.h"
#ifdef KWIN_BUILD_ACTIVITIES
#include "../activities.h"
#endif

#include <QDesktopWidget>

namespace KWin {

WorkspaceWrapper::WorkspaceWrapper(QObject* parent) : QObject(parent)
{
    KWin::Workspace *ws = KWin::Workspace::self();
    KWin::VirtualDesktopManager *vds = KWin::VirtualDesktopManager::self();
    connect(ws, SIGNAL(desktopPresenceChanged(KWin::Client*,int)), SIGNAL(desktopPresenceChanged(KWin::Client*,int)));
    connect(ws, SIGNAL(currentDesktopChanged(int,KWin::Client*)), SIGNAL(currentDesktopChanged(int,KWin::Client*)));
    connect(ws, SIGNAL(clientAdded(KWin::Client*)), SIGNAL(clientAdded(KWin::Client*)));
    connect(ws, SIGNAL(clientAdded(KWin::Client*)), SLOT(setupClientConnections(KWin::Client*)));
    connect(ws, SIGNAL(clientRemoved(KWin::Client*)), SIGNAL(clientRemoved(KWin::Client*)));
    connect(ws, SIGNAL(clientActivated(KWin::Client*)), SIGNAL(clientActivated(KWin::Client*)));
    connect(vds, SIGNAL(countChanged(uint,uint)), SIGNAL(numberDesktopsChanged(uint)));
    connect(vds, SIGNAL(layoutChanged(int,int)), SIGNAL(desktopLayoutChanged()));
    connect(ws, SIGNAL(clientDemandsAttentionChanged(KWin::Client*,bool)), SIGNAL(clientDemandsAttentionChanged(KWin::Client*,bool)));
#ifdef KWIN_BUILD_ACTIVITIES
    KWin::Activities *activities = KWin::Activities::self();
    connect(activities, SIGNAL(currentChanged(QString)), SIGNAL(currentActivityChanged(QString)));
    connect(activities, SIGNAL(added(QString)), SIGNAL(activitiesChanged(QString)));
    connect(activities, SIGNAL(added(QString)), SIGNAL(activityAdded(QString)));
    connect(activities, SIGNAL(removed(QString)), SIGNAL(activitiesChanged(QString)));
    connect(activities, SIGNAL(removed(QString)), SIGNAL(activityRemoved(QString)));
#endif
    connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), SIGNAL(numberScreensChanged(int)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), SIGNAL(screenResized(int)));
    foreach (KWin::Client *client, ws->clientList()) {
        setupClientConnections(client);
    }
}

int WorkspaceWrapper::currentDesktop() const
{
    return VirtualDesktopManager::self()->current();
}

int WorkspaceWrapper::numberOfDesktops() const
{
    return VirtualDesktopManager::self()->count();
}

void WorkspaceWrapper::setCurrentDesktop(int desktop)
{
    VirtualDesktopManager::self()->setCurrent(desktop);
}

void WorkspaceWrapper::setNumberOfDesktops(int count)
{
    VirtualDesktopManager::self()->setCount(count);
}

#define GETTER( rettype, getterName ) \
rettype WorkspaceWrapper::getterName( ) const { \
    return Workspace::self()->getterName(); \
}
GETTER(KWin::Client*, activeClient)
GETTER(QList< KWin::Client* >, clientList)

#undef GETTER

QString WorkspaceWrapper::currentActivity() const
{
#ifdef KWIN_BUILD_ACTIVITIES
    return Activities::self()->current();
#else
    return QString();
#endif
}

QStringList WorkspaceWrapper::activityList() const
{
#ifdef KWIN_BUILD_ACTIVITIES
    return Activities::self()->all();
#else
    return QStringList();
#endif
}

#define SLOTWRAPPER(name) \
void WorkspaceWrapper::name( ) { \
    Workspace::self()->name(); \
}

SLOTWRAPPER(slotSwitchToNextScreen)
SLOTWRAPPER(slotWindowToNextScreen)
SLOTWRAPPER(slotToggleShowDesktop)

SLOTWRAPPER(slotWindowMaximize)
SLOTWRAPPER(slotWindowMaximizeVertical)
SLOTWRAPPER(slotWindowMaximizeHorizontal)
SLOTWRAPPER(slotWindowMinimize)
SLOTWRAPPER(slotWindowShade)
SLOTWRAPPER(slotWindowRaise)
SLOTWRAPPER(slotWindowLower)
SLOTWRAPPER(slotWindowRaiseOrLower)
SLOTWRAPPER(slotActivateAttentionWindow)
SLOTWRAPPER(slotWindowPackLeft)
SLOTWRAPPER(slotWindowPackRight)
SLOTWRAPPER(slotWindowPackUp)
SLOTWRAPPER(slotWindowPackDown)
SLOTWRAPPER(slotWindowGrowHorizontal)
SLOTWRAPPER(slotWindowGrowVertical)
SLOTWRAPPER(slotWindowShrinkHorizontal)
SLOTWRAPPER(slotWindowShrinkVertical)
SLOTWRAPPER(slotWindowQuickTileLeft)
SLOTWRAPPER(slotWindowQuickTileRight)
SLOTWRAPPER(slotWindowQuickTileTopLeft)
SLOTWRAPPER(slotWindowQuickTileTopRight)
SLOTWRAPPER(slotWindowQuickTileBottomLeft)
SLOTWRAPPER(slotWindowQuickTileBottomRight)

SLOTWRAPPER(slotSwitchWindowUp)
SLOTWRAPPER(slotSwitchWindowDown)
SLOTWRAPPER(slotSwitchWindowRight)
SLOTWRAPPER(slotSwitchWindowLeft)

SLOTWRAPPER(slotIncreaseWindowOpacity)
SLOTWRAPPER(slotLowerWindowOpacity)

SLOTWRAPPER(slotWindowOperations)
SLOTWRAPPER(slotWindowClose)
SLOTWRAPPER(slotWindowMove)
SLOTWRAPPER(slotWindowResize)
SLOTWRAPPER(slotWindowAbove)
SLOTWRAPPER(slotWindowBelow)
SLOTWRAPPER(slotWindowOnAllDesktops)
SLOTWRAPPER(slotWindowFullScreen)
SLOTWRAPPER(slotWindowNoBorder)

SLOTWRAPPER(slotWindowToNextDesktop)
SLOTWRAPPER(slotWindowToPreviousDesktop)
SLOTWRAPPER(slotWindowToDesktopRight)
SLOTWRAPPER(slotWindowToDesktopLeft)
SLOTWRAPPER(slotWindowToDesktopUp)
SLOTWRAPPER(slotWindowToDesktopDown)

#undef SLOTWRAPPER

#define SLOTWRAPPER(name,direction) \
void WorkspaceWrapper::name( ) { \
    VirtualDesktopManager::self()->moveTo<direction>(options->isRollOverDesktops()); \
}

SLOTWRAPPER(slotSwitchDesktopNext,DesktopNext)
SLOTWRAPPER(slotSwitchDesktopPrevious,DesktopPrevious)
SLOTWRAPPER(slotSwitchDesktopRight,DesktopRight)
SLOTWRAPPER(slotSwitchDesktopLeft,DesktopLeft)
SLOTWRAPPER(slotSwitchDesktopUp,DesktopAbove)
SLOTWRAPPER(slotSwitchDesktopDown,DesktopBelow)

#undef SLOTWRAPPER

void WorkspaceWrapper::setActiveClient(KWin::Client* client)
{
    KWin::Workspace::self()->activateClient(client);
}

QSize WorkspaceWrapper::workspaceSize() const
{
    return QSize(workspaceWidth(), workspaceHeight());
}

QSize WorkspaceWrapper::displaySize() const
{
    return QSize(KWin::displayWidth(), KWin::displayHeight());
}

int WorkspaceWrapper::displayWidth() const
{
    return KWin::displayWidth();
}

int WorkspaceWrapper::displayHeight() const
{
    return KWin::displayHeight();
}

QRect WorkspaceWrapper::clientArea(ClientAreaOption option, const QPoint &p, int desktop) const
{
    return Workspace::self()->clientArea(static_cast<clientAreaOption>(option), p, desktop);
}

QRect WorkspaceWrapper::clientArea(ClientAreaOption option, const KWin::Client *c) const
{
    return Workspace::self()->clientArea(static_cast<clientAreaOption>(option), c);
}

QRect WorkspaceWrapper::clientArea(ClientAreaOption option, int screen, int desktop) const
{
    return Workspace::self()->clientArea(static_cast<clientAreaOption>(option), screen, desktop);
}

QString WorkspaceWrapper::desktopName(int desktop) const
{
    return VirtualDesktopManager::self()->name(desktop);
}

QString WorkspaceWrapper::supportInformation() const
{
    return Workspace::self()->supportInformation();
}

void WorkspaceWrapper::setupClientConnections(KWin::Client *client)
{
    connect(client, SIGNAL(clientMinimized(KWin::Client*,bool)), SIGNAL(clientMinimized(KWin::Client*)));
    connect(client, SIGNAL(clientUnminimized(KWin::Client*,bool)), SIGNAL(clientUnminimized(KWin::Client*)));
    connect(client, SIGNAL(clientManaging(KWin::Client*)), SIGNAL(clientManaging(KWin::Client*)));
    connect(client, SIGNAL(clientFullScreenSet(KWin::Client*,bool,bool)), SIGNAL(clientFullScreenSet(KWin::Client*,bool,bool)));
    connect(client, SIGNAL(clientMaximizedStateChanged(KWin::Client*,bool,bool)), SIGNAL(clientMaximizeSet(KWin::Client*,bool,bool)));
}

void WorkspaceWrapper::showOutline(const QRect &geometry)
{
    outline()->show(geometry);
}

void WorkspaceWrapper::showOutline(int x, int y, int width, int height)
{
    outline()->show(QRect(x, y, width, height));
}

void WorkspaceWrapper::hideOutline()
{
    outline()->hide();
}

Client *WorkspaceWrapper::getClient(qulonglong windowId)
{
    return Workspace::self()->findClient(WindowMatchPredicate(windowId));
}

QSize WorkspaceWrapper::desktopGridSize() const
{
    return VirtualDesktopManager::self()->grid().size();
}

int WorkspaceWrapper::desktopGridWidth() const
{
    return desktopGridSize().width();
}

int WorkspaceWrapper::desktopGridHeight() const
{
    return desktopGridSize().height();
}

int WorkspaceWrapper::workspaceHeight() const
{
    return desktopGridHeight() * displayHeight();
}

int WorkspaceWrapper::workspaceWidth() const
{
    return desktopGridWidth() * displayWidth();
}

int WorkspaceWrapper::numScreens() const
{
    return screens()->count();
}

int WorkspaceWrapper::activeScreen() const
{
    return screens()->current();
}

} // KWin
