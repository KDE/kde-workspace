import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
import org.kde.qtextracomponents 0.1
import "data.js" as Data
import org.kde.dirmodel 0.1
Item {
    id:root
    property int minimumWidth:formFactor == Horizontal ? height : 1
    property int minimumHeight:formFactor == Vertical ? width  : 1
    property int formFactor: plasmoid.formFactor
    PlasmaCore.Theme {
        id: theme 
    }
    DirModel {
        id:dirModel
        url: "trash:/"
    }
    Connections {
        target: plasmoid
            onFormFactorChanged: {
                    root.formFactor = plasmoid.formFactor
            }
    }
    function action_open() {
        plasmoid.openUrl("trash:/");
    }
    function action_empty() {
        plasmoid.runCommand("ktrash", ["--empty"]);
    }
    Component.onCompleted: { 
        plasmoid.setBackgroundHints( 0 )
        plasmoid.action_open = function() {
            plasmoid.openUrl("trash:/");
        }
        plasmoid.setAction("open", i18n("Open"),"document-open");
        plasmoid.action_empty=function() {
        plasmoid.runCommand("ktrash", ["--empty"]);
        }
        plasmoid.setAction("empty",i18n("Empty"),"trash-empty");
        plasmoid.popupIcon = QIcon("user-trash");
        plasmoid.aspectRatioMode = IgnoreAspectRatio;
    }
    Row {
        id:row
        spacing:0
            PlasmaCore.IconItem {
                id:icon
                width:root.width
                height:width
                source: (dirModel.count > 0) ? "user-trash-full" : "user-trash"
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onReleased: plasmoid.openUrl("trash:/");
                }
            }
            Text {
                id:text
                color:theme.textColor
                font.bold:false
                font.pointSize:root.width/10
                text:"Trash \n"+dirModel.count
                anchors {
                    horizontalCenter:icon.horizontalCenter
                    top:icon.bottom
                }
            }
    }
}