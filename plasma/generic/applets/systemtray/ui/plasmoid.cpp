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

#include "applet.h"

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
Plasmoid::Plasmoid(QObject *parent)
    : QObject(parent),
      m_form(Plasmoid::Planar),
      m_location(Plasmoid::Floating)
{
}


Plasmoid::~Plasmoid()
{
}


Plasmoid::Location Plasmoid::location() const
{
    return m_location;
}


void Plasmoid::setLocation(Plasmoid::Location loc)
{
    if (loc == m_location) {
        return;
    }

    m_location = loc;
    emit changedLocation();
}


unsigned int Plasmoid::id() const
{
    Applet *applet = parentApplet();
    return applet ? applet->id() : 0;
}


QVariant Plasmoid::applet() const
{
    return QVariant::fromValue<QObject*>(parentApplet());
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
        SystemTray::Applet *applet = parentApplet();
        QPoint pos(x, y);
        if ( applet ) {
            menu->adjustSize();
            if (item && applet->containment() && applet->containment()->corona()) {
                pos = applet->containment()->corona()->popupPosition(item, menu->size());
            } else {
                pos = applet->popupPosition(menu->size());
            }
        }
        menu->popup(pos);
    }
}


QPoint Plasmoid::popupPosition(QVariant item_var, QSize size, int align) const
{
    QGraphicsItem *item = qobject_cast<QGraphicsItem*>(item_var.value<QObject*>());
    SystemTray::Applet *applet = parentApplet();
    if (applet) {
        if ( item && applet->containment() && applet->containment()->corona() ) {
            return applet->containment()->corona()->popupPosition(item, size, (Qt::AlignmentFlag)align);
        }
        return applet->popupPosition(size, (Qt::AlignmentFlag)align);
    }
    return QPoint();
}


void Plasmoid::hideFromTaskbar(qulonglong win_id) const
{
    if (win_id > 0) {
        KWindowSystem::setState(win_id, NET::SkipTaskbar | NET::SkipPager);
    }
}


Plasmoid::FormFactor Plasmoid::formFactor() const
{
    return m_form;
}


void Plasmoid::setFormFactor(Plasmoid::FormFactor form_factor)
{
    if (form_factor == m_form) {
        return;
    }

    m_form = form_factor;
    emit changedFormFactor();
}

SystemTray::Applet *Plasmoid::parentApplet() const
{
    return qobject_cast<SystemTray::Applet*>(parent());
}

} // namespace SystemTray
