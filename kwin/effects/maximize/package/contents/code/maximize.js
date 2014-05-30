/********************************************************************
 This file is part of the KDE project.

 Copyright (C) 2012 Martin Gr��lin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
/*global effect, effects, animate, animationTime, Effect*/
var maximizeEffect = {
    duration: animationTime(250),
    loadConfig: function () {
        "use strict";
        maximizeEffect.duration = animationTime(250);
    },
    maximizeChanged: function (window) {
        "use strict";
        if (!window.oldGeometry) {
            return;
        }
        var oldGeometry, newGeometry;
        oldGeometry = window.oldGeometry;
        newGeometry = window.geometry;
        if (oldGeometry.width == newGeometry.width && oldGeometry.height == newGeometry.height)
            oldGeometry = window.olderGeometry;
        window.olderGeometry = window.oldGeometry;
        window.oldGeometry = newGeometry;
        animate({
            window: window,
            duration: maximizeEffect.duration,
            animations: [{
                type: Effect.Size,
                to: {
                    value1: newGeometry.width,
                    value2: newGeometry.height
                },
                from: {
                    value1: oldGeometry.width,
                    value2: oldGeometry.height
                }
            }, {
                type: Effect.Translation,
                to: {
                    value1: 0,
                    value2: 0
                },
                from: {
                    value1: oldGeometry.x - newGeometry.x - (newGeometry.width / 2 - oldGeometry.width / 2),
                    value2: oldGeometry.y - newGeometry.y - (newGeometry.height / 2 - oldGeometry.height / 2)
                }
            }, {
                type: Effect.CrossFadePrevious,
                to: 1.0,
                from: 0.0
            }]
        });
    },
    geometryChange: function (window, oldGeometry) {
        "use strict";
        window.oldGeometry = window.geometry;
        window.olderGeometry = oldGeometry;
    },
    init: function () {
        "use strict";
        effect.configChanged.connect(maximizeEffect.loadConfig);
        effects.windowGeometryShapeChanged.connect(maximizeEffect.geometryChange);
        effects.windowMaximizedStateChanged.connect(maximizeEffect.maximizeChanged);
    }
};
maximizeEffect.init();
