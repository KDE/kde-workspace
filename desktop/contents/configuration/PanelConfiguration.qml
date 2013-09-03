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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0
import org.kde.qtextracomponents 2.0 as QtExtras
import "panelconfiguration"


//TODO: all of this will be done with desktop components
PlasmaCore.FrameSvgItem {
    id: dialogRoot

//BEGIN Properties
    width: 640
    height: 64
    imagePath: "dialogs/background"

    state: {
        switch (panel.location) {
        //TopEdge
        case 3:
            return "TopEdge"
        //LeftEdge
        case 5:
            return "LeftEdge"
        //RightEdge
        case 6:
            return "RightEdge"
        //BottomEdge
        case 4:
        default:
            return "BottomEdge"
        }
    }

    property bool vertical: (panel.location == 5 || panel.location == 6)
//END properties

//BEGIN Connections
    Connections {
        target: panel
        onOffsetChanged: ruler.offset = panel.offset
        onMinimumLengthChanged: ruler.minimumLength = panel.minimumLength
        onMaximumLengthChanged: ruler.maximumLength = panel.maximumLength
    }
//END Connections


//BEGIN UI components

    Ruler {
        id: ruler
        state: dialogRoot.state
    }

    ToolBar {
        id: toolBar
    }
//END UI components

//BEGIN Animations
    //when EdgeHandle is released animate to old panel position
    ParallelAnimation {
        id: panelResetAnimation
        NumberAnimation {
            target: panel
            properties: (panel.location == 5 || panel.location == 6) ? "x" : "y"
            to:  {
                switch (panel.location) {
                //TopEdge
                case 3:
                    return 0
                    break
                //LeftEdge
                case 5:
                    return 0
                    break;
                //RightEdge
                case 6:
                    return panel.screenGeometry.x + panel.screenGeometry.width - panel.width
                    break;
                //BottomEdge
                case 4:
                default:
                    return panel.screenGeometry.y + panel.screenGeometry.height - panel.height
                }
            }
            duration: 150
        }

        NumberAnimation {
            target: configDialog
            properties: (panel.location == 5 || panel.location == 6) ? "x" : "y"
            to: {
                switch (panel.location) {
                //TopEdge
                case 3:
                    return panel.height
                    break
                //LeftEdge
                case 5:
                    return panel.width
                    break;
                //RightEdge
                case 6:
                    return panel.screenGeometry.x + panel.screenGeometry.width - panel.width - configDialog.width
                    break;
                //BottomEdge
                case 4:
                default:
                    return panel.screenGeometry.y + panel.screenGeometry.height - panel.height - configDialog.height
                }
            }
            duration: 150
        }
    }
//END Animations

//BEGIN States
states: [
        State {
            name: "TopEdge"
            PropertyChanges {
                target: dialogRoot
                enabledBorders: "TopBorder|BottomBorder"
            }
            PropertyChanges {
                target: dialogRoot
                implicitHeight: ruler.implicitHeight + toolBar.implicitHeight
            }
        },
        State {
            name: "BottomEdge"
            PropertyChanges {
                target: dialogRoot
                enabledBorders: "TopBorder|BottomBorder"
            }
            PropertyChanges {
                target: dialogRoot
                implicitHeight: ruler.implicitHeight + toolBar.implicitHeight
            }
        },
        State {
            name: "LeftEdge"
            PropertyChanges {
                target: dialogRoot
                enabledBorders: "LeftBorder|RightBorder"
            }
            PropertyChanges {
                target: dialogRoot
                implicitWidth: ruler.implicitWidth + toolBar.implicitWidth
            }
        },
        State {
            name: "RightEdge"
            PropertyChanges {
                target: dialogRoot
                enabledBorders: "LeftBorder|RightBorder"
            }
            PropertyChanges {
                target: dialogRoot
                implicitWidth: ruler.implicitWidth + toolBar.implicitWidth
            }
        }
    ]
//END States
}
