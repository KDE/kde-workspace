import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Column {
    id: root

    clip: true

    PlasmaCore.IconItem {
        id: icon
        anchors.horizontalCenter: parent
        width: theme.hugeIconSize
        height: width
        source: plasmoid.configuration.iconName
    }

    PlasmaComponents.Label {
        id: label
        anchors.horizontalCenter: parent
        width: parent.width - icon.width
        wrapMode: Text.Wrap
        text: plasmoid.configuration.applicationName
    }
    
    Connections {
	target: plasmoid
	onExternalData: {
	    print("************************************************************************************************");
	    print(data);
	    print("************************************************************************************************");
	}
    }

}