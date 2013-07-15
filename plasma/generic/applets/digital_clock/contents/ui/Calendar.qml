import QtQuick 1.1
import org.kde.pim.calendar 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
Item {
    id:root
    width: 700
    height: 360
    Rectangle {
        id:rect
        width: 700
        height: 360
        
        CalendarData {
        id: calendar
        startDate: "2012-01-01"
        endDate: "2013-12-31"
        types: Calendar.Event | Calendar.Todo
        }
        
        Calendar {
            id: monthComponent
            days: 7
            weeks: 6
            startDay:7
            startDate: "2013-01-01"
        }
        
        Row {
            anchors.fill: parent
            Column {
                id:col
                height: parent.height
                width: parent.width / 2
                Rectangle {
                    id:rect1
                    height: 100
                    width: parent.width
                    color: "red"
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
                            anchors.leftMargin:80
                            anchors.rightMargin:50
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
                                    onClicked:monthComponent.nextyear()
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
                                    onClicked:monthComponent.previousyear()
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
                            anchors.leftMargin:130
                            anchors.right: col.right
                            MouseArea {
                                id:mouse
                                anchors.fill: parent
                                onClicked: {
                                    monthComponent.next()
                                }
                            }
                        }
                    }
                }
                GridView {
                    id: gv
                    width: parent.width
                    height: parent.height - 100
                    cellWidth: width / monthComponent.days
                    cellHeight: height / monthComponent.weeks
                    model: monthComponent.daysModel
                    delegate: Rectangle {
                        width: gv.cellWidth
                        height: gv.cellHeight
                        color: (containsEventItems) ? "purple" : "white"
                        Text {
                            anchors.centerIn: parent
                            text: dayNumber
                            opacity: (isPreviousMonth || isNextMonth) ? 0.5 : 1.0
                        }
                    }
                }
            }
            
            ListView {
                height: parent.height
                width: parent.width / 2
                model: calendar.model
                delegate: Rectangle {
                    width: 360
                    height: 50
                    color: "lightgray"
                    Column {
                        anchors.fill: parent
                        Text {
                            text: summary
                            font.bold: true
                            color: "white"
                        }
                    Text {
                        text: description
                        color: "white"
                    }
                }
            }

            section.property: "mimeType"
            section.delegate: Rectangle {
                width: 360
                height: 20
                color: "lightblue"
                Text {
                    text: section
                }

            }
        }
    }
}

}
