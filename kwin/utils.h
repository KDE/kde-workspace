/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

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

#ifndef KWIN_UTILS_H
#define KWIN_UTILS_H

// cmake stuff
#include <config-X11.h>
#include <config-kwin.h>
#include <kwinconfig.h>
// kwin
#include <kwinglobals.h>
// KDE
#include <KDE/NET>
// Qt
#include <QList>
#include <QPoint>
#include <QRect>
#include <QScopedPointer>
// X
#include <X11/Xlib.h>
#include <fixx11h.h>
// system
#include <limits.h>

namespace KWin
{

// window types that are supported as normal windows (i.e. KWin actually manages them)
const int SUPPORTED_MANAGED_WINDOW_TYPES_MASK = NET::NormalMask | NET::DesktopMask | NET::DockMask
        | NET::ToolbarMask | NET::MenuMask | NET::DialogMask /*| NET::OverrideMask*/ | NET::TopMenuMask
        | NET::UtilityMask | NET::SplashMask;
// window types that are supported as unmanaged (mainly for compositing)
const int SUPPORTED_UNMANAGED_WINDOW_TYPES_MASK = NET::NormalMask | NET::DesktopMask | NET::DockMask
        | NET::ToolbarMask | NET::MenuMask | NET::DialogMask /*| NET::OverrideMask*/ | NET::TopMenuMask
        | NET::UtilityMask | NET::SplashMask | NET::DropdownMenuMask | NET::PopupMenuMask
        | NET::TooltipMask | NET::NotificationMask | NET::ComboBoxMask | NET::DNDIconMask;

const long ClientWinMask = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                           XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                           XCB_EVENT_MASK_KEYMAP_STATE |
                           XCB_EVENT_MASK_BUTTON_MOTION |
                           XCB_EVENT_MASK_POINTER_MOTION | // need this, too!
                           XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
                           XCB_EVENT_MASK_FOCUS_CHANGE |
                           XCB_EVENT_MASK_EXPOSURE |
                           XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                           XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;

const QPoint invalidPoint(INT_MIN, INT_MIN);

class Toplevel;
class Client;
class Unmanaged;
class Deleted;
class Group;
class Options;

typedef QList< Toplevel* > ToplevelList;
typedef QList< Client* > ClientList;
typedef QList< const Client* > ConstClientList;
typedef QList< Unmanaged* > UnmanagedList;
typedef QList< Deleted* > DeletedList;

typedef QList< Group* > GroupList;

extern Options* options;

enum Layer {
    UnknownLayer = -1,
    FirstLayer = 0,
    DesktopLayer = FirstLayer,
    BelowLayer,
    NormalLayer,
    DockLayer,
    AboveLayer,
    ActiveLayer, // active fullscreen, or active dialog
    UnmanagedLayer, // layer for override redirect windows.
    NumLayers // number of layers, must be last
};

// yes, I know this is not 100% like standard operator++
inline void operator++(Layer& lay)
{
    lay = static_cast< Layer >(lay + 1);
}

// for Client::takeActivity()
enum ActivityFlags {
    ActivityFocus = 1 << 0, // focus the window
    ActivityFocusForce = 1 << 1, // focus even if Dock etc.
    ActivityRaise = 1 << 2 // raise the window
};

enum StrutArea {
    StrutAreaInvalid = 0, // Null
    StrutAreaTop     = 1 << 0,
    StrutAreaRight   = 1 << 1,
    StrutAreaBottom  = 1 << 2,
    StrutAreaLeft    = 1 << 3,
    StrutAreaAll     = StrutAreaTop | StrutAreaRight | StrutAreaBottom | StrutAreaLeft
};
Q_DECLARE_FLAGS(StrutAreas, StrutArea)

class StrutRect : public QRect
{
public:
    explicit StrutRect(QRect rect = QRect(), StrutArea area = StrutAreaInvalid);
    StrutRect(const StrutRect& other);
    inline StrutArea area() const {
        return m_area;
    };
private:
    StrutArea m_area;
};
typedef QVector<StrutRect> StrutRects;

// some enums to have more readable code, instead of using bools
enum ForceGeometry_t { NormalGeometrySet, ForceGeometrySet };



enum ShadeMode {
    ShadeNone, // not shaded
    ShadeNormal, // normally shaded - isShade() is true only here
    ShadeHover, // "shaded", but visible due to hover unshade
    ShadeActivated // "shaded", but visible due to alt+tab to the window
};

// Whether to keep all windows mapped when compositing (i.e. whether to have
// actively updated window pixmaps).
enum HiddenPreviews {
    // The normal mode with regard to mapped windows. Hidden (minimized, etc.)
    // and windows on inactive virtual desktops are not mapped, their pixmaps
    // are only their icons.
    HiddenPreviewsNever,
    // Like normal mode, but shown windows (i.e. on inactive virtual desktops)
    // are kept mapped, only hidden windows are unmapped.
    HiddenPreviewsShown,
    // All windows are kept mapped regardless of their state.
    HiddenPreviewsAlways
};

class Motif
{
public:
    // Read a window's current settings from its _MOTIF_WM_HINTS
    // property.  If it explicitly requests that decorations be shown
    // or hidden, 'got_noborder' is set to true and 'noborder' is set
    // appropriately.
    static void readFlags(xcb_window_t w, bool& got_noborder, bool& noborder,
                          bool& resize, bool& move, bool& minimize, bool& maximize,
                          bool& close);
    struct MwmHints {
        ulong flags;
        ulong functions;
        ulong decorations;
        long input_mode;
        ulong status;
    };
    enum {
        MWM_HINTS_FUNCTIONS = (1L << 0),
        MWM_HINTS_DECORATIONS = (1L << 1),

