/*
    Copyright (C) 2012  Frederik Gladhorn <gladhorn@kde.org>*

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "accessibleplasmaview.h"
#include <plasma/containment.h>
#include <desktopcorona.h>

using namespace Plasma;

AccessiblePlasmaView::AccessiblePlasmaView(Plasma::View *view, QAccessible::Role role)
    : QAccessibleWidgetEx(view, role)
{}

QString AccessiblePlasmaView::text ( QAccessible::Text t, int child ) const
{
    if (child == 0 && t == QAccessible::Name)
        return QString("Plasma Desktop");
    return QAccessibleWidgetEx::text ( t, child );
}

int AccessiblePlasmaView::childCount() const
{
    return view()->containment() ? view()->containment()->applets().size() : 0;
}

int AccessiblePlasmaView::indexOfChild ( const QAccessibleInterface* child ) const
{
    return QAccessibleWidgetEx::indexOfChild ( child );
}

int AccessiblePlasmaView::navigate ( QAccessible::RelationFlag rel, int entry, QAccessibleInterface** target ) const
{
    *target = 0;
    switch (rel) {
    case QAccessible::Child:
        // FIXME check if valid
        *target = new AccessiblePlasmaApplet(view()->containment()->applets().at(entry - 1));
        return 0;
    default:
        break;
    }

    return QAccessibleWidgetEx::navigate ( rel, entry, target );
}


AccessiblePlasmaPanelView::AccessiblePlasmaPanelView ( View* view)
    : AccessiblePlasmaView ( view, QAccessible::Pane )
{}

QString AccessiblePlasmaPanelView::text ( QAccessible::Text t, int child ) const
{
    if (child == 0 && t == QAccessible::Name)
        return QString("Plasma Panel");
    return QAccessibleWidgetEx::text ( t, child );
}

AccessiblePlasmaApp::AccessiblePlasmaApp(PlasmaApp *app)
    : QAccessibleApplication(), m_app(app)
{}

int AccessiblePlasmaApp::childCount() const
{
    // FIXME: return actual desktops or somesuch
    return QAccessibleApplication::childCount();
}

// int AccessiblePlasmaApp::navigate(QAccessible::RelationFlag , int , QAccessibleInterface** ) const
// {
//     
//     return QAccessibleApplication::navigate(, , );
// }

AccessiblePlasmaApplet::AccessiblePlasmaApplet(Applet* applet)
    : QAccessibleObjectEx(applet)
{}

QString AccessiblePlasmaApplet::text(QAccessible::Text t, int child) const
{
    if (t == QAccessible::Name)
        return applet()->name();
    return QString();
}

int AccessiblePlasmaApplet::childCount() const
{
    return 0;
}
int AccessiblePlasmaApplet::childAt(int x, int y) const
{
    return -1;
}
int AccessiblePlasmaApplet::indexOfChild(const QAccessibleInterface* ) const
{
    return -1;
}
int AccessiblePlasmaApplet::navigate(QAccessible::RelationFlag relation, int index, QAccessibleInterface** iface) const
{
    return -1;
}
QRect AccessiblePlasmaApplet::rect(int child) const
{
    // FIXME check if this is sensible
    return applet()->screenRect();
}
QVariant AccessiblePlasmaApplet::invokeMethodEx(QAccessible::Method method, int child, const QVariantList& params)
{
    return QVariant();
}
QAccessible::Role AccessiblePlasmaApplet::role(int child) const
{
    return QAccessible::Pane;
}
QAccessible::Relation AccessiblePlasmaApplet::relationTo(int child, const QAccessibleInterface* other, int otherChild) const
{
    return QAccessible::Unrelated;
}
QAccessible::State AccessiblePlasmaApplet::state(int child) const
{
    // FIXME
    return 0;
}






