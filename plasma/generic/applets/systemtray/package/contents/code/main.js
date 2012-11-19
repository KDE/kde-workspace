/***********************************************************************************************************************
 * KDE System Tray (Plasmoid)
 *
 * Copyright (C) 2012 ROSA  <support@rosalab.ru>
 * License: LGPLv2+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **********************************************************************************************************************/

// This isn't a stateless library, it is intended only as a stuff for main.qml

// Constants
var ICONS_SIZE = 24 // Size of icons, icons are square i.e. width == height
var MINIMUM_SIZE = 8 // minimum size of widget
var ARROW_MARGINS = 5 // margins for an arrow
var BLINK_INTERVAL = 750 // time interval of blinking
var TASK_NOTIFICATIONS_TYPEID = "org.kde.notifications"
var USE_GRID_IN_POPUP = false  // true if in popup icons should be placed like a grid without names


// [const] Location of item (model for icon)
var LOCATION_TRAY = 0
var LOCATION_POPUP = LOCATION_TRAY + 1
var LOCATION_NOTIFICATION = LOCATION_POPUP + 1


// [const] List of possible locations
var LOCATIONS = [
    LOCATION_TRAY,
    LOCATION_POPUP,
    LOCATION_NOTIFICATION
]

// [const] Number of locations
var LOCATIONS_NUMBER = LOCATIONS.length

// This list determines order of categories of tasks
var CATEGORIES = [
    UnknownCategory,
    ApplicationStatus,
    Communications,
    SystemServices,
    Hardware
]

// all available tasks by their id
var allTasks = {}

// Set of tasks sets, each set for one location (area)
var tasks = new Array(LOCATIONS_NUMBER)


// Returns location of task by ID of task
function findLocation(id)
{
    for (var i = 0; i < LOCATIONS_NUMBER; ++i) {
        if (tasks[i].get(id))
            return LOCATIONS[i]
    }
}
