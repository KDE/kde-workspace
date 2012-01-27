/*
    Copyright (C) 2012  Frederik Gladhorn <gladhorn@kde.org>

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


#ifndef ACCESSIBLEPLASMAVIEW_H
#define ACCESSIBLEPLASMAVIEW_H

#include <qaccessibleobject.h>
#include <qaccessiblewidget.h>

#include <plasma/applet.h>

#include "desktopview.h"
#include "panelview.h"
#include "plasmaapp.h"


class AccessiblePlasmaView : public QAccessibleWidgetEx
{
public:
    AccessiblePlasmaView(Plasma::View *view, QAccessible::Role role = QAccessible::Window);

    int childCount() const;
    int indexOfChild ( const QAccessibleInterface* child ) const;
    int navigate ( QAccessible::RelationFlag rel, int entry, QAccessibleInterface** target ) const;

    QString text ( Text t, int child ) const;

private:
    inline Plasma::View *view() const
    {
        return static_cast<Plasma::View*>(object());
    }
};

class AccessiblePlasmaPanelView : public AccessiblePlasmaView
{
public:
    AccessiblePlasmaPanelView (Plasma::View* view);
    QString text ( QAccessible::Text t, int child ) const;

private:
    inline PanelView *view() const
    {
        return static_cast<PanelView*>(object());
    }
};

class AccessiblePlasmaApp : public QAccessibleApplication
{
public:
    AccessiblePlasmaApp(PlasmaApp *app);

    int childCount() const;
//     int navigate(RelationFlag , int , QAccessibleInterface** ) const;

private:
    PlasmaApp *m_app;
};

class AccessiblePlasmaApplet : public QAccessibleObjectEx
{
public:
    AccessiblePlasmaApplet(Plasma::Applet *applet);
    virtual int childCount() const;
    virtual int indexOfChild(const QAccessibleInterface* ) const;

    virtual int childAt(int x, int y) const;
    virtual QVariant invokeMethodEx(Method method, int child, const QVariantList& params);
    virtual int navigate(RelationFlag relation, int index, QAccessibleInterface** iface) const;
    virtual Relation relationTo(int child, const QAccessibleInterface* other, int otherChild) const;
    virtual Role role(int child) const;
    virtual State state(int child) const;
    virtual QString text(Text t, int child) const;

    virtual QRect rect(int child) const;
    
    inline Plasma::Applet *applet() const {
        return static_cast<Plasma::Applet*>(object());
    }
};

#endif // ACCESSIBLEPLASMAVIEW_H
