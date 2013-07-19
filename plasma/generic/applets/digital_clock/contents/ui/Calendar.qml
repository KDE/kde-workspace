import QtQuick 1.1
import org.kde.pim.calendar 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
Item {
    id:root
    width: parent.width
    height: parent.height
    property string date ;
     
    function isToday(date) {
        if(date==Qt.formatDateTime(new Date(), "dd/M/yyyy")) 
            return true ;
        else return false;
    } 
    CalendarData {
        id: calendar
        startDate: "2013-01-01"
        endDate: "2014-12-31"
        types: Calendar.Event | Calendar.Todo
    }
    Calendar {
        id: monthComponent
        days: 7
        weeks: 5
        startDay:1
        startDate: "2013-01-01"
    }
    Row {
        anchors {
            fill:root
        }
        Column {
            id:col
            height: rect.height
            width: parent.width / 2
            anchors {
                top:parent.top
                bottom:parent.bottom
            }
            Rectangle {
                id:rect1
                height: parent.height/7
                width: parent.width
                color: "transparent"
                Row {
                    anchors {
                        fill:parent
                    }
                    Components.ToolButton {
                        flat: true;
                        text: "<";
                        width: 24;
                        height: 24;
                        id:monthright
                        anchors.left: col.left
                        onClicked: {
                            monthComponent.previous()
                        }
                    }
                    Components.ToolButton {
                        id:month
                        width:50
                        height:24
                        anchors.leftMargin:rect1.width/7
                        anchors.rightMargin:rect1.width/3
                        anchors.left:monthright.right
                        text:monthComponent.monthName
                        onClicked: {
                            sectionScroll = sectionScrollComponent.createObject(month)
                            sectionScroll.open()
                        }
                        Component {
                            id:sectionScrollComponent
                            Components.ContextMenu {
                                id:sectionScroll
                                visualParent:month
                                Components.MenuItem {
                                    text:"January"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(1)
                                    }
                                }
                                Components.MenuItem {
                                    text:"February"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(2)
                                    }
                                }
                                Components.MenuItem {
                                    text:"March"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(3)
                                    }
                                }
                                Components.MenuItem {
                                    text:"April"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(4)
                                    }
                                }
                                Components.MenuItem {
                                    text:"May"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(5)
                                    }
                                }
                                Components.MenuItem {
                                    text:"June"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(6)
                                    }
                                }
                                Components.MenuItem {
                                    text:"July"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(7)
                                    }
                                }
                                Components.MenuItem {
                                    text:"August"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(8)
                                    }
                                }
                                Components.MenuItem {
                                    text:"September" 
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(9)
                                    } 
                                }
                                Components.MenuItem {
                                    text:"October"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(10)
                                    }
                                }
                                Components.MenuItem {
                                    text:"November"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(11)
                                    }
                                }
                                Components.MenuItem {
                                    text:"December"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.monthChanged(12)
                                    }
                                }
                            }
                        }
                    }
                    Components.ToolButton {
                        id:year
                        width:24
                        height:24
                        text:monthComponent.year
                        anchors.left:month.right
                        anchors.leftMargin:rect1.width/7
                        anchors.rightMargin:rect1.width/3
                        Components.ToolButton {
                            id:increase
                            text:"^"
                            width:12
                            height:12
                            anchors.left:year.right
                            MouseArea {
                                anchors.fill:parent
                                onClicked:monthComponent.nextYear()
                            }
                        }
                        Components.ToolButton {
                            id:decrease
                            text:"v"
                            width:12
                            height:12
                            anchors.left:year.right
                            anchors.top:increase.bottom
                            MouseArea {
                                anchors.fill:parent
                                onClicked:monthComponent.previousYear()
                            }
                        }
                    }
                    Components.ToolButton {
                        id:next1
                        flat: true;
                        text: ">";
                        width: 24;
                        height: 24;
                        anchors.left:year.right
                        anchors.leftMargin:rect1.height*2.6
                        anchors.right: col.right
                        MouseArea {
                            id:mouse
                            anchors.fill: parent
                            onClicked: {
                                monthComponent.next()
                            }
                        }
                    }
                    Row {
                        anchors {
                            top:monthright.bottom
                            left:parent.left
                            right:rect1.right
                            leftMargin:gv.cellWidth/2;
                        }
                        spacing:gv.cellWidth/2;
                        Components.Label {
                            font.pointSize:rect1.height/5
                            text : monthComponent.dayName(7)
                            horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.height/5
                            anchors.leftMargin:gv.cellWidth/2;
                            text:monthComponent.dayName(1)
                           horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.height/5
                            text : monthComponent.dayName(2)
                            horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.height/5
                            text : monthComponent.dayName(3)
                           horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.height/5
                            text : monthComponent.dayName(4)
                           horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.height/5
                            text : monthComponent.dayName(5)
                           horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.height/5
                            text : monthComponent.dayName(6)
                           horizontalAlignment:Text.AlignHCenter
                        }
                    }
                }
            }
            Rectangle {
                id:grid
                width:rect1.width
                height:parent.height-rect1.height-riw.height
                color: "transparent"
                    GridView {
                        id: gv
                        width: parent.width
                        height: parent.height
                        cellWidth: (grid.width )/ monthComponent.days
                        cellHeight: (grid.height) / monthComponent.weeks
                        anchors {
                            fill:parent
                        }
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        focus:true
                        model: monthComponent.daysModel
                        delegate: Rectangle {
                            width: gv.cellWidth
                            height: gv.cellHeight
                            color: (containsEventItems) ? "purple" : "transparent"
                            Rectangle {
                                id:outer
                                width: gv.cellWidth-5
                                height: gv.cellHeight-5
                                color:"transparent"
                                border.color:(dateMouse.containsMouse)?"black":"transparent"
                                Rectangle {
                                    width: gv.cellWidth
                                    height: gv.cellHeight
                                    color:"transparent"
                                    opacity:isToday(label.text+"/"+monthComponent.month()+"/"+monthComponent.year)?true:false   ;                                anchors.fill:parent
                                    border.color:"blue"
                                }
                                Components.Label {
                                    id:label
                                    anchors.centerIn: parent
                                    text: dayNumber
                                    font.bold:(containsEventItems) ? true:false
                                    opacity: (isPreviousMonth || isNextMonth || dateMouse.containsMouse) ? 0.5 : 1.0
                                }
                                MouseArea {
                                    id:dateMouse
                                    anchors.fill:parent
                                    hoverEnabled:true
                                    onClicked:{//tests here ----> 
                                        //   label.color="red"; 
                                        date=dayNumber+"/"+monthComponent.month()+"/"+monthComponent.year
                                    }
                                }
                            }
                        }
                    }
            }
            Rectangle {
                id:test
                width:parent.width
                height:20
                color:"transparent"
                Row {
                    id:riw
                    width:rect1.width
                    height:20
                    spacing:rect1.width/10
                    anchors {
                        left:test.left
                        right:test.right
                        verticalCenter:test.verticalCenter
                    }
                    Components.ToolButton {
                        id:currentDate
                        text:"#"
                        width:24
                        height:24
                    }
                    Components.TextField {
                        id:dateField
                        text: date
                        width:rect1.width/3
                    }
                    Components.TextField {
                        id:weekField
                        text:monthComponent.weekNumber(date);
                        width:rect1.width/3
                    }
                }
            }
        }
        Rectangle {
            id:rig
            height: parent.height
            width: parent.width / 4
            color: "transparent"
            anchors {
                right:parent.right
                left:col.right
            }
            ListView {
                id:list
                height: parent.height
                width: parent.width / 4
                model: calendar.model
                delegate: Rectangle {
                    width: parent.width/1.009
                    height: 25
                    color: "transparent"
                    Column {
                        anchors.fill: parent
                        Components.Label {
                            text: summary
                            font.bold: true
                            font.pointSize:rect1.width/30
                        }
                        Components.Label {
                            text: description
                            font.pointSize:rect1.width/30
                        }
                    }
                }
                anchors {
                    fill:parent
                }
                section.property: "mimeType"
                section.delegate: Rectangle {
                    id:sect
                    width: parent.width/1.009
                    height: 20
                    color: "grey"
                    Components.Label {
                        id:sec_l
                        text: section
                        font.pointSize:rect1.width/30
                        anchors.verticalCenter: sect.verticalCenter
                    }
                }
            }
        }
    }
}
