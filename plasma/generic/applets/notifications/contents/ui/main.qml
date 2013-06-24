/***************************************************************************
 *   Copyright 2011 Davide Bettio <davide.bettio@kdemail.net>              *
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.locale 0.1 as KLocale

import "plasmapackage:/ui/uiproperties.js" as UiProperties


MouseEventListener {
    id: notificationsApplet
    state: "default"
    width: 32
    height: 32
    property int minimumWidth: mainScrollArea.implicitWidth
    property int minimumHeight: mainScrollArea.implicitHeight
    property int maximumWidth: -1
    property int maximumHeight: mainScrollArea.implicitHeight

    property int toolIconSize: UiProperties.toolIconSize
    property int layoutSpacing: UiProperties.layoutSpacing

    property real globalProgress: 0

    property bool showNotifications: false
    property bool showJobs: false

    property Item notifications: notificationsLoader.item
    property Item jobs: jobsLoader.item

    //notifications + jobs
    property int totalCount: (notifications ? notifications.count : 0) + (jobs ? jobs.count : 0)
    onTotalCountChanged: {
        if (totalCount > 0) {
            state = "new-notifications"
        } else {
            state = "default"
            plasmoid.hidePopup()
        }

        var data = new Object
        data["image"] = "preferences-desktop-notification"
        data["mainText"] = i18n("Notifications and Jobs")
        if (totalCount == 0) {
            data["subText"] = i18n("No notifications or jobs")
        } else if (!notifications.count) {
            data["subText"] = i18np("%1 running job", "%1 running jobs", jobs.count)
        } else if (!jobs.count) {
            data["subText"] = i18np("%1 notification", "%1 notifications", notifications.count)
        } else  {
            data["subText"] = i18np("%1 running job", "%1 running jobs", jobs.count) + "<br/>" + i18np("%1 notification", "%1 notifications", notifications.count)
        }
        plasmoid.popupIconToolTip = data
        plasmoid.passivePopup = jobs.count != 0
    }

    property Item notificationIcon

    Component.onCompleted: {
        //plasmoid.popupIcon = QIcon("preferences-desktop-notification")
        plasmoid.aspectRatioMode = "ConstrainedSquare"
        plasmoid.status = PassiveStatus
        allApplications = new Object
        plasmoid.addEventListener('ConfigChanged', configChanged);
        configChanged()
    }

    function configChanged()
    {
        showNotifications = plasmoid.readConfig("ShowNotifications")
        showJobs = plasmoid.readConfig("ShowJobs")
    }

    KLocale.Locale {
        id: locale
    }

    PlasmaCore.Svg {
        id: configIconsSvg
        imagePath: "widgets/configuration-icons"
    }

    property Component compactRepresentation: Component {
        NotificationIcon {
            id: notificationIcon
            Component.onCompleted: notificationsApplet.notificationIcon = notificationIcon
        }
    }

    hoverEnabled: !UiProperties.touchInput

    PlasmaExtras.ScrollArea {
        id: mainScrollArea
        anchors.fill: parent
        implicitWidth: theme.defaultFont.mSize.width * 40
        implicitHeight: Math.min(theme.defaultFont.mSize.height * 40, Math.max(theme.defaultFont.mSize.height * 6, contentsColumn.height))
        state: ""

        states: [
            State {
                name: "underMouse"
                when: notificationsApplet.containsMouse
                PropertyChanges {
                    target: mainScrollArea
                    implicitHeight: implicitHeight
                }
            },
            State {
                name: ""
                when: !notificationsApplet.containsMouse
                PropertyChanges {
                    target: mainScrollArea
                    implicitHeight: Math.min(theme.defaultFont.mSize.height * 40, Math.max(theme.defaultFont.mSize.height * 6, contentsColumn.height))
                }
            }
        ]

        Flickable {
            id: popupFlickable
            anchors.fill:parent

            contentWidth: width
            contentHeight: contentsColumn.height
            clip: true

            Column {
                id: contentsColumn
                width: popupFlickable.width

                //TODO: load those on demand based on configuration
                Loader {
                    id: jobsLoader
                    source: showJobs ? "Jobs.qml" : ""
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                }
                Loader {
                    id: notificationsLoader
                    source: showNotifications ? "Notifications.qml" : ""
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                }
            }
        }
    }
}
