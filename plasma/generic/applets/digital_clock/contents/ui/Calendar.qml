/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
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
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
import org.kde.locale 0.1

Rectangle {
    id: listWidget
    width: 300; height: 300
    visible:true
    color: "transparent"
    property date today: new Date()
    property date showDate: new Date()
    property int daysInMonth: new Date(showDate.getFullYear(), showDate.getMonth() + 1, 0).getDate()
    property int firstDay: new Date(showDate.getFullYear(), showDate.getMonth(), 1).getDay()
 
    Item {
        id:title
        anchors.top: parent.top
        anchors.right:parent.right
        anchors.left:parent.left
        anchors.topMargin: 10
        width: parent.width
        height: childrenRect.height 
       Image {
            id:monthright
            source: "previous.png"
            anchors.left: parent.left
            anchors.leftMargin: 10 
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    showDate = new Date(showDate.getFullYear(), showDate.getMonth(), 0)
                }
            }
        }
        Components.Label  {
            id:month
            text: Qt.formatDateTime(showDate, "MMMM")
            anchors.left:monthright.right
        }
        Image {
            source: "next.png"
            anchors.left:month.right
            MouseArea {
                anchors.fill: parent
                onClicked:
                {
                    showDate = new Date( showDate.getFullYear(), showDate.getMonth() + 1, 1)
                }
            }
        }
        Image {
            source: "previous.png"
            anchors.right: yearleft.left
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    showDate = new Date(showDate.getFullYear()-1,0)
                    console.log(showDate) 
                }
            }
        }
        Components.Label  {
            id:yearleft
            text: Qt.formatDateTime(showDate, "yyyy")
            anchors.right:year.left
        } 
        Image {
            id:year
            source: "next.png"
            anchors.right: parent.right
            anchors.rightMargin: 10 
            MouseArea {
                anchors.fill: parent
                onClicked:
                {
                    showDate = new Date(showDate.getFullYear()+1, 1)
                }
            }
        }
    } 
    function isToday(index) {
        if (today.getFullYear() != showDate.getFullYear())
            return false;
        if (today.getMonth() != showDate.getMonth())
            return false;
 
        return (index === today.getDate() - 1)
    } 
    Item {
        id: dateLabels
        anchors.top: title.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10 
        height: listWidget.height - title.height - 20 - title.anchors.topMargin
        property int rows: 6
        Item {
            id: dayLabels
            width: parent.width
            height: childrenRect.height 
            Grid {
                columns: 7
                spacing: 5 
                Repeater {
                    model: 7 
                    Rectangle {
                        color: "#00ffffff"
                        width: (listWidget.width - 20 - 60)/7
                        height: childrenRect.height
                        Components.Label  {
                            color: "#00000C"
                            text: Qt.formatDate(new Date(showDate.getFullYear(), showDate.getMonth(), index - firstDay +1), "ddd");
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }
        }
        Item {
            id: dateGrid
            width: parent.width
            anchors.top: dayLabels.bottom
            anchors.topMargin: 5
            anchors.bottom: parent.bottom
            property int currentActive: -1
            Grid {
                columns: 7
                rows: dateLabels.rows
                spacing: 10 
                Repeater {
                    id: repeater
                    model: firstDay + daysInMonth
                    Rectangle {
                        id:rect
                        property bool highLighted: false
                        property color normalColor
                        border.color:"transparent"
                        normalColor:"#eeeeee" 
                        Component.onCompleted: {
                            if (index < firstDay) {
                                normalColor = listWidget.color="transparent";
                            } else if(isToday(index-firstDay)) {
                                   border.color="blue"
                            }
                        }
                        color: normalColor
                        width: (listWidget.width - 20 - 60)/7
                        height: (dateGrid.height - (dateLabels.rows - 1)*10)/dateLabels.rows
                        Components.Label  {
                            id: dateText
                            anchors.centerIn: parent
                            text: index + 1 - firstDay
                            opacity: (index < firstDay) ? 0 : 1
                            font.bold: isToday(index - firstDay)  || highLighted
                        } 
                        MouseArea {
                            id: dateMouse
                            enabled: index >= firstDay
                            anchors.fill: parent
                            onClicked: {
                                rect.border.color="black"
                                var clickedDate = new Date( showDate.getFullYear(), showDate.getMonth() + 1, index + 1 - firstDay)
                                console.log(Qt.formatDate(clickedDate, "dd/MM/yyyy"))
                                if (dateGrid.currentActive != -1) {
                                    repeater.itemAt(dateGrid.currentActive).highLighted = false;
                                } 
                                if (!isToday(index - firstDay)){
                                    highLighted = true
                                    dateGrid.currentActive = index
                                }
                            }
                            onEntered : {
                                dateText.opacity=0.5
                            }
                            onExited: { 
                                dateText.opacity=1
                            }
                        }
                    }
                }
            }
        }
    }
}



