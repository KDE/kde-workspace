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

/*

 This file is for (very) small utility functions/classes.

*/

#include "utils.h"

#include <kxerrorhandler.h>
#include <X11/Xatom.h>

#ifndef KCMRULES
#include <assert.h>
#include <kdebug.h>
#include <kkeyserver.h>

#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <QX11Info>

#include <stdio.h>

#include "atoms.h"
#include "cursor.h"
#include "workspace.h"

#endif

namespace KWin
{

#ifndef KCMRULES

//************************************
// StrutRect
//************************************

StrutRect::StrutRect(QRect rect, StrutArea area)
    : QRect(rect)
    , m_area(area)
{
}

StrutRect::StrutRect(const StrutRect& other)
    : QRect(other)
    , m_area(other.area())
{
}

//************************************
// Motif
//************************************

void Motif::readFlags(WId w, bool& got_noborder, bool& noborder,
                      bool& resize, bool& move, bool& minimize, bool& maximize, bool& close)
{
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char* data;
    MwmHints* hints = 0;
    if (XGetWindowProperty(display(), w, atoms->motif_wm_hints, 0, 5,
                          false, atoms->motif_wm_hints, &type, &format,
                          &length, &after, &data) == Success) {
        if (data)
            hints = (MwmHints*) data;
    }
    got_noborder = false;
    noborder = false;
    resize = true;
    move = true;
    minimize = true;
    maximize = true;
    close = true;
    if (hints) {
        // To quote from Metacity 'We support those MWM hints deemed non-stupid'
        if (hints->flags & MWM_HINTS_FUNCTIONS) {
            // if MWM_FUNC_ALL is set, other flags say what to turn _off_
            bool set_value = ((hints->functions & MWM_FUNC_ALL) == 0);
            resize = move = minimize = maximize = close = !set_value;
            if (hints->functions & MWM_FUNC_RESIZE)
                resize = set_value;
            if (hints->functions & MWM_FUNC_MOVE)
                move = set_value;
            if (hints->functions & MWM_FUNC_MINIMIZE)
                minimize = set_value;
            if (hints->functions & MWM_FUNC_MAXIMIZE)
                maximize = set_value;
            if (hints->functions & MWM_FUNC_CLOSE)
                close = set_value;
        }
        if (hints->flags & MWM_HINTS_DECORATIONS) {
            got_noborder = true;
            noborder = !hints->decorations;
        }
        XFree(data);
    }
}

//************************************
// KWinSelectionOwner
//************************************

KWinSelectionOwner::KWinSelectionOwner(int screen_P)
    : KSelectionOwner(make_selection_atom(screen_P), screen_P)
{
}

Atom KWinSelectionOwner::make_selection_atom(int screen_P)
{
    if (screen_P < 0)
        screen_P = DefaultScreen(display());
    char tmp[ 30 ];
    sprintf(tmp, "WM_S%d", screen_P);
    return XInternAtom(display(), tmp, False);
}

void KWinSelectionOwner::getAtoms()
{
    KSelectionOwner::getAtoms();
    if (xa_version == None) {
        Atom atoms[ 1 ];
        const char* const names[] =
        { "VERSION" };
        XInternAtoms(display(), const_cast< char** >(names), 1, False, atoms);
        xa_version = atoms[ 0 ];
    }
}

void KWinSelectionOwner::replyTargets(Atom property_P, Window requestor_P)
{
    KSelectionOwner::replyTargets(property_P, requestor_P);
    Atom atoms[ 1 ] = { xa_version };
    // PropModeAppend !
    XChangeProperty(display(), requestor_P, property_P, XA_ATOM, 32, PropModeAppend,
    reinterpret_cast< unsigned char* >(atoms), 1);
}

bool KWinSelectionOwner::genericReply(Atom target_P, Atom property_P, Window requestor_P)
{
    if (target_P == xa_version) {
        long version[] = { 2, 0 };
        XChangeProperty(display(), requestor_P, property_P, XA_INTEGER, 32,
        PropModeReplace, reinterpret_cast< unsigned char* >(&version), 2);
    } else
        return KSelectionOwner::genericReply(target_P, property_P, requestor_P);
    return true;
}

Atom KWinSelectionOwner::xa_version = None;


#endif

QByteArray getStringProperty(WId w, Atom prop, char separator)
{
    Atom type;
    int format, status;
    unsigned long nitems = 0;
    unsigned long extra = 0;
    unsigned char *data = 0;
    QByteArray result = "";
    KXErrorHandler handler; // ignore errors
    status = XGetWindowProperty(display(), w, prop, 0, 10000,
    false, XA_STRING, &type, &format,
    &nitems, &extra, &data);
    if (status == Success) {
        if (data && separator) {
            for (int i = 0; i < (int)nitems; i++)
                if (!data[i] && i + 1 < (int)nitems)
                    data[i] = separator;
        }
        if (data)
            result = (const char*) data;
        XFree(data);
    }
    return result;
}

#ifndef KCMRULES
static Time next_x_time;
static Bool update_x_time_predicate(Display*, XEvent* event, XPointer)
{
    if (next_x_time != CurrentTime)
        return False;
    // from qapplication_x11.cpp
    switch(event->type) {
    case ButtonPress:
        // fallthrough intended
    case ButtonRelease:
        next_x_time = event->xbutton.time;
        break;
    case MotionNotify:
        next_x_time = event->xmotion.time;
        break;
    case KeyPress:
        // fallthrough intended
    case KeyRelease:
        next_x_time = event->xkey.time;
        break;
    case PropertyNotify:
        next_x_time = event->xproperty.time;
        break;
    case EnterNotify:
    case LeaveNotify:
        next_x_time = event->xcrossing.time;
        break;
    case SelectionClear:
        next_x_time = event->xselectionclear.time;
        break;
    default:
        break;
    }
    return False;
}

/*
 Updates xTime(). This used to simply fetch current timestamp from the server,
 but that can cause xTime() to be newer than timestamp of events that are
 still in our events queue, thus e.g. making XSetInputFocus() caused by such
 event to be ignored. Therefore events queue is searched for first
 event with timestamp, and extra PropertyNotify is generated in order to make
 sure such event is found.
*/
void updateXTime()
{
    static QWidget* w = 0;
    if (!w)
        w = new QWidget;
    long data = 1;
    XChangeProperty(display(), w->winId(), atoms->kwin_running, atoms->kwin_running, 32,
    PropModeAppend, (unsigned char*) &data, 1);
    next_x_time = CurrentTime;
    XEvent dummy;
    XCheckIfEvent(display(), &dummy, update_x_time_predicate, NULL);
    if (next_x_time == CurrentTime) {
        XSync(display(), False);
        XCheckIfEvent(display(), &dummy, update_x_time_predicate, NULL);
    }
    assert(next_x_time != CurrentTime);
    QX11Info::setAppTime(next_x_time);
    XEvent ev; // remove the PropertyNotify event from the events queue
    XWindowEvent(display(), w->winId(), PropertyChangeMask, &ev);
}

static int server_grab_count = 0;

void grabXServer()
{
    if (++server_grab_count == 1)
        XGrabServer(display());
}

void ungrabXServer()
{
    assert(server_grab_count > 0);
    if (--server_grab_count == 0) {
        XUngrabServer(display());
        XFlush(display());
    }
}

bool grabbedXServer()
{
    return server_grab_count > 0;
}

static bool keyboard_grabbed = false;

bool grabXKeyboard(Window w)
{
    if (QWidget::keyboardGrabber() != NULL)
        return false;
    if (keyboard_grabbed)
        return false;
    if (qApp->activePopupWidget() != NULL)
        return false;
    if (w == None)
        w = rootWindow();
    if (XGrabKeyboard(display(), w, False,
    GrabModeAsync, GrabModeAsync, xTime()) != GrabSuccess)
        return false;
    keyboard_grabbed = true;
    return true;
}

void ungrabXKeyboard()
{
    if (!keyboard_grabbed) {
        // grabXKeyboard() may fail sometimes, so don't fail, but at least warn anyway
        kDebug(1212) << "ungrabXKeyboard() called but keyboard not grabbed!";
    }
    keyboard_grabbed = false;
    XUngrabKeyboard(display(), CurrentTime);
}

QPoint cursorPos()
{
    return Cursor::self()->pos();
}

// converting between X11 mouse/keyboard state mask and Qt button/keyboard states

int qtToX11Button(Qt::MouseButton button)
{
    if (button == Qt::LeftButton)
        return Button1;
    else if (button == Qt::MidButton)
        return Button2;
    else if (button == Qt::RightButton)
        return Button3;
    return AnyButton; // 0
}

Qt::MouseButton x11ToQtMouseButton(int button)
{
    if (button == Button1)
        return Qt::LeftButton;
    if (button == Button2)
        return Qt::MidButton;
    if (button == Button3)
        return Qt::RightButton;
    if (button == Button4)
        return Qt::XButton1;
    if (button == Button5)
        return Qt::XButton2;
    return Qt::NoButton;
}

int qtToX11State(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
    int ret = 0;
    if (buttons & Qt::LeftButton)
        ret |= Button1Mask;
    if (buttons & Qt::MidButton)
        ret |= Button2Mask;
    if (buttons & Qt::RightButton)
        ret |= Button3Mask;
    if (modifiers & Qt::ShiftModifier)
        ret |= ShiftMask;
    if (modifiers & Qt::ControlModifier)
        ret |= ControlMask;
    if (modifiers & Qt::AltModifier)
        ret |= KKeyServer::modXAlt();
    if (modifiers & Qt::MetaModifier)
        ret |= KKeyServer::modXMeta();
    return ret;
}

Qt::MouseButtons x11ToQtMouseButtons(int state)
{
    Qt::MouseButtons ret = 0;
    if (state & Button1Mask)
        ret |= Qt::LeftButton;
    if (state & Button2Mask)
        ret |= Qt::MidButton;
    if (state & Button3Mask)
        ret |= Qt::RightButton;
    if (state & Button4Mask)
        ret |= Qt::XButton1;
    if (state & Button5Mask)
        ret |= Qt::XButton2;
    return ret;
}

Qt::KeyboardModifiers x11ToQtKeyboardModifiers(int state)
{
    Qt::KeyboardModifiers ret = 0;
    if (state & ShiftMask)
        ret |= Qt::ShiftModifier;
    if (state & ControlMask)
        ret |= Qt::ControlModifier;
    if (state & KKeyServer::modXAlt())
        ret |= Qt::AltModifier;
    if (state & KKeyServer::modXMeta())
        ret |= Qt::MetaModifier;
    return ret;
}

#endif
} // namespace

#ifndef KCMRULES
#include "utils.moc"
#endif
