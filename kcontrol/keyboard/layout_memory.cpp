/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "layout_memory.h"

#include <QtCore/QSet>
#include <QtGui/QX11Info>

#include <kdebug.h>
#include <kwindowsystem.h>

#include "x11_helper.h"
#include "xkb_helper.h"


LayoutMemory::LayoutMemory(const KeyboardConfig& keyboardConfig_):
	prevLayoutList(X11Helper::getLayoutsList()),
	keyboardConfig(keyboardConfig_)
{
	registerListeners();
}

LayoutMemory::~LayoutMemory()
{
	unregisterListeners();
}

void LayoutMemory::configChanged()
{
//	this->layoutMap.clear();	// if needed this will be done on layoutMapChanged event
	unregisterListeners();
	registerListeners();
}

void LayoutMemory::registerListeners()
{
	if( keyboardConfig.switchingPolicy ==  KeyboardConfig::SWITCH_POLICY_WINDOW
			|| keyboardConfig.switchingPolicy ==  KeyboardConfig::SWITCH_POLICY_APPLICATION ) {
		connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(windowChanged(WId)));
//		connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(windowRemoved(WId)));
	}
	if( keyboardConfig.switchingPolicy ==  KeyboardConfig::SWITCH_POLICY_DESKTOP ) {
		connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(desktopChanged(int)));
	}
}

void LayoutMemory::unregisterListeners()
{
    disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(windowChanged(WId)));
    disconnect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(desktopChanged(int)));
//	disconnect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(windowRemoved(WId)));
}

QString LayoutMemory::getCurrentMapKey() {
	switch(keyboardConfig.switchingPolicy) {
	case KeyboardConfig::SWITCH_POLICY_WINDOW: {
		WId wid = KWindowSystem::self()->activeWindow();
		KWindowInfo winInfo = KWindowSystem::windowInfo(wid, NET::WMWindowType);
		NET::WindowType windowType = winInfo.windowType( NET::NormalMask | NET::DesktopMask | NET::DialogMask );
		kDebug() << "window type" << windowType;

		// we ignore desktop type so that our keybaord layout applet on desktop could change layout properly
		if( windowType == NET::Desktop )
			return previousLayoutMapKey;
		if( windowType != NET::Unknown && windowType != NET::Normal && windowType != NET::Dialog )
			return QString();

		return QString::number(wid);
	}
	case KeyboardConfig::SWITCH_POLICY_APPLICATION: {
		WId wid = KWindowSystem::self()->activeWindow();
		KWindowInfo winInfo = KWindowSystem::windowInfo(wid, NET::WMWindowType, NET::WM2WindowClass);
		NET::WindowType windowType = winInfo.windowType( NET::NormalMask | NET::DesktopMask | NET::DialogMask );
		kDebug() << "window type" << windowType;

		// we ignore desktop type so that our keybaord layout applet on desktop could change layout properly
		if( windowType == NET::Desktop )
			return previousLayoutMapKey;
		if( windowType != NET::Unknown && windowType != NET::Normal && windowType != NET::Dialog )
			return QString();

		// shall we use pid or window class ??? - class seems better (see e.g. https://bugs.kde.org/show_bug.cgi?id=245507)
		// for window class shall we use class.class or class.name? (seem class.class is a bit better - more app-oriented)
		kDebug() << "New active window with class.class: " << winInfo.windowClassClass();
		return QString(winInfo.windowClassClass());
//		NETWinInfo winInfoForPid( QX11Info::display(), wid, QX11Info::appRootWindow(), NET::WMPid);
//		return QString::number(winInfoForPid.pid());
	}
	case KeyboardConfig::SWITCH_POLICY_DESKTOP:
		return QString::number(KWindowSystem::self()->currentDesktop());
	default:
		return QString();
	}
}

static
bool isExtraSubset(const QList<LayoutUnit>& allLayouts, const QList<LayoutUnit>& newList)
{
	if( allLayouts.first() != newList.first() )
		return false;
	foreach(const LayoutUnit& layoutUnit, newList) {
		if( ! allLayouts.contains(layoutUnit) )
			return false;
	}
	return true;
}

void LayoutMemory::layoutMapChanged()
{
	QList<LayoutUnit> newLayoutList(X11Helper::getLayoutsList());

	if( prevLayoutList == newLayoutList )
		return;

	kDebug() << "Layout map change: " << LayoutSet::toString(prevLayoutList) << "-->" << LayoutSet::toString(newLayoutList);
	prevLayoutList = newLayoutList;

	//TODO: need more thinking here on how to support external map resetting
	if( keyboardConfig.configureLayouts
			&& keyboardConfig.isSpareLayoutsEnabled()
			&& isExtraSubset(keyboardConfig.layouts, newLayoutList) ) {
		kDebug() << "Layout map change for extra layout";
		layoutChanged();	// to remember new map for active "window"
	}
	else {
//		if( newLayoutList != keyboardConfig.getDefaultLayouts() ) {
			//		layoutList = newLayoutList;
			kDebug() << "Layout map change from external source: clearing layout memory";
			layoutMap.clear();
//		}
	}
}

void LayoutMemory::layoutChanged()
{
	QString layoutMapKey = getCurrentMapKey();
	if( layoutMapKey.isEmpty() )
		return;

	layoutMap[ layoutMapKey ] = X11Helper::getCurrentLayouts();
}

void LayoutMemory::setCurrentLayoutFromMap()
{
	QString layoutMapKey = getCurrentMapKey();
	if( layoutMapKey.isEmpty() )
		return;

	if( ! layoutMap.contains(layoutMapKey) ) {
//		kDebug() << "new key for layout map" << layoutMapKey;

		if( ! X11Helper::isDefaultLayout() ) {
//			kDebug() << "setting default layout for container key" << layoutMapKey;
			if( keyboardConfig.configureLayouts && keyboardConfig.isSpareLayoutsEnabled()
					&& X11Helper::getLayoutsList() != keyboardConfig.getDefaultLayouts() ) {
				XkbHelper::initializeKeyboardLayouts(keyboardConfig.getDefaultLayouts());
			}
			X11Helper::setDefaultLayout();
		}
	}
	else {
		LayoutSet layoutFromMap = layoutMap[layoutMapKey];
		kDebug() << "Setting layout map item" << layoutFromMap.currentLayout.toString()
				<< "for container key" << layoutMapKey;

		LayoutSet currentLayouts = X11Helper::getCurrentLayouts();
		if( layoutFromMap.layouts != currentLayouts.layouts ) {
			if( keyboardConfig.configureLayouts && keyboardConfig.isSpareLayoutsEnabled() ) {
				XkbHelper::initializeKeyboardLayouts(layoutFromMap.layouts);
			}
			X11Helper::setLayout( layoutFromMap.currentLayout );
		}
		else if( layoutFromMap.currentLayout != currentLayouts.currentLayout ) {
			X11Helper::setLayout( layoutFromMap.currentLayout );
		}
	}

	previousLayoutMapKey = layoutMapKey;
}

//#include <kplugininfo.h>
//#include <plasma/containment.h>

void LayoutMemory::windowChanged(WId /*wId*/)
{
//	KPluginInfo::List plugins = Plasma::Containment::listContainments();
//	foreach(KPluginInfo info, plugins) {
//		kDebug() << "applets" << info.name();
//	}
	setCurrentLayoutFromMap();
}

void LayoutMemory::desktopChanged(int /*desktop*/)
{
	setCurrentLayoutFromMap();
}
