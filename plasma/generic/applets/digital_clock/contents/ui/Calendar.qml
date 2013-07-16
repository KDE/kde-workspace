import QtQuick 1.1
import org.kde.pim.calendar 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
Item {
    id:root
    width: parent.width
    height: parent.height
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
                    // anchors.centerIn: parent
                    Components.ToolButton {
                        flat: true;
                        text: "<";
                        width: 24;
                        height: 24;
                        id:monthright
                        anchors.left: col.left
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                monthComponent.previous()
                            }
                        }
                    }
                    Components.ToolButton {
                        id:month
                        width:50
                        height:24
                        anchors.leftMargin:30
                        anchors.rightMargin:20
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
                                }
                                Components.MenuItem {
                                    text:"February"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.next()
                                    }
                                }
                                Components.MenuItem {
                                    text:"March"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.next()
                                    }
                                }
                                Components.MenuItem {
                                    text:"April"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.next()
                                    }
                                }
                                Components.MenuItem {
                                    text:"May"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.next()
                                    }
                                }
                                Components.MenuItem {
                                    text:"June"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.next()
                                    }
                                }
                                Components.MenuItem {
                                    text:"July"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.next()
                                    }
                                }
                                Components.MenuItem {
                                    text:"August"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.next()
                                    }
                                }
                                Components.MenuItem {
                                    text:"September" 
                                    onClicked: {
                                        mouse.onClicked= monthComponent.next()
                                    } 
                                }
                                Components.MenuItem {
                                    text:"October"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.next()
                                    }
                                }
                                Components.MenuItem {
                                    text:"November"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.next()
                                    }
                                }
                                Components.MenuItem {
                                    text:"December"
                                    onClicked: {
                                        mouse.onClicked= monthComponent.next()
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
                        anchors.leftMargin:10
                        anchors.rightMargin:10
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
                        anchors.leftMargin:40
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
                            bottom:rect1.bottom
                            right:rect1.right
                        }
                        spacing :parent.width/24
                        Components.Label {
                            font.pointSize:rect1.width/30
                            text : monthComponent.dayName(7)
                            horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.width/30
                            text:monthComponent.dayName(1)
                            horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.width/30
                            text : monthComponent.dayName(2)
                            horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.width/30
                            text : monthComponent.dayName(3)
                            horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.width/30
                            text : monthComponent.dayName(4)
                            horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.width/30
                            text : monthComponent.dayName(5)
                            horizontalAlignment:Text.AlignHCenter
                        }
                        Components.Label {
                            font.pointSize:rect1.width/30
                            text : monthComponent.dayName(6)
                            horizontalAlignment:Text.AlignHCenter
                            anchors.right:rect1.right
                        }
                    }
                }
            }
            Rectangle {
                id:grid
                width:rect1.width
                height:parent.height-rect1.height
                color: "transparent"
                GridView {
                    id: gv
                    width: grid.width
                    height: grid.height/1.5
                    cellWidth: width / monthComponent.days
                    cellHeight: height / monthComponent.weeks
                    anchors {
                        fill:parent
                    }
                    highlight:highlight
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    highlightFollowsCurrentItem:false
                    focus:true
                    model: monthComponent.daysModel
                    delegate: Rectangle {
                        width: gv.cellWidth
                        height: gv.cellHeight
                        color: (containsEventItems) ? "purple" : "white"
                        border.color:"black"
                        Components.Label {
                            id:label
                            anchors.centerIn: parent
                            text: dayNumber
                            opacity: (isPreviousMonth || isNextMonth || dateMouse.containsMouse) ? 0.5 : 1.0
                        }
                        MouseArea {
                            id:dateMouse
                            anchors.fill:parent
                            hoverEnabled:true
                            onClicked:label.color="red"
                        }
                    }
                }
            }
        }
        Rectangle {
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
