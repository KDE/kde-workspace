/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.components 0.1 as PlasmaComponents

Column {
    id: root
    property alias cfg_Color: picker.color

    PlasmaComponents.Label {
        text: "Color:"
    }
    Column {
        id: picker

        property color color

        property real hue
        property real saturation
        property real lightness

        onColorChanged: {
            var min = Math.min(color.r, Math.min(color.g, color.b))
            var max = Math.max(color.r, Math.max(color.g, color.b))
            var c = max - min
            var h

            if (c == 0) {
                h = 0
            } else if (max == color.r) {
                h = ((color.g - color.b) / c) % 6
            } else if (max == color.g) {
                h = ((color.b - color.r) / c) + 2
            } else if (max == color.b) {
                h = ((color.r - color.g) / c) + 4
            }
            picker.hue = (1/6) * h
            picker.saturation = c / (1 - Math.abs(2 * ((max+min)/2) - 1))
            picker.lightness = (max + min)/2
        }
        onHueChanged: redrawTimer.restart()
        onSaturationChanged: redrawTimer.restart()
        onLightnessChanged: redrawTimer.restart()
        Timer {
            id: redrawTimer
            interval: 10
            onTriggered: {
                hsCanvas.requestPaint();
                vCanvas.requestPaint();
                hsMarker.x = Math.round(hsCanvas.width*picker.hue - 2)
                hsMarker.y = Math.round(hsCanvas.width*(1-picker.saturation) - 2)
                vMarker.y = Math.round(hsCanvas.height*(1-picker.lightness) - 2)

                //this to work assumes the above rgb->hsl conversion is correct
                picker.color = Qt.hsla(picker.hue, picker.saturation, picker.lightness, 1)
            }
        }
        Rectangle {
            /*anchors {
                left: parent.left
                right: parent.right
            }*/
            width: 200
            height: 30
            color: Qt.hsla(picker.hue, picker.saturation, picker.lightness, 1)
            MouseArea {
                anchors.fill: parent
                onClicked: pickerRow.height = (pickerRow.height == 0 ? pickerRow.implicitHeight : 0)
            }
        }
        Row {
            id: pickerRow
            clip: true
            height: 0
            Behavior on height {
                NumberAnimation {
                    duration: 250
                    easing.type: "InOutQuad"
                }
            }
            MouseArea {
                width: 255
                height: 255
                onPressed: {
                    hsMarker.x = mouse.x - 2
                    hsMarker.y = mouse.y - 2
                    picker.hue = mouse.x/width
                    picker.saturation = 1 - hsMarker.y/height
                }
                onPositionChanged: {
                    hsMarker.x = mouse.x - 2
                    hsMarker.y = mouse.y - 2
                    picker.hue = mouse.x/width
                    picker.saturation = 1 - hsMarker.y/height
                }
                Canvas {
                    id: hsCanvas
                    anchors.fill: parent
                    onPaint: {
                        var ctx = getContext('2d');
                        var gradient = ctx.createLinearGradient(0, 0, width, 0)
                        for (var i = 0; i < 10; ++i) {
                            gradient.addColorStop(i/10, Qt.hsla(i/10, 1, picker.lightness, 1));
                        }

                        ctx.fillStyle = gradient
                        ctx.fillRect(0, 0, width, height);

                        gradient = ctx.createLinearGradient(0, 0, 0, height)
                        gradient.addColorStop(0, Qt.hsla(0, 0, picker.lightness, 0));
                        gradient.addColorStop(1, Qt.hsla(0, 0, picker.lightness, 1));

                        ctx.fillStyle = gradient
                        ctx.fillRect(0, 0, width, height);
                    }
                }
                Rectangle {
                    id: hsMarker
                    width: 5
                    height: 5
                    radius: 5
                    color: "black"
                    border {
                        color: "white"
                        width: 1
                    }
                }
            }
            MouseArea {
                width: 30
                height: 255
                onPressed: {
                    vMarker.y = mouse.y - 2
                    picker.lightness = 1 - vMarker.y/height
                }
                onPositionChanged: {
                    vMarker.y = mouse.y - 2
                    picker.lightness = 1 - vMarker.y/height
                }
                Canvas {
                    id: vCanvas
                    width: 30
                    height: 255
                    onPaint: {
                        var ctx = getContext('2d');
                        var gradient = ctx.createLinearGradient(0, 0, 0, height)
                        gradient.addColorStop(0, Qt.hsla(picker.hue, picker.saturation, 1, 1));
                        gradient.addColorStop(0.5, Qt.hsla(picker.hue, picker.saturation, 0.5, 1));
                        gradient.addColorStop(1, Qt.hsla(picker.hue, picker.saturation, 0, 1));

                        ctx.fillStyle = gradient
                        ctx.fillRect(0, 0, width, height);
                    }
                }
                Rectangle {
                    id: vMarker
                    width: 30
                    height: 5
                    radius: 5
                    color: "black"
                    border {
                        color: "white"
                        width: 1
                    }
                }
            }
        }
    }
}