import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Column {
    id: root

    clip: true

    PlasmaCore.IconItem {
        id: icon
        anchors.verticalCenter: parent
        width: theme.hugeIconSize
        height: width
        source: "utilities-terminal"
    }

    PlasmaComponents.Label {
        id: messageText
        anchors.verticalCenter: parent
        width: parent.width - icon.width
        wrapMode: Text.Wrap
        text: "Konsole"
    }

    Component.onCompleted: {
	//plasmoid.setBackgroundHints("NoBackground");
	print("************************************************************************************************");
	print(plasmoid.configuration.iconName);
	print(plasmoid.configuration.applicationName);
	print("************************************************************************************************");
    }

}