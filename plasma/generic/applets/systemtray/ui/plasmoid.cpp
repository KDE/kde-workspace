/***********************************************************************************************************************
 * ROSA System Tray (KDE Plasmoid)
 * Copyright (C) 2011-2012 ROSA  <support@rosalab.ru>
 * License: GPLv3+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **********************************************************************************************************************/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include "plasmoid.h"

#include "applet.h"



namespace SystemTray
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// struct Plasmoid::_Private
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Plasmoid::_Private
{
    Plasmoid::FormFactor form;
    Plasmoid::Location   location;

    _Private();
};


Plasmoid::_Private::_Private():
    form(Plasmoid::Planar),
    location(Plasmoid::Floating)
{

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Plasmoid
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Plasmoid::Plasmoid(QObject *parent):
    QObject(parent),
    d(new _Private)
{
}


Plasmoid::~Plasmoid()
{
    delete d;
}


Plasmoid::Location Plasmoid::location() const
{
    return d->location;
}


void Plasmoid::setLocation(Plasmoid::Location loc)
{
    if (loc == d->location)
        return;
    d->location = loc;
    emit changedLocation();
}


Plasmoid::FormFactor Plasmoid::formFactor() const
{
    return d->form;
}


void Plasmoid::setFormFactor(Plasmoid::FormFactor form_factor)
{
    if (form_factor == d->form)
        return;
    d->form = form_factor;
    emit changedFormFactor();
}


} // namespace SystemTray
