/*
 *  Copyright 2013 Sebastian Kügler <sebas@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.private.systemtray 2.0 as SystemTray

Item {
    id: iconsPage
    width: childrenRect.width
    height: childrenRect.height
    implicitWidth: mainColumn.implicitWidth
    implicitHeight: pageColumn.implicitHeight

//     property alias cfg_Test: testConfigField.text
//     property alias cfg_BoolTest: testBoolConfigField.checked

    SystemTray.Host {
        id: host
    }

    Column {
        id: pageColumn
        anchors.fill: parent
        spacing: theme.defaultFont.pixelSize / 2
        PlasmaExtras.Title {
            text: i18n("SystemTray Settings")
        }
        ListView {
            model: host.categories
            width: parent.width
            height: theme.defaultFont.pixelSize * 10
            delegate: Row {
                height: implicitHeight
                width: parent.width
                QtControls.CheckBox {
                    id: categoryCheck
                    text: modelData
                }
            }

        }
        ListView {
            model: host.tasks
            width: parent.width
            height: 400

            spacing: parent.spacing

            delegate: Row {
                height: implicitHeight
                width: parent.width
                QtControls.Label {
                    text: name
                    elide: Text.ElideRight
                    width: parent.width / 3
                }
                QtControls.ComboBox {
                    currentIndex: 0
                    model: ListModel {
                        id: cbItems
                        ListElement { text: "Auto"; val: 1 }
                        ListElement { text: "Shown"; val: 2 }
                        ListElement { text: "Hidden"; val: 0 }
                    }
                    width: 200
                    onCurrentIndexChanged: console.debug(cbItems.get(currentIndex).text + ", " + cbItems.get(currentIndex).val)
                }
            }
        }
    }
}
