/********************************************************************
 This file is part of the KDE project.

 Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>

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
/*global effect, effects, animate, cancel, set, animationTime, Effect, QEasingCurve */
var translucencyEffect = {
    activeWindow: effects.activeWindow,
    settings: {
        duration: animationTime(250),
        decoration: 100,
        moveresize: 100,
        dialogs: 100,
        inactive: 100,
        comboboxpopups: 100,
        menus: 100,
        dropdownmenus: 100,
        popupmenus: 100,
        tornoffmenus: 100
    },
    loadConfig: function () {
        "use strict";
        var i, individualMenu, windows;
        // TODO: add animation duration
        translucencyEffect.settings.decoration     = effect.readConfig("Decoration", 100);
        translucencyEffect.settings.moveresize     = effect.readConfig("MoveResize", 80);
        translucencyEffect.settings.dialogs        = effect.readConfig("Dialogs", 100);
        translucencyEffect.settings.inactive       = effect.readConfig("Inactive", 100);
        translucencyEffect.settings.comboboxpopups = effect.readConfig("ComboboxPopups", 100);
        translucencyEffect.settings.menus          = effect.readConfig("Menus", 100);
        individualMenu = effect.readConfig("IndividualMenuConfig", false);
        if (individualMenu === true) {
            translucencyEffect.settings.dropdownmenus = effect.readConfig("DropdownMenus", 100);
            translucencyEffect.settings.popupmenus    = effect.readConfig("PopupMenus", 100);
            translucencyEffect.settings.tornoffmenus  = effect.readConfig("TornOffMenus", 100);
        } else {
            translucencyEffect.settings.dropdownmenus = translucencyEffect.settings.menus;
            translucencyEffect.settings.popupmenus    = translucencyEffect.settings.menus;
            translucencyEffect.settings.tornoffmenus  = translucencyEffect.settings.menus;
        }

        windows = effects.stackingOrder;
        for (i = 0; i < windows.length; i += 1) {
            // stop all existing animations
            translucencyEffect.cancelAnimations(windows[i]);
            // schedule new animations based on new settings
            translucencyEffect.startAnimation(windows[i]);
            if (windows[i] !== effects.activeWindow) {
                translucencyEffect.inactive.animate(windows[i]);
            }
        }
    },
    /**
     * @brief Starts the set animations depending on window type
     *
     */
    startAnimation: function (window) {
        "use strict";
        var checkWindow = function (window, value) {
            if (value !== 100) {
                var ids = set({
                    window: window,
                    duration: 1,
                    animations: [{
                        type: Effect.Opacity,
                        from: value / 100.0,
                        to: value / 100.0
                    }]
                });
                window.translucencyWindowTypeAnimation = ids;
            }
        };
        if (window.desktopWindow === true || window.dock === true || window.visible === false) {
            return;
        }
        if (window.dialog === true) {
            checkWindow(window, translucencyEffect.settings.dialogs);
        } else if (window.dropdownMenu === true) {
            checkWindow(window, translucencyEffect.settings.dropdownmenus);
        } else if (window.popupMenu === true) {
            checkWindow(window, translucencyEffect.settings.popupmenus);
        } else if (window.comboBox === true) {
            checkWindow(window, translucencyEffect.settings.comboboxpopups);
        } else if (window.menu === true) {
            checkWindow(window, translucencyEffect.settings.tornoffmenus);
        }
        translucencyEffect.startDecorationAnimation(window);
    },
    startDecorationAnimation: function (window) {
        "use strict";
        var ids;
        if (translucencyEffect.settings.decoration === 100 || window.hasDecoration === false) {
            return;
        }
        ids = set({
            window: window,
            duration: 1,
            animations: [{
                type: Effect.DecorationOpacity,
                from: translucencyEffect.settings.decoration / 100.0,
                to: translucencyEffect.settings.decoration / 100.0
            }]
        });
        window.decorationOpacityAnimation = ids;
    },
    /**
     * @brief Cancels all animations for window type and inactive window
     *
     */
    cancelAnimations: function (window) {
        "use strict";
        if (window.translucencyWindowTypeAnimation !== undefined) {
            cancel(window.translucencyWindowTypeAnimation);
            window.translucencyWindowTypeAnimation = undefined;
        }
        if (window.translucencyInactiveAnimation !== undefined) {
            cancel(window.translucencyInactiveAnimation);
            window.translucencyInactiveAnimation = undefined;
        }
        if (window.decorationOpacityAnimation !== undefined) {
            cancel(window.decorationOpacityAnimation);
            window.decorationOpacityAnimation = undefined;
        }
    },
    moveResize: {
        start: function (window) {
            "use strict";
            var ids;
            if (translucencyEffect.settings.moveresize === 100) {
                return;
            }
            ids = set({
                window: window,
                duration: translucencyEffect.settings.duration,
                animations: [{
                    type: Effect.Opacity,
                    to: translucencyEffect.settings.moveresize / 100.0
                }]
            });
            window.translucencyMoveResizeAnimations = ids;
        },
        finish: function (window) {
            "use strict";
            if (window.translucencyMoveResizeAnimations !== undefined) {
                // start revert animation
                animate({
                    window: window,
                    duration: translucencyEffect.settings.duration,
                    animations: [{
                        type: Effect.Opacity,
                        from: translucencyEffect.settings.moveresize / 100.0
                    }]
                });
                // and cancel previous animation
                cancel(window.translucencyMoveResizeAnimations);
                window.translucencyMoveResizeAnimations = undefined;
            }
        }
    },
    inactive: {
        activated: function (window) {
            "use strict";
            if (translucencyEffect.settings.inactive === 100) {
                return;
            }
            translucencyEffect.inactive.animate(translucencyEffect.activeWindow);
            translucencyEffect.activeWindow = window;
            if (window === null) {
                return;
            }
            if (window.translucencyInactiveAnimation !== undefined) {
                // start revert animation
                animate({
                    window: window,
                    duration: translucencyEffect.settings.duration,
                    animations: [{
                        type: Effect.Opacity,
                        from: translucencyEffect.settings.inactive / 100.0
                    }]
                });
                // and cancel previous animation
                cancel(window.translucencyInactiveAnimation);
                window.translucencyInactiveAnimation = undefined;
            }
        },
        animate: function (window) {
            "use strict";
            var ids;
            if (translucencyEffect.settings.inactive === 100) {
                return;
            }
            if (window === null) {
                return;
            }
            if (window === effects.activeWindow ||
                    window.managed === false ||
                    window.desktopWindow === true ||
                    window.dock === true ||
                    window.visible === false ||
                    window.deleted === true) {
                return;
            }
            ids = set({
                window: window,
                duration: translucencyEffect.settings.duration,
                animations: [{
                    type: Effect.Opacity,
                    to: translucencyEffect.settings.inactive / 100.0
                }]
            });
            window.translucencyInactiveAnimation = ids;
        }
    },
    desktopChanged: function () {
        "use strict";
        var i, windows;
        windows = effects.stackingOrder;
        for (i = 0; i < windows.length; i += 1) {
            translucencyEffect.cancelAnimations(windows[i]);
            translucencyEffect.startAnimation(windows[i]);
            if (windows[i] !== effects.activeWindow) {
                translucencyEffect.inactive.animate(windows[i]);
            }
        }
    },
    init: function () {
        "use strict";
        effect.configChanged.connect(translucencyEffect.loadConfig);
        effects.desktopPresenceChanged.connect(translucencyEffect.cancelAnimations);
        effects.desktopPresenceChanged.connect(translucencyEffect.startAnimation);
        effects.windowAdded.connect(translucencyEffect.startAnimation);
        effects.windowUnminimized.connect(translucencyEffect.startAnimation);
        effects.windowClosed.connect(translucencyEffect.cancelAnimations);
        effects.windowMinimized.connect(translucencyEffect.cancelAnimations);
        effects.windowUnminimized.connect(translucencyEffect.inactive.animate);
        effects.windowStartUserMovedResized.connect(translucencyEffect.moveResize.start);
        effects.windowFinishUserMovedResized.connect(translucencyEffect.moveResize.finish);
        effects.windowActivated.connect(translucencyEffect.inactive.activated);
        effects['desktopChanged(int,int)'].connect(translucencyEffect.desktopChanged);
        translucencyEffect.loadConfig();
    }
};
translucencyEffect.init();
