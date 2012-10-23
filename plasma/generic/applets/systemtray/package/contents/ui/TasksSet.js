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

// This is stateless library, it may be shared between several qml files
.pragma library


// Represents task
var Task = function(id, index, category, data)
{
    this.id = id
    this.index = index
    this.category = category
    this.data = data
}


/**
 * Creates new set of tasks
 * @param categories_list a list of categories, this list gives order of categories
 */
var TasksSet = function(categories_list)
{
    var _categories = categories_list
    var _tasks = {} // internal set of tasks
    var _tasks_as_array = [] // array of tasks

    // number of tasks in each location with the same category
    var _cat_sizes = {}
    for (var i = 0, s = _categories.length; i < s; ++i) {
        _cat_sizes[_categories[i]] = 0
    }

    // Finds correct model index for new task
    function _findIndex(category)
    {
        var index = 0   // index is a sum of numbers of tasks groupped by category
        for (var c = 0, l = _categories.length; c < l && _categories[c] !== category; ++c)
            index += _cat_sizes[_categories[c]]
        return index
    }

    // Increments model indexes for tasks
    function _incIndexes(index)
    {
        for (var i = 0, len = _tasks_as_array.length; i < len; ++i) {
            var t = _tasks_as_array[i]
            if (t.index >= index)
                t.index++
        }
    }

    // Decrement indexes of tasks
    function _decIndexes(index)
    {
        for (var i = 0, len = _tasks_as_array.length; i < len; ++i) {
            var t = _tasks_as_array[i]
            if (t.index >= index)
                t.index--
        }
    }

    // Returns object representing properties of task
    function _get(id) {
        return _tasks[id]
    }
    this.get = _get

    /**
     * Adds new task to set
     * @param id unique id of task
     * @param category a category of task
     * @param data an additional data to accociate with task
     * @return a new task object
     */
    function _add(id, category, data)
    {
        var index = _findIndex(category)  // first of all, find correct model index to insert task
        _incIndexes(index)
        var t = new Task(id, index, category, data)
        _tasks[id] = t
        _cat_sizes[category]++
        _tasks_as_array.push(t)
        return t
    }
    this.add = _add

    /**
     * Removes tasks from set
     * @param id an unique ID of task
     * @return an object containing properties of old task
     */
    function _remove(id)
    {
        var t = _tasks[id] //find task using ID
        // remove task from array, we remove last item and move it to new place
        var arr_index = 0
        var len = _tasks_as_array.length
        for (; _tasks_as_array[arr_index] !== t; ++arr_index);
        if (arr_index < len-1)
            _tasks_as_array[arr_index] = _tasks_as_array[len-1] // move item
        _tasks_as_array.pop()
        // remove task from internal set & decrement indexes
        _cat_sizes[t.category]--
        delete _tasks[id]
        _decIndexes(t.index)
        return t
    }
    this.remove = _remove
}
