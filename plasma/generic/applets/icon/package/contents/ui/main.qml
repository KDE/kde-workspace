import QtQuick 1.0
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
 
Item {

    width: 100
    height: 100
    
    property string icon;
    property string text;

    PlasmaWidgets.IconWidget {
        id: icon
        text: "Konsole"
        Component.onCompleted: setIcon("utilities-terminal")
        anchors.fill: parent
    }

/*    Text {
	id: label
	text: "Konsole"
	anchors.left: parent.left
	anchors.right: parent.right
	anchors.bottom: parent.bottom
	horizontalAlignment: Text.AlignHCenter
    }*/

    Component.onCompleted: {
	plasmoid.setBackgroundHints("NoBackground");
	print(plasmoid.backgroundHints);
    }

}