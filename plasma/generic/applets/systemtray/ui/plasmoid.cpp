/***********************************************************************************************************************
 * KDE System Tray (Plasmoid)
 *
 * Copyright (C) 2012 ROSA  <support@rosalab.ru>
 * License: GPLv2+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 **********************************************************************************************************************/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include "plasmoid.h"

#include "../core/task.h"

#include <inttypes.h>

#include <QtGui/QMenu>

#include <KDE/Plasma/IconWidget>
#include <KDE/KAction>
#include <KDE/Plasma/Containment>
#include <KDE/Plasma/Corona>
#include <KDE/KWindowSystem>


namespace SystemTray
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Plasmoid
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Plasmoid::Plasmoid(SystemTray::Applet *parent)
    : QObject(parent),
      m_applet(parent)
{
}


Plasmoid::~Plasmoid()
{
}

QVariant Plasmoid::createShortcutAction(QString action_id) const
{
    KAction *action = new KAction(parent());
    action->setObjectName(action_id);
    return QVariant::fromValue<QObject*>(action);
}


void Plasmoid::updateShortcutAction(QVariant action, QString shortcut) const
{
    KAction *act = qobject_cast<KAction*>(action.value<QObject*>());
    if (!act) {
        return;
    }

    act->forgetGlobalShortcut();
    if (!shortcut.isEmpty()) {
        act->setGlobalShortcut(KShortcut(QKeySequence(shortcut)),
                               KAction::ShortcutTypes(KAction::ActiveShortcut | KAction::DefaultShortcut),
                               KAction::NoAutoloading);
    }
}


void Plasmoid::destroyShortcutAction(QVariant action) const
{
    KAction *act = qobject_cast<KAction*>(action.value<QObject*>());
    if (act) {
        delete act;
    }
}


void Plasmoid::showMenu(QVariant menu_var, int x, int y, QVariant item_var) const
{
    QGraphicsItem *item = qobject_cast<QGraphicsItem*>(item_var.value<QObject*>());
    QMenu *menu = qobject_cast<QMenu*>(menu_var.value<QObject*>());
    if (menu) {
        QPoint pos(x, y);
        if ( m_applet ) {
            menu->adjustSize();
            if (item && m_applet->containment() && m_applet->containment()->corona()) {
                pos = m_applet->containment()->corona()->popupPosition(item, menu->size());
            } else {
                pos = m_applet->popupPosition(menu->size());
            }
        }
        menu->popup(pos);
    }
}


QPoint Plasmoid::popupPosition(QVariant item_var, QSize size, int align) const
{
    QGraphicsItem *item = qobject_cast<QGraphicsItem*>(item_var.value<QObject*>());
    if (m_applet) {
        if ( item && m_applet->containment() && m_applet->containment()->corona() ) {
            return m_applet->containment()->corona()->popupPosition(item, size, (Qt::AlignmentFlag)align);
        }
        return m_applet->popupPosition(size, (Qt::AlignmentFlag)align);
    }
    return QPoint();
}


void Plasmoid::hideFromTaskbar(qulonglong win_id) const
{
    if (win_id > 0) {
        KWindowSystem::setState(win_id, NET::SkipTaskbar | NET::SkipPager);
    }
}

QString Plasmoid::getUniqueId(QObject *obj) const
{
    return QString::number(reinterpret_cast<uintmax_t>(obj));
}


} // namespace SystemTray
