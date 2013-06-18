import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts
import org.kde.qtextracomponents 0.1 as QtExtra

Item {
    id: fontsi
    width: parent.width
    height: fontstyle.height + padding
    clip: true
    property string text
    property int padding : 6
    property int fontSize : 8
    property alias font: fontstyle.font
    property int interval : 40
    property color textColor: "black"
        Text {
	anchors.verticalCenter: parent.verticalCenter
	id: fontstyle
	font.pointSize: fontSize
	color: textColor
	text: parent.text
        }
}
