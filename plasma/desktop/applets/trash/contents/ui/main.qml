import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
import org.kde.qtextracomponents 0.1
import "data.js" as Data
import org.kde.dirmodel 0.1
Column {
    id:trash
    property int minimumWidth
    property int minimumHeight
    property int orientation: Qt.Vertical
    PlasmaCore.Theme {
        id: theme 
    }
    DirModel {
        id:dirModel
        url: "trash:/"
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
    Item {
        id:item
        visible: true
        width: 100
        height: 100
            PlasmaCore.IconItem {
                id:iconButton
                width:100
                height:100
                anchors.centerIn:parent
                source: (dirModel.count > 0) ? "user-trash-full" : "user-trash"
	        Text { 
                    id:text
                    anchors.top:iconButton.bottom
                    anchors.centerIn:parent
                    text:"Trash \n " + dirModel.count
                }
            }
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onReleased: plasmoid.openUrl("trash:/");
            PlasmaCore.ToolTip {
                target: mouseArea
                mainText:"trash"
                subText: dirModel.count
                }
            Connections {
                target: plasmoid
                onFormFactorChanged: {
                    plasmoid.formFactor()
                }
            }
        }
    } 
}
 