        MWM_FUNC_ALL = (1L << 0),
        MWM_FUNC_RESIZE = (1L << 1),
        MWM_FUNC_MOVE = (1L << 2),
        MWM_FUNC_MINIMIZE = (1L << 3),
        MWM_FUNC_MAXIMIZE = (1L << 4),
        MWM_FUNC_CLOSE = (1L << 5)
    };
};

// Class which saves original value of the variable, assigns the new value
// to it, and in the destructor restores the value.
// Used in Client::isMaximizable() and so on.
// It also casts away contness and generally this looks like a hack.
template< typename T >
class TemporaryAssign
{
public:
    TemporaryAssign(const T& var, const T& value)
        : variable(var), orig(var) {
        const_cast< T& >(variable) = value;
    }
    ~TemporaryAssign() {
        const_cast< T& >(variable) = orig;
    }
private:
    const T& variable;
    T orig;
};

template <typename T>
class ScopedCPointer : public QScopedPointer<T, QScopedPointerPodDeleter>
{
public:
    ScopedCPointer(T *p = 0) : QScopedPointer<T, QScopedPointerPodDeleter>(p) {}
};

QByteArray getStringProperty(xcb_window_t w, xcb_atom_t prop, char separator = 0);
void updateXTime();
void grabXServer();
void ungrabXServer();
bool grabbedXServer();
bool grabXKeyboard(xcb_window_t w = rootWindow());
void ungrabXKeyboard();

/**
 * Small helper class which performs @link grabXServer in the ctor and
 * @link ungrabXServer in the dtor. Use this class to ensure that grab and
 * ungrab are matched.
 **/
class XServerGrabber
{
public:
    XServerGrabber() {
        grabXServer();
    }
    ~XServerGrabber() {
        ungrabXServer();
    }
};

// the docs say it's UrgencyHint, but it's often #defined as XUrgencyHint
#ifndef UrgencyHint
#define UrgencyHint XUrgencyHint
#endif

// for STL-like algo's
#define KWIN_CHECK_PREDICATE( name, cls, check ) \
    struct name \
    { \
        inline bool operator()( const cls* cl ) { return check; } \
    }

#define KWIN_COMPARE_PREDICATE( name, cls, type, check ) \
    struct name \
    { \
        typedef type type_helper; /* in order to work also with type being 'const Client*' etc. */ \
        inline name( const type_helper& compare_value ) : value( compare_value ) {} \
        inline bool operator()( const cls* cl ) { return check; } \
        const type_helper& value; \
    }

#define KWIN_PROCEDURE( name, cls, action ) \
    struct name \
    { \
        inline void operator()( cls* cl ) { action; } \
    }

KWIN_CHECK_PREDICATE(TruePredicate, Client, cl == cl /*true, avoid warning about 'cl' */);

template< typename T >
Client* findClientInList(const ClientList& list, T predicate)
{
    for (ClientList::ConstIterator it = list.begin(); it != list.end(); ++it) {
        if (predicate(const_cast< const Client* >(*it)))
            return *it;
    }
    return NULL;
}

template< typename T >
Unmanaged* findUnmanagedInList(const UnmanagedList& list, T predicate)
{
    for (UnmanagedList::ConstIterator it = list.begin(); it != list.end(); ++it) {
        if (predicate(const_cast< const Unmanaged* >(*it)))
            return *it;
    }
    return NULL;
}

inline
int timestampCompare(xcb_timestamp_t time1, xcb_timestamp_t time2)   // like strcmp()
{
    return NET::timestampCompare(time1, time2);
}

inline
xcb_timestamp_t timestampDiff(xcb_timestamp_t time1, xcb_timestamp_t time2)   // returns time2 - time1
{
    return NET::timestampDiff(time1, time2);
}

QPoint cursorPos();

// converting between X11 mouse/keyboard state mask and Qt button/keyboard states
int qtToX11Button(Qt::MouseButton button);
Qt::MouseButton x11ToQtMouseButton(int button);
int qtToX11State(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
Qt::MouseButtons x11ToQtMouseButtons(int state);
Qt::KeyboardModifiers x11ToQtKeyboardModifiers(int state);

void checkNonExistentClients();

} // namespace

// Must be outside namespace
Q_DECLARE_OPERATORS_FOR_FLAGS(KWin::StrutAreas)

#endif
