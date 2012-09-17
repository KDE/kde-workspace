/***********************************************************************************************************************
 * ROSA System Tray (KDE Plasmoid)
 * Copyright ⓒ 2011-2012 ROSA  <support@rosalab.ru>
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
.pragma library

// Location of icon (model for icon)
var LOCATION = {
    TRAY:      0,
    POPUP:     1,
    NOTIFICATIONS: 2
}


/// Value for index of model indicating undefined index
var NO_INDEX = -1


/**
 * Represents task
 * @param id an ID of task
 * @param model_index an index of item in corresponding model
 * @param location a location of icon (location corresponds to model)
 */
var Task = function(id, model_index, location, item) {
    this.id     = id
    this.model_index = model_index
    this.location = location
    this.item = item
}

/// Set of all known tasks
var tasks = {}

/**
 * Add task to set of tasks.
 * @param id an ID of task
 * @param model_index an index of item in corresponding model
 * @param location a location of icon (location corresponds to model)
 */
function addTask(id, model_index, location, item) {
    tasks[id] = new Task(id, model_index, location, item)
}


/**
 * Decrements model indexes of tasks to make indexes corresponding to model
 * @param task a task
 */
function unbind(task) {
    var loc = task.location
    var index = task.model_index
    // update model indexes
    if (index !== NO_INDEX) {
        for (var t in tasks) {
            if ( tasks[t].location === loc && tasks[t].model_index > index )
                --tasks[t].model_index
        }
    }
    task.model_index = NO_INDEX
}


/**
 * Removes task from set of tasks
 * @param id an ID of task
 */
function removeTask(id) {
    var task = tasks[id]
    unbind(task)
    delete tasks[id]
}
