/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.containments 0.1 as PlasmaContainments

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

AppletAppearance {
    id: appletItem
    title: applet.name
    minimumWidth: Math.max(LayoutManager.cellSize.width,
                           appletContainer.minimumWidth +
                           appletItem.contents.anchors.leftMargin +
                           appletItem.contents.anchors.rightMargin)

    minimumHeight: Math.max(LayoutManager.cellSize.height,
                            appletContainer.minimumHeight +
                            appletItem.contents.anchors.topMargin +
                            appletItem.contents.anchors.bottomMargin)

    property alias applet: appletContainer.applet


    PlasmaContainments.AppletContainer {
        id: appletContainer
        //anchors.fill: appletItem.contents
        x: contents.x
        y: contents.y
        z: 500
        width: contents.width
        height: contents.height

        onAppletChanged: {
            applet.appletDestroyed.connect(appletDestroyed)
            appletTimer.running = true
        }
        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }
        function appletDestroyed() {
            LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)
            appletItem.destroy()
        }
    }

    //FIXME: this delay is because backgroundHints gets updated only after a while in qml applets
    Timer {
        id: appletTimer
        interval: 250
        repeat: false
        running: false
        onTriggered: updateBackgroundHints()
    }

    function updateBackgroundHints() {
            print(" %%%%%% Updating backgroundHints..." + applet.backgroundHints);
            hasBackground = (applet.backgroundHints != "NoBackground");
            if (applet.backgroundHints == -1) {
                appletItem.imagePath = "widgets/background";
                backgroundHints = "StandardBackground";
            } else if (applet.backgroundHints == 2) {
                appletItem.imagePath = "widgets/translucentbackground"
                backgroundHints = "TranslucentBackground";
            } else if (applet.backgroundHints == 0) {
                //appletItem.imagePath = "widgets/translucentbackground"
                backgroundHints = "NoBackground";
            } else {
                backgroundHints = "DefaultBackground";
                appletItem.imagePath = "widgets/background";
            }
            print(" @@@@@@@ Background: " + applet.name + " " + applet.backgroundHints + " " + hasBackground + " " + appletItem.imagePath);
            print(" @@@@ hints saved as " + backgroundHints);
            applet.backgroundHints = "NoBackground";
    }
}
