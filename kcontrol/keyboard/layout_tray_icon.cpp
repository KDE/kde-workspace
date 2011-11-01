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

#include "layout_tray_icon.h"

//#include <kdebug.h>
#include <kstatusnotifieritem.h>
#include <klocalizedstring.h>
#include <kmenu.h>

#include "xkb_rules.h"
#include "x11_helper.h"
#include "xkb_helper.h"
#include "keyboard_config.h"
#include "flags.h"
#include "layouts_menu.h"


//
// Layout Tray Icon
//
LayoutTrayIcon::LayoutTrayIcon(const Rules* rules_, const KeyboardConfig& keyboardConfig_):
	keyboardConfig(keyboardConfig_),
	rules(rules_),
	flags(new Flags()),
	layoutsMenu(new LayoutsMenu(keyboardConfig_, *rules, *flags))
{
    m_notifierItem = new KStatusNotifierItem(this);
    m_notifierItem->setCategory(KStatusNotifierItem::SystemServices);
    m_notifierItem->setStatus(KStatusNotifierItem::Active);
    m_notifierItem->setToolTipTitle(i18nc("tooltip title", "Keyboard Layout"));
    m_notifierItem->setTitle(i18nc("tooltip title", "Keyboard Layout"));

	KMenu* menu = new KMenu("");
    m_notifierItem->setContextMenu(menu);
	m_notifierItem->setStandardActionsEnabled(false);

    layoutMapChanged();

    m_notifierItem->setStatus(KStatusNotifierItem::Active);

    init();
}

LayoutTrayIcon::~LayoutTrayIcon()
{
	destroy();
	delete flags;
	delete layoutsMenu;
}

void LayoutTrayIcon::init()
{
    connect(m_notifierItem, SIGNAL(activateRequested(bool,QPoint)), this, SLOT(toggleLayout()));
    connect(m_notifierItem, SIGNAL(scrollRequested(int,Qt::Orientation)), this, SLOT(scrollRequested(int,Qt::Orientation)));
	connect(flags, SIGNAL(pixmapChanged()), this, SLOT(layoutChanged()));
}

void LayoutTrayIcon::destroy()
{
	disconnect(flags, SIGNAL(pixmapChanged()), this, SLOT(layoutChanged()));
    disconnect(m_notifierItem, SIGNAL(scrollRequested(int,Qt::Orientation)), this, SLOT(scrollRequested(int,Qt::Orientation)));
    disconnect(m_notifierItem, SIGNAL(activateRequested(bool,QPoint)), this, SLOT(toggleLayout()));
}

void LayoutTrayIcon::layoutMapChanged()
{
	flags->clearCache();

	KMenu* menu = m_notifierItem->contextMenu();
	menu->clear();
	QList<QAction*> actions = layoutsMenu->contextualActions();
	menu->addActions(actions);

	layoutChanged();
}

void LayoutTrayIcon::layoutChanged()
{
	LayoutUnit layoutUnit = X11Helper::getCurrentLayout();
	if( layoutUnit.isEmpty() )
		return;

//	QString shortText = Flags::getShortText(layoutUnit, *keyboardConfig);
//	kDebug() << "systray: LayoutChanged" << layoutUnit.toString() << shortText;
	QString longText = Flags::getLongText(layoutUnit, rules);

	m_notifierItem->setToolTipSubTitle(longText);

	const QIcon icon(getFlag(layoutUnit.layout));
	m_notifierItem->setToolTipIconByPixmap(icon);

	QIcon textOrIcon = flags->getIconWithText(layoutUnit, keyboardConfig);
	m_notifierItem->setIconByPixmap( textOrIcon );
}

void LayoutTrayIcon::toggleLayout()
{
	X11Helper::switchToNextLayout();
}

void LayoutTrayIcon::scrollRequested(int delta, Qt::Orientation /*orientation*/)
{
	X11Helper::scrollLayouts(delta > 0 ? 1 : -1);
}

const QIcon LayoutTrayIcon::getFlag(const QString& layout) const
{
	return keyboardConfig.isFlagShown() ? flags->getIcon(layout) : QIcon();
}
