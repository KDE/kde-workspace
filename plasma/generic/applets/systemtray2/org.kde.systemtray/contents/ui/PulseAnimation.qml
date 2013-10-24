/*
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
//import "Animations.js" as Animations

SequentialAnimation {
    id: pulseAnimation
    objectName: "pulseAnimation"

    property Item targetItem
    //property int duration: Animations.normalDuration
    property int duration: 1000

    // Fast scaling while we're animation == more FPS
    ScriptAction { script: { targetItem.smooth = false } }

    SequentialAnimation {

        loops: Animation.Infinite

        PropertyAnimation {
            target: targetItem
            property: "scale"
            from: 1
            to: 1.3
            duration: pulseAnimation.duration / 2
            easing.type: Easing.InExpo;
        }

        PropertyAnimation {
            target: targetItem
            property: "scale"
            from: 1.3
            to: 1
            duration: pulseAnimation.duration / 2
            easing.type: Easing.OutExpo;
        }
    }

    ScriptAction { script: targetItem.smooth = true }
}


