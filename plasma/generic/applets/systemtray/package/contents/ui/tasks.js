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


/// Set of all known tasks
var tasks = {}

//Arrays of numbers of tasks grouped by category
var category_size = [ {}, {}, {} ]


// initialize arrays
/**
 * Clears arrays for number of tasks groupped by categoryies
 * @param categories_list a list of categories
 */
function clearCategoryArrays(categories_list) {
    for (var i = 0; i < category_size.length; ++i) {
        var clean_obj = category_size[i] = new Object
        for (var cat = 0; cat < categories_list.length; ++cat) {
            clean_obj[categories_list[cat]] = 0
        }
    }
}


/**
 * Represents task
 * @param id an ID of task
 * @param model_index an index of item in corresponding model
 * @param location a location of icon (location corresponds to model)
 * @param category a category of task
 */
var Task = function(id, model_index, location, category, item) {
    this.id     = id
    this.model_index = model_index
    this.location = location
    this.category = category
    this.item = item
}


/**
 * Increments model indexes for all tasks located in specified location
 * @param index if task has model index same as model_index or more than it then its model index will be incremented
 * @param location a location
 */
function incrementIndexes(index, location) {
    if (index === NO_INDEX)
        return
    for (var t in tasks) {
        if ( tasks[t].location === location && tasks[t].model_index >= index)
            ++tasks[t].model_index
    }
}

/**
 * Add task to set of tasks.
 * @param id an ID of task
 * @param model_index an index of item in corresponding model
 * @param location a location of icon (location corresponds to model)
 */
function addTask(id, model_index, location, category, item) {
    incrementIndexes(model_index, location)
    tasks[id] = new Task(id, model_index, location, category, item)
    category_size[location][category]++
}


/**
 * Decrements model indexes of tasks to make indexes corresponding to model
 * @param task a task
 */
function decrementIndexes(task) {
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
    decrementIndexes(task)
    category_size[task.location][task.category]--
    delete tasks[id]
}
