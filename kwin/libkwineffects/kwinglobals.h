/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>

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

#ifndef KWIN_LIB_KWINGLOBALS_H
#define KWIN_LIB_KWINGLOBALS_H

#include <QtGui/QX11Info>
#include <QtCore/QPoint>
#include <QtGui/QRegion>

#include <kdemacros.h>

#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <fixx11h.h>

#include <kwinconfig.h>

#define KWIN_EXPORT KDE_EXPORT

namespace KWin
{


enum CompositingType {
    NoCompositing = 0,
    /**
     * Used as a flag whether OpenGL based compositing is used.
     * The flag is or-ed to the enum values of the specific OpenGL types.
     * The actual Compositors use the @c OpenGL1Compositing or @c OpenGL2Compositing
     * flags. If you need to know whether OpenGL is used, either and the flag or
     * use EffectsHandler::isOpenGLCompositing().
     **/
    OpenGLCompositing = 1,
    XRenderCompositing = 1<<1,
    OpenGL1Compositing = 1<<2 | OpenGLCompositing,
    OpenGL2Compositing = 1<<3 | OpenGLCompositing
};

enum OpenGLPlatformInterface {
    NoOpenGLPlatformInterface = 0,
    GlxPlatformInterface,
    EglPlatformInterface
};

enum clientAreaOption {
    PlacementArea,         // geometry where a window will be initially placed after being mapped
    MovementArea,          // ???  window movement snapping area?  ignore struts
    MaximizeArea,          // geometry to which a window will be maximized
    MaximizeFullArea,      // like MaximizeArea, but ignore struts - used e.g. for topmenu
    FullScreenArea,        // area for fullscreen windows
    // these below don't depend on xinerama settings
    WorkArea,              // whole workarea (all screens together)
    FullArea,              // whole area (all screens together), ignore struts
    ScreenArea             // one whole screen, ignore struts
};

enum ElectricBorder {
    ElectricTop,
    ElectricTopRight,
    ElectricRight,
    ElectricBottomRight,
    ElectricBottom,
    ElectricBottomLeft,
    ElectricLeft,
    ElectricTopLeft,
    ELECTRIC_COUNT,
    ElectricNone
};

enum QuickTileFlag {
    QuickTileNone = 0,
    QuickTileLeft = 1,
    QuickTileRight = 1<<1,
    QuickTileTop = 1<<2,
    QuickTileBottom = 1<<3,
    QuickTileHorizontal = QuickTileLeft|QuickTileRight,
    QuickTileVertical = QuickTileTop|QuickTileBottom,
    QuickTileMaximize = QuickTileLeft|QuickTileRight|QuickTileTop|QuickTileBottom
};

Q_DECLARE_FLAGS(QuickTileMode, QuickTileFlag)

// TODO: Hardcoding is bad, need to add some way of registering global actions to these.
// When designing the new system we must keep in mind that we have conditional actions
// such as "only when moving windows" desktop switching that the current global action
// system doesn't support.
enum ElectricBorderAction {
    ElectricActionNone,          // No special action, not set, desktop switch or an effect
    ElectricActionDashboard,     // Launch the Plasma dashboard
    ElectricActionShowDesktop,   // Show desktop or restore
    ElectricActionLockScreen,   // Lock screen
    ElectricActionPreventScreenLocking,
    ELECTRIC_ACTION_COUNT
};

// DesktopMode and WindowsMode are based on the order in which the desktop
//  or window were viewed.
// DesktopListMode lists them in the order created.
enum TabBoxMode {
    TabBoxDesktopMode,                      // Focus chain of desktops
    TabBoxDesktopListMode,                  // Static desktop order
    TabBoxWindowsMode,                      // Primary window switching mode
    TabBoxWindowsAlternativeMode,           // Secondary window switching mode
    TabBoxCurrentAppWindowsMode,            // Same as primary window switching mode but only for windows of current application
    TabBoxCurrentAppWindowsAlternativeMode  // Same as secondary switching mode but only for windows of current application
};

enum KWinOption {
    CloseButtonCorner
};

inline
KWIN_EXPORT Display* display()
{
    return QX11Info::display();
}

inline
KWIN_EXPORT xcb_connection_t *connection()
{
    return XGetXCBConnection(display());
}

inline
KWIN_EXPORT Window rootWindow()
{
    return QX11Info::appRootWindow();
}

inline
KWIN_EXPORT Window xTime()
{
    return QX11Info::appTime();
}

inline
KWIN_EXPORT int displayWidth()
{
    return XDisplayWidth(display(), DefaultScreen(display()));
}

inline
KWIN_EXPORT int displayHeight()
{
    return XDisplayHeight(display(), DefaultScreen(display()));
}

/** @internal */
class KWIN_EXPORT Extensions
{
public:
    static void init();
    static bool shapeAvailable() {
        return shape_version > 0;
    }
    static bool shapeInputAvailable();
    static int shapeNotifyEvent();
    static bool hasShape(Window w);
    static bool randrAvailable() {
        return has_randr;
    }
    static int randrNotifyEvent();
    static bool damageAvailable() {
        return has_damage;
    }
    static int damageNotifyEvent();
    static bool compositeAvailable() {
        return composite_version > 0;
    }
    static bool compositeOverlayAvailable();
    static bool renderAvailable() {
        return render_version > 0;
    }
    static bool fixesAvailable() {
        return fixes_version > 0;
    }
    static bool fixesRegionAvailable();
    static bool syncAvailable() {
        return has_sync;
    }
    static bool nonNativePixmaps() {
        return non_native_pixmaps;
    }
    static int syncAlarmNotifyEvent();
    static void fillExtensionsData(const char**& extensions, int& nextensions, int*&majors, int*& error_bases);
private:
    static void addData(const char* name);
    static int shape_version;
    static int shape_event_base;
    static bool has_randr;
    static int randr_event_base;
    static bool has_damage;
    static int damage_event_base;
    static int composite_version;
    static int render_version;
    static int fixes_version;
    static bool has_sync;
    static int sync_event_base;
    static const char* data_extensions[ 32 ];
    static int data_nextensions;
    static int data_opcodes[ 32 ];
    static int data_error_bases[ 32 ];
    static bool non_native_pixmaps;
};

} // namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(KWin::QuickTileMode)

#endif
