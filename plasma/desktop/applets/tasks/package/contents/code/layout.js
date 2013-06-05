/***************************************************************************
 *   Copyright (C) 2012-2013 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


function horizontalMargins() {
    return taskFrame.margins.left + taskFrame.margins.right;
}

function verticalMargins() {
    return taskFrame.margins.top + taskFrame.margins.bottom;
}

function maxStripes() {
    var length = tasks.vertical ? taskList.width : taskList.height;
    var minimum = tasks.vertical ? preferredMinWidth() : preferredMinHeight();

    return Math.min(tasks.maxStripes, Math.max(1, Math.floor(length / minimum)));
}

function tasksPerStripe() {
    if (tasks.forceStripes) {
        return Math.ceil(tasksModel.count / maxStripes());
    } else {
        var length = tasks.vertical ? taskList.height : taskList.width;
        var minimum = tasks.vertical ? preferredMinHeight() : preferredMinWidth();

        return Math.floor(length / minimum);
    }
}

function calculateStripes() {
    var stripes = tasks.forceStripes ? tasks.maxStripes : Math.min(tasks.maxStripes, Math.ceil(tasksModel.count / tasksPerStripe()));

    return Math.min(stripes, maxStripes());
}

function full() {
    return (maxStripes() == calculateStripes());
}

function optimumCapacity() {
    var length = tasks.vertical ? taskList.height : taskList.width;
    var maximum = tasks.vertical ? preferredMaxHeight() : preferredMaxWidth();

    return Math.ceil(length / maximum) * maxStripes();
}

function layoutWidth() {
    if (tasks.forceStripes && !tasks.vertical) {
        return Math.min(tasks.width, Math.max(preferredMaxWidth(), tasksPerStripe() * preferredMaxWidth()));
    } else {
        return tasks.width;
    }
}

function layoutHeight() {
    if (tasks.forceStripes && tasks.vertical) {
        return Math.min(tasks.height, Math.max(preferredMaxHeight(), tasksPerStripe() * preferredMaxHeight()));
    } else {
        return tasks.height;
    }
}

function preferredMinWidth() {
    if (tasks.vertical) {
        return horizontalMargins() + theme.smallIconSize;
    } else {
        return horizontalMargins() + theme.smallIconSize + 3 + (theme.defaultFont.mSize.width * 12);
    }
}

function preferredMaxWidth() {
    return preferredMinWidth() * 1.8;
}

function preferredMinHeight() {
    return verticalMargins() + Math.max(theme.smallIconSize, theme.defaultFont.mSize.height);
}

function preferredMaxHeight() {
    return verticalMargins() + Math.min(theme.smallIconSize * 3, theme.defaultFont.mSize.height * 3);
}

function taskWidth() {
    if (tasks.vertical) {
        return Math.floor(taskList.width / calculateStripes());
    } else {
        if (full() && Math.max(1, tasksModel.count) > tasksPerStripe()) {
            return Math.floor(taskList.width / Math.ceil(tasksModel.count / maxStripes()));
        } else {
            return Math.min(preferredMaxWidth(), Math.floor(taskList.width / Math.min(tasksModel.count, tasksPerStripe())));
        }
    }
}

function taskHeight() {
    if (tasks.vertical) {
        if (full() && Math.max(1, tasksModel.count) > tasksPerStripe()) {
            return Math.floor(taskList.height / Math.ceil(tasksModel.count / maxStripes()));
        } else {
            return Math.min(preferredMaxHeight(), Math.floor(taskList.height / Math.min(tasksModel.count, tasksPerStripe())));
        }
    } else {
        return Math.floor(taskList.height / calculateStripes());
    }
}
