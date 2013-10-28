/*
 *   Copyright 2012 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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
import org.kde.plasma.components 0.1 as PlasmaComponents

import "plasmapackage:/ui/NotificationDelegate"

Column {
    id: notificationsRoot
    property alias count: notificationsRepeater.count
    anchors {
        left: parent.left
        right: parent.right
    }

    function addNotification(source, appIcon, image, appName, summary, body, expireTimeout, urgency, appRealName, configurable, actions) {
        // Do not show duplicated notifications
        for (var i = 0; i < notificationsModel.count; ++i) {
            if (notificationsModel.get(i).source == source &&
                notificationsModel.get(i).appName == appName &&
                notificationsModel.get(i).summary == summary &&
                notificationsModel.get(i).body == body) {
                return
            }
        }

        for (var i = 0; i < notificationsModel.count; ++i) {
            if (notificationsModel.get(i).source == source) {
                notificationsModel.remove(i)
                break
            }
        }
        if (notificationsModel.count > 20) {
            notificationsModel.remove(notificationsModel.count-1)
        }
        var notification = {"source"  : source,
                "appIcon" : appIcon,
                "image"   : image,
                "appName" : appName,
                "summary" : summary,
                "body"    : body,
                "expireTimeout": expireTimeout,
                "urgency" : urgency,
                "configurable": configurable,
                "appRealName": appRealName,
                "actions" : actions}
        notificationsModel.inserting = true;
        notificationsModel.insert(0, notification);
        notificationsModel.inserting = false;
        if (plasmoid.popupShowing) {
            return
        }
        if (!lastNotificationPopup) {
            lastNotificationPopup = lastNotificationPopupComponent.createObject(notificationsRoot)
        }
        lastNotificationPopup.popup(notification)
    }

    function executeAction(source, id) {
        //try to use the service
        if (source.indexOf("notification") !== -1) {
            var service = notificationsSource.serviceForSource(source)
            var op = service.operationDescription("invokeAction")
            op["actionId"] = id

            service.startOperationCall(op)
        //try to open the id as url
        } else if (source.indexOf("Job") !== -1) {
            plasmoid.openUrl(id)
        }
    }

    function configureNotification(appRealName) {
      var service = notificationsSource.serviceForSource("notification")
      var op = service.operationDescription("configureNotification")
      op["appRealName"] = appRealName;
      service.startOperationCall(op)
    }

    function closeNotification(source) {
      var service = notificationsSource.serviceForSource(source)
      var op = service.operationDescription("userClosed")
      service.startOperationCall(op)
    }

    property QtObject lastNotificationPopup
    Component {
        id: lastNotificationPopupComponent
        LastNotificationPopup {
        }
    }

    ListModel {
        id: notificationsModel
        property bool inserting: false;
    }
    ListModel {
        id: allApplicationsModel
        function addApplication(icon, name)
        {
            for (var i = 0; i < count; ++i) {
                var item = get(i)
                if (item.name == name) {
                    setProperty(i, "count", item.count + 1)
                    return
                }
            }
            append({"icon": icon, "name": name, "count": 1})
        }
        function removeApplication(name)
        {
            for (var i = 0; i < count; ++i) {
                var item = get(i)
                if (item.name == name) {
                    if (item.count <= 1) {
                        remove(i)
                        appTabBar.currentTab = allAppsTab
                        return
                    }
                    setProperty(i, "count", item.count - 1)
                    return
                }
            }
        }
    }

    PlasmaCore.DataSource {
        id: idleTimeSource
        engine: "powermanagement"
        interval: 30000
        connectedSources: ["UserActivity"]
        //Idle whith more than 5 minutes of user inactivity
        property bool idle: data["UserActivity"]["IdleTime"] > 300000
    }

    PlasmaCore.DataSource {
        id: notificationsSource
        engine: "notifications"
        interval: 0

        onSourceAdded: {
            connectSource(source);
        }

        onNewData: {
            var actions = new Array()
            if (data["actions"] && data["actions"].length % 2 == 0) {
                for (var i = 0; i < data["actions"].length; i += 2) {
                    var action = new Object()
                    action["id"] = data["actions"][i]
                    action["text"] = data["actions"][i+1]
                    actions.push(action)
                }
            }
            notificationsRoot.addNotification(
                    sourceName,
                    data["appIcon"],
                    data["image"],
                    data["appName"],
                    data["summary"],
                    data["body"],
                    data["expireTimeout"],
                    data["urgency"],
                    data["appRealName"],
                    data["configurable"],
                    actions)
        }

    }

    Title {
        visible: notificationsRepeater.count > 1 || (jobs && jobs.count > 0 && notificationsRepeater.count > 0)
        text: i18n("Notifications")
        PlasmaComponents.ToolButton {
            iconSource: "window-close"
            width: notificationsApplet.toolIconSize
            height: width
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
            onClicked: notificationsModel.clear()
        }
    }
    PlasmaComponents.ListItem {
        visible: allApplicationsModel.count > 1
        PlasmaComponents.TabBar {
            id: appTabBar
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, parent.width-8)
            PlasmaComponents.TabButton {
                id: allAppsTab
                text: i18n("All")
                iconSource: "dialog-information"
            }
            Repeater {
                model: allApplicationsModel
                PlasmaComponents.TabButton {
                    text: name
                    iconSource: icon
                }
            }
        }
    }
    Repeater {
        id: notificationsRepeater
        model: notificationsModel
        delegate: NotificationDelegate {
            toolIconSize: notificationsApplet.toolIconSize
        }
    }
}
