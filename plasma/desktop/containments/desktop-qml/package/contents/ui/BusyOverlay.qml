import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaComponents.BusyIndicator {
    id: busyIndicator
    z: appletContainer.z + 1
    visible: applet.busy
    running: visible
    anchors.centerIn: parent
}
