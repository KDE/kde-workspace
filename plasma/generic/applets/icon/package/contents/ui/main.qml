import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.icon 1.0

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
    
    Logic {
	id: logic
    }
    
    Connections {
	target: plasmoid
	onExternalData: {
	    print("************************************************************************************************");
	    print(data);
	    plasmoid.configuration.applicationName = logic.getName(data);
	    plasmoid.configuration.iconName = logic.getIcon(data);
	    print("************************************************************************************************");
	}
    }

}