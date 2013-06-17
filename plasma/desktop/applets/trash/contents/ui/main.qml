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
    property int minButtonSize: 16
    property bool show_trash: true
    property int visibleButtons: 1
    property int orientation: Qt.Vertical
    onWidthChanged: checkLayout();
    onHeightChanged: checkLayout();    
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
        plasmoid.addEventListener("ConfigChanged", configChanged);
    }
    function checkLayout() {
        switch(plasmoid.formFactor) {
            case Vertical:
                if (width >= minButtonSize*visibleButtons) {
                    orientation = Qt.Horizontal;
                    minimumHeight = width/visibleButtons - 1;
                    plasmoid.setPreferredSize(width, width/visibleButtons);
                } 
                else {
                    orientation = Qt.Vertical;
                    minimumHeight = width*visibleButtons;
                    plasmoid.setPreferredSize(width, width*visibleButtons);
                }
                break;
            case Horizontal:
                if (height < minButtonSize*visibleButtons) {
                    orientation = Qt.Horizontal;
                    minimumWidth = height*visibleButtons;
                    plasmoid.setPreferredSize(height*visibleButtons, height);
                } 
                else {
                    orientation = Qt.Vertical;
                    minimumWidth = height/visibleButtons - 1;
                    plasmoid.setPreferredSize(height/visibleButtons, height);
                }
                break;
            default:
                if (width > height) {
                    orientation = Qt.Horizontal;
                    minimumWidth = minButtonSize*visibleButtons;
                    minimumHeight = minButtonSize;
                }
                else {
                    orientation = Qt.Vertical;
                    minimumWidth = minButtonSize;
                    minimumHeight = minButtonSize*visibleButtons;
                }
                break;
        }
    }
    function configChanged() {
        show_trash = plasmoid.readConfig("show_trash");
        visibleButtons = show_trash
        showModel.get(0).show = show_trash;
        checkLayout();
     }
    
    Item {
        id:item    
        visible: showModel.get(index).show
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
                mainText: modelData.tooltip_mainText
                subText: modelData.tooltip_subText
                image: modelData.icon
            }
        }
    } 
    
}
 


