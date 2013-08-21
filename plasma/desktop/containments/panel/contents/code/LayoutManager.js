/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

.pragma library

var order = new Array();


var layout;
var root;
var plasmoid;


function restore() {
    var configString = String(plasmoid.configuration.AppletOrder)

    //array, a cell for encoded item orger
    var itemsStrings = configString.split(";");

    for (var i = 0; i < itemsStrings.length; i++) {
        
    }
}

function save() {
    plasmoid.configuration.AppletOrder = order.join(';');
}

function insertBefore(item1, item2) {
    if (item1 === item2) {
        return;
    }
    var removed = new Array();

    var child;

    for (var i = layout.children.length - 1; i >= 0; --i) {
        child = layout.children[i];
        removed.push(child);
        child.parent = root;

        if (child === item1) {
            break;
        }
    }

    item2.parent = layout;

    for (var j = removed.length - 1; j >= 0; --j) {
        removed[j].parent = layout;
    }
}

function insertAfter(item1, item2) {
    if (item1 === item2) {
        return;
    }
    var removed = new Array();

    var child;

    for (var i = layout.children.length - 1; i >= 0; --i) {
        child = layout.children[i];
        if (child === item1) {
            break;
        }

        removed.push(child);
        child.parent = root;
    }

    item2.parent = layout;

    for (var j = removed.length - 1; j >= 0; --j) {
        removed[j].parent = layout;
    }
}
