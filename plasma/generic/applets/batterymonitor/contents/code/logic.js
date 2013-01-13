/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2012 Viranch Mehta <viranch.mehta@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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

var ram = 0
var disk = 1

function updateCumulative() {
    var sum = 0;
    var charged = true;
    for (var i=0; i<batteries.count; i++) {
        var b = batteries.get(i);
        if (b["Plugged in"]) {
            sum += b["Percent"];
        }
        if (b["State"] != "NoCharge") {
            charged = false;
        }
    }

    if (batteries.count > 0) {
        batteries.cumulativePercent = Math.round(sum/batteries.count);
    } else {
        batteries.cumulativePercent = 0;
    }
    batteries.allCharged = charged;
}

function stringForState(batteryData) {
    var pluggedIn = batteryData["Plugged in"];
    var percent = batteryData["Percent"];
    var state = batteryData["State"];

    if (pluggedIn) {
        if (state == "NoCharge") {
            return i18n("<b>%1% (charged)</b>", percent);
        } else if (state == "Discharging") {
            return i18n("<b>%1% (discharging)</b>", percent);
        } else {//charging
            return i18n("<b>%1% (charging)</b>", percent);
        }
    }

    return i18nc("Battery is not plugged in", "<b>Not present</b>");
}

function updateTooltip() {
    var text="";
    for (var i=0; i<batteries.count; i++) {
        if (batteries.count == 1) {
            text += i18n("Battery:");
        } else {
            if (text != "") {
                text += "<br/>";
            }

            text += i18nc("tooltip: placeholder is the battery ID", "Battery %1:", i+1);
        }

        text += " ";
        text += stringForState(pmSource.data["Battery"+i]);
    }

    if (text != "") {
        text += "<br/>";
    }

    if (pmSource.data["AC Adapter"]) {
        text += i18nc("tooltip", "AC Adapter:") + " ";
        text += pmSource.data["AC Adapter"]["Plugged in"] ? i18nc("tooltip", "<b>Plugged in</b>") : i18nc("tooltip", "<b>Not plugged in</b>");
    }
    batteries.tooltipText = text;
}

function updateBrightness() {
    // we don't want passive brightness change send setBrightness call
    if (!pmSource.data["PowerDevil"]) {
        return;
    }
    dialogItem.disableBrightnessUpdate = true;
    dialogItem.screenBrightness = pmSource.data["PowerDevil"]["Screen Brightness"];
    dialogItem.disableBrightnessUpdate = false;
}

function callForType(type) {
    if (type == ram) {
        return "suspendToRam";
    } else if (type == disk) {
        return "suspendToDisk";
    }

    return "suspendHybrid";
}
