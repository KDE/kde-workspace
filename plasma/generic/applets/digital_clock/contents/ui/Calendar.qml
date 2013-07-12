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
import org.kde.locale 0.1 as Locale

Rectangle {
    id: listWidget
    width: 300; height: 300
    visible:true
    color: "transparent"
    property date today: new Date()
    property date showDate: new Date()
    property int daysInMonth: new Date(showDate.getFullYear(), showDate.getMonth() + 1, 0).getDate()
    property int firstDay: new Date(showDate.getFullYear(), showDate.getMonth(), 1).getDay()
    property date clickedDate: new Date()
    property int weeknumber:( Qt.formatDateTime(clickedDate, "dd")/8)+1

    Item {
        id:title
        anchors.top: parent.top
        anchors.right:parent.right
        anchors.left:parent.left
        anchors.topMargin: 5
        width: parent.width
        height: childrenRect.height 
        Components.ToolButton {
        flat: true;
        text: "<";
        width: 24;
        height: 24;
        id:monthright
        anchors.left: parent.left
        MouseArea {
            anchors.fill: parent
            onClicked: {
                    showDate = new Date(showDate.getFullYear(), showDate.getMonth(), 0)
                }
            }
        }
        Components.ToolButton {
            id:month
            width:24
            height:24
            anchors.left:monthright.right
            anchors.leftMargin:70
            text:Qt.formatDateTime(showDate, "MMMM")
            onClicked: {
                sectionScroll = sectionScrollComponent.createObject(month)
                sectionScroll.open()
            }
            Components.Label {
                text:Qt.formatDateTime(showDate, "M")
                id:presentmonth
                visible:false
            }
            Component {
                id: sectionScrollComponent
                Components.ContextMenu {
                    id: sectionScroll
                    visualParent:month
                    Components.MenuItem {
                        text:"January"
                        onClicked:{//Made an algorithm to find out month wise date as shown below ---> 
                            showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+1 , -(presentmonth.text-1));                                        close ();
                        }
                    }
                    Components.MenuItem {
                        text:"February"
                        onClicked:{ 
                            showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+2, -(presentmonth.text-1)+1);
                            close ();
                        }
                    }
                    Components.MenuItem {
                        text:"March"
                        onClicked: { 
                            showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+3, -(presentmonth.text-1)+2);                                        close ();
                        }
                    }
                    Components.MenuItem {
                        text:"April"
                        onClicked: {
                            showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+4 , -(presentmonth.text-1)+3);                                        close ();
                        }
                    }
                    Components.MenuItem {
                        text:"May"
                        onClicked: { 
                            showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+5 , -(presentmonth.text-1)+4)
                            close ()
                        }
                    }
                    Components.MenuItem {
                        text: "June"
                        onClicked: { 
                            showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+6, -(presentmonth.text-1)+5)
                            close ()
                        }
                    }
                    Components.MenuItem {
                        text:"July"
                        onClicked: { 
                            showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+7 , -(presentmonth.text-1)+6)
                            close ()
                        }
                    }
                    Components.MenuItem {
                        text:"August"
                        onClicked: { 
                            showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+8, -(presentmonth.text-1)+7)
                            close ()
                        }
                    }
                    Components.MenuItem {
                        text: "September"
                        onClicked: { 
                            showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+9 , -(presentmonth.text-1)+8)
                            close ()
                        }
                    }
                    Components.MenuItem {
                        text:"October"
                        onClicked: { 
                                        showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+10 , -(presentmonth.text-1)+9)
                                        close ()
                                    }
                    }
                    Components.MenuItem {
                        text:"November"
                        onClicked: { 
                                        showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+11 , -(presentmonth.text-1)+10)
                                        close ()
                                    }
                    }
                    Components.MenuItem {
                        text:"December"
                        onClicked: { 
                                        showDate = new Date( showDate.getFullYear(), showDate.getMonth()-(presentmonth.text-1)+12, -(presentmonth.text-1)+11)
                                        close ()
                                    }
                    }
                }
            }
        }
        /* Image {
            id:n
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
                }
            }
        }*/
        Components.TextField{
            id:yearleft
            text: Qt.formatDateTime(showDate, "yyyy")
            width:45
            height:24
            anchors.left:month.right
            anchors.leftMargin:10
            Components.ToolButton {
                id:increase
                text:"^"
                width:12
                height:12
                anchors.left:yearleft.right
                MouseArea {
                    anchors.fill:parent
                    onClicked:
                    {
                         showDate = new Date(showDate.getFullYear()+1,showDate.getMonth(),1)
                    }
                }
            }
            Components.ToolButton {
                id:decrease
                text:"v"
                width:12
                height:12
                anchors.left:yearleft.right
                anchors.top:increase.bottom
                MouseArea {
                    anchors.fill:parent
                    onClicked:
                    {
                         showDate = new Date(showDate.getFullYear()-1,0)
                    }
                }
            }
        }
        Components.ToolButton {
            flat: true;
            text: ">";
            width: 24;
            height: 24;
            id:year
            anchors.right: parent.right
            MouseArea {
                anchors.fill: parent
                onClicked:
                {
                    showDate = new Date(showDate.getFullYear(),showDate.getMonth() + 1, 1)
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
        property int rows: 7
        Item {
            id: dayLabels
            width: parent.width
            height: childrenRect.height 
            Grid {
                columns: 7
                spacing: 10
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
        Components.ToolButton {
            id: currentDateButton;
            text: "#";
            width: 24;
            height: 24;
            anchors {
                left: parent.left;
                bottom: parent.bottom;
            }
        }
        Components.TextField {
            id: dateField;
            text:Qt.formatDate(clickedDate, "dd/MM/yyyy")
            anchors {
                left: currentDateButton.right;
                bottom: parent.bottom;
            }
        }
        Components.TextField {
            id: weekField;
            text:weeknumber
            anchors {
                left: dateField.right;
                right: parent.right;
                bottom: parent.bottom;
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
                columns: 8
                rows: dateLabels.rows
                spacing: 10 
                Repeater {
                    id: repeater
                    model: firstDay + daysInMonth
                    Rectangle {
                        id:rect
                        property bool highLighted: false
                        property color normalColor
                        border.color:dateMouse.containsMouse?"black":"transparent"
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
                            opacity: (index < firstDay) ? 0.5 : 1
                           // font.bold: isToday(index - firstDay)  || highLighted
                        } 
                        MouseArea {
                            id: dateMouse
                            enabled: index >= firstDay
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                             //  rect.border.color=isToday(index-firstDay)?"blue":"black"
                                clickedDate = new Date( showDate.getFullYear(), showDate.getMonth() + 1, index + 1 - firstDay)
                                console.log(Qt.formatDate(clickedDate, "dd/MM/yyyy"))
                                if (dateGrid.currentActive != -1) {
                                    repeater.itemAt(dateGrid.currentActive).highLighted = false;
                                } 
                                if (!isToday(index - firstDay)){
                                    highLighted = true
                                    dateGrid.currentActive = index
                                }
                            }
                          /*  onPressed : {
                                rect.border.color="black"
                            }
                            onDoubleClicked: {
                                rect.border.color="transparent"
                            }*/
                            onEntered : {
                                dateText.opacity=0.5
                                // rect.border.color="black"
                            }
                            onExited: { 
                                dateText.opacity=1
                              //     rect.border.color="transparent"
                              //  rect.border.color=rect.border.color==black?"transparent":"blue"
                            }
                        }
                    }
                }
            }
        }
    }
}



