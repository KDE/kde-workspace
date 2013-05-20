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
    var count = 0;
    var charged = true;
    for (var i=0; i<batteries.count; i++) {
        var b = batteries.get(i);
        if (!b["Is Power Supply"]) {
          continue;
        }
        if (b["Plugged in"]) {
            sum += b["Percent"];
        }
        if (b["State"] != "NoCharge") {
            charged = false;
        }
        count++;
    }

    if (batteries.count > 0) {
        batteries.cumulativePercent = Math.round(sum/count);
    } else {
        batteries.cumulativePercent = 0;
    }
    batteries.allCharged = charged;
}

function stringForState(batteryData) {
    var pluggedIn = batteryData["Plugged in"];
    var percent = batteryData["Percent"];
    var state = batteryData["State"];
    var powerSupply = batteryData["Is Power Supply"];

    var text="<b>";
    if (pluggedIn) {
        // According to UPower spec, the chargeState is only valid for primary batteries
        if (powerSupply) {
            switch(state) {
                case "NoCharge": text += i18n("%1% (charged)", percent); break;
                case "Discharging": text += i18n("%1% (discharging)", percent); break;
                default: text += i18n("%1% (charging)", percent);
            }
        } else {
            text += i18n("%1%", percent);
        }
    } else {
        text += i18nc("Battery is not plugged in", "Not present");
    }
    text += "</b>";

    return text;
}

function updateTooltip() {
    var text="";

    for (var i=0; i<batteries.count; i++) {
        var b = batteries.get(i);
        if (text != "") {
            text += "<br/>";
        }

        text += b["Pretty Name"];

        text += ": ";
        text += stringForState(pmSource.data["Battery"+i]);
    }

    if (text != "") {
        text += "<br/>";
    }

    if (pmSource.data["AC Adapter"]) {
        text += i18nc("tooltip", "AC Adapter:") + " ";
        text += pmSource.data["AC Adapter"]["Plugged in"] ? i18nc("tooltip", "<b>Plugged in</b>") : i18nc("tooltip", "<b>Not plugged in</b>");
    }
    batteries.tooltipText = "<p style='white-space: nowrap'>" + text + "</p>";
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
