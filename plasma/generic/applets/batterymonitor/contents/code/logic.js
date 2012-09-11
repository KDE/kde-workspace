
var ram = 0
var disk = 1

function updateCumulative() {
    var sum = 0;
    var haveBattery = false;
    var charged = true;
    for (var i=0; i<batteries.count; i++) {
        var b = batteries.get(i);
        if (b["Plugged in"]) {
            sum += b["Percent"];
            haveBattery = true;
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
    batteries.cumulativePluggedin = haveBattery;
    batteries.allCharged = charged;
}

function stringForState(batteryData) {
    var pluggedIn = batteryData["Plugged in"];
    var percent = batteryData["Percent"];
    var state = batteryData["State"];

    if (pluggedIn) {
        if (state == "NoCharge") {
            return i18n("%1% (charged)", percent);
        } else if (state == "Discharging") {
            return i18n("%1% (discharging)", percent);
        } else {//charging
            return i18n("%1% (charging)", percent);
        }
    }

    return i18nc("Battery is not plugged in", "Not present");
}

function updateTooltip() {
    var text="";
    for (var i=0; i<batteries.count; i++) {
        if (batteries.count == 1) {
            text += i18n("<b>Battery:</b>");
        } else {
            if (text != "") {
                text += "<br/>";
            }

            text += i18nc("tooltip: placeholder is the battery ID", "<b>Battery %1:</b>", i+1);
        }

        text += " ";
        text += stringForState(pmSource.data["Battery"+i]);
    }

    if (text != "") {
        text += "<br/>";
    }

    text += i18nc("tooltip", "<b>AC Adapter:</b>") + " ";
    text += pmSource.data["AC Adapter"]["Plugged in"] ? i18nc("tooltip", "Plugged in") : i18nc("tooltip", "Not plugged in");
    batteries.tooltipText = text;
}

function callForType(type) {
    if (type == ram) {
        return "suspendToRam";
    } else if (type == disk) {
        return "suspendToDisk";
    }

    return "suspendHybrid";
}
