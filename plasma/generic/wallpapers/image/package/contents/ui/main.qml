/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.wallpapers.image 2.0 as Wallpaper
import org.kde.plasma.core 2.0 as PlasmaCore

Rectangle {
    id: root
    color: wallpaper.configuration.Color
    property string configuredImage: wallpaper.configuration.Image
    property string modelImage: imageWallpaper.wallpaperPath
    property Item currentImage: imageB
    property Item otherImage: imageA

    //public API, the C++ part will look for those
    function setUrl(url) {
        print("Url dropped: " + url)
        wallpaper.configuration.Image = url
    }

    function action_open() {
        Qt.openUrlExternally(imageWallpaper.wallpaperPath);
    }

    //private
    function fadeWallpaper() {
        fadeAnim.running = false
        if (currentImage == imageA) {
            currentImage = imageB
            otherImage = imageA
        } else {
            currentImage = imageA
            otherImage = imageB
        }
        currentImage.source = modelImage
        currentImage.opacity = 0
        otherImage.z = 0
        currentImage.z = 1
        fadeAnim.running = true
    }

    Component.onCompleted: {
        imageWallpaper.addUrl(configuredImage)
        wallpaper.setAction("open", i18n("Open Wallpaper Image"),"document-open");
        fadeWallpaper()
    }

    Behavior on color {
        ColorAnimation { duration: units.longDuration }
    }

    Wallpaper.Image {
        id: imageWallpaper
        //the oneliner of difference between image and slideshow wallpapers
        //renderingMode: Wallpaper.Image.SlideShow
        slidePaths: wallpaper.configuration.SlidePaths
        //resizeMethod: wallpaper.configuration.ResizeMethod
        onResizeMethodChanged:{
            print("Resize method now " + resizeMethod)
        }
    }

    onConfiguredImageChanged: {
        imageWallpaper.addUrl(configuredImage)
    }
    onModelImageChanged: {
        fadeWallpaper()
    }

    SequentialAnimation {
        id: fadeAnim
        running: false
        PropertyAnimation {
            target: currentImage
            properties: "opacity"
            from: 0
            to: 1
            duration: units.longDuration
        }
        ScriptAction {
            script: otherImage.opacity = 0
        }
    }

    Image {
        id: imageA
        anchors.fill: parent
        asynchronous: true
        clip: true
        fillMode: wallpaper.configuration.ResizeMethod
        sourceSize {
            width: imageA.width
            height: imageA.height
        }
    }
    Image {
        id: imageB
        anchors.fill: parent
        asynchronous: true
        clip: true
        fillMode: wallpaper.configuration.ResizeMethod
        sourceSize {
            width: imageB.width
            height: imageB.height
        }
    }
}
