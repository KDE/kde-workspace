/*
 * Copyright 2013 Heena Mahour <heena393@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
 * Copyright 2013 Martin Klapetek <mklapetek@kde.org>
 * Copyright 2014 David Edmundson <davidedmundson@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components

Item {
    id: main

    Layout.minimumWidth: sizehelper.paintedWidth + (units.smallSpacing * 2)
    Layout.maximumWidth: Layout.minimumWidth
    Layout.preferredWidth: Layout.minimumWidth

    Layout.minimumHeight: sizehelper.paintedHeight + (units.smallSpacing * 2)
    Layout.maximumHeight: Layout.minimumHeight
    Layout.preferredHeight: Layout.minimumHeight

    property int formFactor: plasmoid.formFactor
    property int timePixelSize: theme.defaultFont.pixelSize
    property int timezonePixelSize: theme.smallestFont.pixelSize

    property bool constrained: formFactor == PlasmaCore.Types.Vertical || formFactor == PlasmaCore.Types.Horizontal

    property bool vertical: plasmoid.formFactor == PlasmaCore.Types.Vertical

    property bool showSeconds: plasmoid.configuration.showSeconds
    property bool showTimezone: plasmoid.configuration.showTimezone
    property string timeFormat

    onShowSecondsChanged: {
        timeFormatCorrection(Qt.locale().timeFormat(Locale.ShortFormat))
    }
    onShowTimezoneChanged: {
        timeFormatCorrection(Qt.locale().timeFormat(Locale.ShortFormat))
    }

    onWidthChanged: geotimer.start()
    onHeightChanged: geotimer.start()

    Connections {
        target: plasmoid.configuration
        onBoldTextChanged: geotimer.start()
        onItalicTextChanged: geotimer.start()
    }

    Timer {
        id: geotimer
        interval: 4 // just to compress resize events of width and height; below 60fps
        onTriggered: updateSize()
    }

    Components.Label  {
        id: time
        font {
            weight: plasmoid.configuration.boldText ? Font.Bold : Font.Normal
            italic: plasmoid.configuration.italicText
            pixelSize: Math.min(main.width/6, main.height)
        }
        width: Math.max(paintedWidth, time.paintedWidth)
        text: Qt.formatTime(dataSource.data["Local"]["Time"], main.timeFormat);
        wrapMode: main.vertical ? Text.WordWrap : Text.NoWrap
        horizontalAlignment: main.vertical ? Text.AlignHCenter : Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            right: parent.right
            leftMargin: units.smallSpacing
            rightMargin: units.smallSpacing
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                plasmoid.expanded = !plasmoid.expanded;
                calTimer.start();
            }
            Timer {
                id: calTimer
                interval: 100
                onTriggered: {
                    if (calendarLoader.source == "") {
                        calendarLoader.source = "CalendarView.qml"
                    }
                }
            }
        }
    }

    Components.Label {
        id: sizehelper
        font.weight: time.font.weight
        font.italic: time.font.italic
        wrapMode: main.vertical ? Text.Wrap : Text.NoWrap
        visible: false
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            right: parent.right
            leftMargin: units.smallSpacing
            rightMargin: units.smallSpacing
        }
    }

    function updateSize() {
        // Start at minimum size, and increase the hidden label
        // size until we fit into the plasmoid with a bit of margin
        // It would be nice if we had proper font metrics in QML.
        var maxSize = theme.smallestFont.pixelSize;
        var threshold = theme.mSize(theme.defaultFont).height / 2;
        var stop = false;

        while (!stop) {
            maxSize = maxSize + 1;
            sizehelper.font.pixelSize = maxSize;
            var pw = sizehelper.paintedWidth;

            if (!constrained) { // free floating cases
                if (maxSize + threshold >= main.height) {
                    stop = true;
                } else if (pw + threshold >= main.width) {
                    stop = true;
                }
            } else { // panel situations
                if (main.vertical) {
                    if (pw + threshold >= main.width) {
                        stop = true;
                    }
                } else {
                    if (maxSize + threshold >= main.height) {
                        stop = true;
                    }
                }
            }
        }
        time.font.pixelSize = maxSize;
        sizehelper.font.timePixelSize = maxSize;

    }

    // Qt's QLocale does not offer any modular time creating like Klocale did
    // eg. no "gimme time with seconds" or "gimme time without seconds and with timezone".
    // QLocale supports only two formats - Long and Short. Long is unusable in many situations
    // and Short does not provide seconds. So if seconds are enabled, we need to add it here.
    //
    // What happens here is that it looks for the delimiter between "h" and "m", takes it
    // and appends it after "mm" and then appends "ss" for the seconds. Also it checks
    // if the format string already does not contain the seconds part.
    //
    // It can happen that Qt uses the 'C' locale (it's a fallback) and that locale
    // has always ":ss" part in ShortFormat, so we need to remove it.
    function timeFormatCorrection(timeFormatString) {
        if (main.showSeconds && timeFormatString.indexOf('s') == -1) {
            timeFormatString = timeFormatString.replace(/^(hh*)(.+)(mm)(.*?)/i,
                                                        function(match, firstPart, delimiter, secondPart, rest, offset, original) {
                return firstPart + delimiter + secondPart + delimiter + "ss" + rest
            });
        } else if (!main.showSeconds && timeFormatString.indexOf('s') != -1) {
            timeFormatString = timeFormatString.replace(/.ss?/i, "");
        }

        // We set the text of the sizehelper to a fixed-size string,
        // only depending on the length of the time format, but not on
        // the actually rendered time. This makes the width of the plasmoid
        // not change when the time changes, which can lead to relayouts of
        // the whole panel, and jumpiness.
        //
        // Also do this before appending the timezone part there
        // so we can simply add the full timezone length without
        // needing to subtract 1 for the "t"

        var stLength = timeFormatString.length;

        // Some locales have format like "h:mm", ie. without the leading zero.
        // That means that we reserve enough splace for say "9:31" but not enough
        // for "10:31". So if we have just single "h" in the format, we add 1
        // to compensate for the missing leading zero
        if (timeFormatString.search(/hh/i) == -1) {
            stLength++;
        }

        if (main.showTimezone) {
            stLength += Qt.formatTime(dataSource.data["Local"]["Time"], "t").length;
        }

        // If we're showing seconds, we have ":" twice in there. The colon char is
        // very slim, yet we add one "A" for each, which makes it too wide if seconds
        // are shown. So, if we're showing seconds, make it one "A" less as two colons
        // can fit the "A" width no problems
        if (main.showSeconds) {
            stLength--;
        }

        var st = new Array(stLength).join("A");

        if (sizehelper.text != st) {
            sizehelper.text = st;
        }

        //FIXME: this always appends the timezone part at the end, it should probably be
        //       Locale-driven, however QLocale does not provide any hint about where to
        //       put it
        if (main.showTimezone && timeFormatString.indexOf('t') == -1) {
            timeFormatString = timeFormatString + " t";
        }


        main.timeFormat = timeFormatString;
    }

    Component.onCompleted: {
        timeFormatCorrection(Qt.locale().timeFormat(Locale.ShortFormat))
    }
}
