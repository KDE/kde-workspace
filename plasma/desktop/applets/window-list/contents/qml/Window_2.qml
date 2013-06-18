import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts

QGraphicsWidget {
    id: listWidget
    property alias listView: tasks
    property int itemSpacing: 3
    height: tasks.height
    width: tasks.width
    ListView {
        id: tasks
        height: 12
        width: 300
        anchors.fill: parent
        model: PlasmaCore.DataModel { dataSource: tasksSource }
        delegate: listDelegate
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlight: Rectangle { color: "green"; radius: 5 }      
        focus: true
        clip: true
        spacing: listWidget.itemSpacing
    }
}

