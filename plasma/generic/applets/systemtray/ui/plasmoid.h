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

#ifndef __SYSTEMTRAY__PLASMOID_H
#define __SYSTEMTRAY__PLASMOID_H


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include "applet.h"

#include <QtCore/QObject>
#include <QtCore/QSet>


namespace SystemTray
{
class Task;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Plasmoid
// Makes applet be accessible from QML code, like global object "plasmoid" for declarative applets
class Plasmoid: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* applet READ applet CONSTANT) ///< return pointer to applet
public:

    explicit Plasmoid(SystemTray::Applet *parent);
    virtual ~Plasmoid();
    QObject* applet() const { return m_applet; }

private:
    SystemTray::Applet* m_applet;
};

} // namespace SystemTray

#endif // __SYSTEMTRAY__PLASMOID_H

