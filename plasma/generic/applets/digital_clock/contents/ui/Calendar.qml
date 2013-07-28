import QtQuick 1.1
import org.kde.pim.calendar 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
import org.kde.plasma.extras 0.1 as PlasmaExtras
Item {
    id:root
    width: parent.width
    height: parent.height
    property string date ;
    property string date1;
    property int week;

    function isToday(date) {
        if(date==Qt.formatDateTime(new Date(), "dd/M/yyyy")) 
            return true ;
        else return false;
    }
    
    function isTodayMonth() {
        return Qt.formatDateTime(new Date(), "yyyy-MM-dd")
    }

    Calendar {
        id: monthComponent
        days: 7
        weeks: 6
        startDay:1
        sorting:Calendar.Ascending
        startDate: "2013-07-01"
        onStartDateChanged: {
            month.text=monthName
            year.text=year
        }
    }

    Row {
        anchors {
            fill:root
        }
        Column {
            id:col
            height: parent.height
            width: parent.width / 2
            anchors {
                top:parent.top
                bottom:parent.bottom
            }
            Rectangle {
                id:rect1
                clip:true
                height: parent.height/10
                width: parent.width
                color: "transparent"
                anchors {
                    right:grid.right
                }
                Row {
                    id:r1
                    Components.ToolButton {
                        flat: true;
                        text: "<";
                        width: 24;
                        height: 24;
                        id:monthright
                        onClicked: {
                            monthComponent.previousMonth()
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
                                        monthComponent.startDate="2013-01-01"
                                    }
                                }
                                Components.MenuItem {
                                    text:"February"
                                    onClicked: {
                                        monthComponent.startDate="2013-02-01"
                                    }
                                }
                                Components.MenuItem {
                                    text:"March"
                                    onClicked: {
                                        monthComponent.startDate="2013-03-01"
                                    }
                                }
                                Components.MenuItem {
                                    text:"April"
                                    onClicked: {
                                        monthComponent.startDate="2013-04-01"
                                    }
                                }
                                Components.MenuItem {
                                    text:"May"
                                    onClicked: {
                                        monthComponent.startDate="2013-05-01"
                                    }
                                }
                                Components.MenuItem {
                                    text:"June"
                                    onClicked: {
                                        monthComponent.startDate="2013-06-01"
                                    }
                                }
                                Components.MenuItem {
                                    text:"July"
                                    onClicked: {
                                        monthComponent.startDate="2013-07-01"
                                    }
                                }
                                Components.MenuItem {
                                    text:"August"
                                    onClicked: {
                                        monthComponent.startDate="2013-08-01"
                                    }
                                }
                                Components.MenuItem {
                                    text:"September" 
                                    onClicked: {
                                        monthComponent.startDate="2013-09-01"
                                    } 
                                }
                                Components.MenuItem {
                                    text:"October"
                                    onClicked: {
                                        monthComponent.startDate="2013-10-01"
                                    }
                                }
                                Components.MenuItem {
                                    text:"November"
                                    onClicked: {
                                        monthComponent.startDate="2013-11-01"
                                    }
                                }
                                Components.MenuItem {
                                    text:"December"
                                    onClicked: {
                                        monthComponent.startDate="2013-12-01"
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
                        anchors.leftMargin:rect1.height
                        anchors.right: rig.left
                        MouseArea {
                            id:mouse
                            anchors.fill: parent
                            onClicked: {
                                monthComponent.nextMonth()
                            }
                        }
                    }
                    Row {
                        id:r2
                        anchors {
                            top:monthright.bottom
                            left:parent.left
                            right:rig.left
                            bottom:grid.top
                            leftMargin:grid.width/5
                        }
                        clip:true
                        spacing:gv.cellWidth/1.5
                        Components.Label {
                            font.pointSize:rect1.height/8
                            text : monthComponent.dayName(7)
                        }
                        Components.Label {
                            font.pointSize:rect1.height/8
                            text:monthComponent.dayName(1)
                        }
                        Components.Label {
                            font.pointSize:rect1.height/8
                            text : monthComponent.dayName(2)

                        }
                        Components.Label {
                            font.pointSize:rect1.height/8
                            text : monthComponent.dayName(3)
                        }
                        Components.Label {
                            font.pointSize:rect1.height/8
                            text : monthComponent.dayName(4)
                        }
                        Components.Label {
                            font.pointSize:rect1.height/8
                            text : monthComponent.dayName(5)
                        }
                        Components.Label {
                            font.pointSize:rect1.height/8
                            text : monthComponent.dayName(6)
                        }
                    }
                }
            }
            Row {
                id:grid
                width:col.width
                height:parent.height-rect1.height-riw.height
                anchors {
                    top:rect.bottom
                    right:rig.left
                }
                ListView {
                    id:list_week
                    width: grid.width/8
                    height: parent.height
                    model: monthComponent.weeksModel
                    delegate: Rectangle {
                        id:r
                        width: grid.width/8
                        height: gv.cellHeight//grid.height / monthComponent.weeks
                        color: "transparent"
                        Components.Label {
                            id:weekNumber
                            anchors.centerIn: parent
                            text: modelData + 1
                            opacity:0.5
                        }
                        anchors {
                            top:header.bottom
                            right:gv.left
                        }
                    }
                }
                GridView {
                    id: gv
                    width: grid.width*7/8
                    height: parent.height
                    cellWidth:(grid.width*7/8)/monthComponent.days
                    cellHeight: (grid.height) / monthComponent.weeks
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    interactive:false
                    focus:true
                    model: monthComponent.model
                    property Item selectedItem
                    delegate: Rectangle {
                        id:myRectangle
                        width: gv.cellWidth
                        height: gv.cellHeight
                        color: "transparent"
                        Rectangle {
                            id:outer
                            width: gv.cellWidth-5
                            height: gv.cellHeight-5
                            color:(dateMouse.containsMouse)?"#eeeeee":"transparent"
                            border.color: gv.selectedItem == myRectangle ? "black" : "transparent"
                            Rectangle {
                                width: gv.cellWidth
                                height: gv.cellHeight
                                color:"transparent"
                                opacity:isToday(dayNumber+"/"+monthNumber+"/"+yearNumber)?true:false; 
                                anchors.fill:parent
                                border.color:"blue"
                            }
                            Components.Label {
                                id:label
                                anchors.centerIn: parent
                                text: dayNumber
                                font.bold:(containsEventItems)||(containsTodoItems) ? true:false
                                opacity: (isPreviousMonth || isNextMonth || dateMouse.containsMouse) ? 0.5 : 1.0
                            }
                            MouseArea {
                                id:dateMouse
                                anchors.fill:parent
                                hoverEnabled:true
                                onEntered: {
                                    monthComponent.setSelectedDay(yearNumber, monthNumber, dayNumber);                                     list.model=monthComponent.selectedDayModel
                                    if(list.count==0) {
                                       // error.visible=true
                                        list.model=monthComponent.upcomingEventsModel
                                    }
                                }
                                onClicked: {
                                    monthComponent.upcommingEventsFromDay(yearNumber, monthNumber, dayNumber);                                   gv.selectedItem=myRectangle
                                    var rowNumber = Math.floor(index / 7)   ;
                                    week=1+monthComponent.weeksModel[rowNumber];
                                    date=dayNumber+"/"+monthNumber+"/"+yearNumber
                                    date1=yearNumber+"-"+monthNumber+"-"+dayNumber
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
                       iconSource:"view-pim-calendar"
                        width:24
                        height:24
                          MouseArea {
                            id:mouse2
                            hoverEnabled:true
                            anchors.fill: parent
                            onClicked: {
                                monthComponent.startDate=isTodayMonth();
                            }
                            PlasmaCore.ToolTip {
                                id: tool
                                target: mouse2
                                mainText:"Select Today"
                            }
                        }
                    }
                    Components.TextField {
                        id:dateField
                        text: date
                        width:rect1.width/3
                    }
                    Components.TextField {
                        id:weekField
                        text:week
                        width:rect1.width/3
                    }
                }
            }
        }
        Column {
            id:rig
            height: parent.height
            width: parent.width / 2
            anchors.left:grid.right
            Rectangle {
                width:parent.width
                height:50
                color:"transparent"
                Components.Label {
                    id:error
                    text: "No event for this date"
                    visible:list.count==0
                    font.italic:true
                }
            }
            ListView {
                id:list
                height: parent.height
                width: (parent.width / 4)-(scrollBar.visible ? scrollBar.width : 0)
                model: monthComponent.upcomingEventsModel//monthComponent.selectedDayModel
                anchors.left:grid.right
                delegate: Rectangle {
                    width: parent.width-(scrollBar.visible ? scrollBar.width : 0)
                    height: 40
                    color: "transparent"
                    Row {
                        spacing:4
                        width: parent.width-(scrollBar.visible ? scrollBar.width : 0)
                        height:50
                        Rectangle {
                            id:circle1
                            width:6
                            height:6
                            color:"black"
                            border.width:1
                            radius:width*0.5
                            anchors.verticalCenter:incidence.verticalCenter
                        }
                        Rectangle {
                            id:incidence
                            width: parent.width-(scrollBar.visible ? scrollBar.width : 0)
                            height:20
                            color:"transparent"
                            Components.Label {
                                id:sum
                                text:  mimeType.split('.')[mimeType.split('.').length - 1]  + ": \n"+ summary 
                                font.capitalization:Font.Capitalize
                                font.italic:true
                                anchors {
                                    left:circle1.right
                                }
                            }
                        }
                    }
                }
                section.property:"startDate"
                section.delegate: Rectangle {
                    id:sect
                    width: parent.width-(scrollBar.visible ? scrollBar.width : 0)
                    height: 20
                    color: "transparent"
                    Components.Label {
                        id:sec_l
                        text: section
                        font.weight:Font.Bold
                        anchors.verticalCenter: sect.verticalCenter
                    }
                }
            }
            Components.ScrollBar {
                id: scrollBar
                orientation: Qt.Vertical
                flickableItem:list
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
            }
        }
    }
}
