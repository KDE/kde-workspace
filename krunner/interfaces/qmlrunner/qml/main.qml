/***************************************************************************
 *   Copyright (C) 2012 by Aleix Pol Gonzalez <aleixpol@kde.org>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 1.1
import org.kde.runnermodel 0.1 as RunnerModels
import org.kde.plasma.components 0.1
import org.kde.qtextracomponents 0.1

Item {
    width: 400
    height: (inputRow.height+inputRow.anchors.margins+
                (!contentItem.content || runnerModel.count==0 ? 0 : (contentItem.content.childrenRect.height+contentItem.anchors.margins*2)))
//     Rectangle {
//         id: bg
//         anchors.fill: parent
//         color: "transparent"
// //         opacity: .0
// 
//     }
    id: main
    
    Connections {
        target: app
        onChangeTerm: input.text=term
    }
    
    RunnerModels.RunnerModel { id: runnerModel }
    
    Row {
        id: inputRow
        spacing: 10
        height: 40
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 10
        }
        
        ToolButton {
            iconSource: "configure"
            id: settingsSwitch
            onClicked: app.configure()
        }
        
        ToolButton {
            iconSource: "plasma"
            id: changeStuff
            checkable: true
            onClicked: setView("ResultsPath.qml")
        }
        
        TextField {
            id: input
            onTextChanged: { runnerModel.query = text; contentItem.content.currentIndex = 0 }
            width: 300
            focus: true
            
            onAccepted: runnerModel.run(contentItem.content.currentIndex)
        }
    }
    
    function setView(str) {
        var comp=Qt.createComponent(str)
        
        if(comp.status == Component.Ready) {
            if(contentItem.content)
                contentItem.content.destroy()
            try {
                var obj = comp.createObject(main, { model: runnerModel } )
                contentItem.content=obj
            } catch(e) {
                console.log("error: "+e)
            }
        } else {
            console.log("Error loading component:", comp.errorString());
        }
    }
    
    Component.onCompleted: {
        setView("ResultsList.qml")
    }
    
    Keys.onPressed: {
        if(event.key == Qt.Key_Tab || event.key == Qt.Key_Backtab) {
            var inc = (event.key == Qt.Key_Backtab) ? -1 : 1;
            contentItem.content.currentIndex = (contentItem.content.currentIndex+inc) % runnerModel.count
            event.accepted=true
        }
    }
    
    Keys.onEscapePressed: app.hide()
    
    Item {
        id: contentItem
        anchors {
            top: inputRow.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: 10
        }
        clip: true
        
        property Item content
        
        onContentChanged: content.anchors.fill=contentItem
    }
}
