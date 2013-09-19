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

function activateNextPrevTask(parentItem, next) {
    var taskIdList;

    if (parentItem && parentItem.isGroupParent) {
         taskIdList = tasksModel.taskIdList(visualModel.modelIndex(parentItem.itemIndex));
    } else {
        taskIdList = tasksModel.taskIdList();
    }

    if (!taskIdList.length) {
        return;
    }

    var activeTaskId = tasksModel.activeTaskId();
    var target = taskIdList[0];

    for (var i = 0; i < taskIdList.length; ++i) {
        if (taskIdList[i] == activeTaskId)
        {
            if (next && i < (taskIdList.length - 1)) {
                target = taskIdList[i + 1];
            } else if (!next)
            {
                if (i) {
                    target = taskIdList[i - 1];
                } else {
                    target = taskIdList[taskIdList.length - 1];
                }
            }

            break;
        }
    }

    activateItem(target, false);
}

function insertionIndexAt(sourceIndex, x, y) {
    var above = target.childAt(x, y);

    if (above) {
        var index = above.itemIndex;

        if (index > sourceIndex) {
            ++index;
        }

        return index;
    } else {
        var distance = tasks.vertical ? x : y;
        var step = tasks.vertical ? Layout.taskWidth() : Layout.taskHeight();
        var stripe = Math.ceil(distance / step);

        if (stripe == Layout.calculateStripes()) {
            return -1;
        } else {
            return stripe * Layout.tasksPerStripe();
        }
    }
}

function publishIconGeometries(taskItems) {
    for (var i = 0; i < taskItems.length - 1; ++i) {
        var task = taskItems[i];

        if (task.isGroupParent) {
            var taskIdList = tasksModel.taskIdList(visualModel.modelIndex(task.itemIndex));

            for (j = 0; j < taskIdList.length; ++j) {
                tasks.itemGeometryChanged(taskIdList[j].itemId, task.x, task.y, task.width, task.height);
            }
        } else if (!task.isLauncher) {
            tasks.itemGeometryChanged(task.itemId, task.x, task.y, task.width, task.height);
        }
    }
}
