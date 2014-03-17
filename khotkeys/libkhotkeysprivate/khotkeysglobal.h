/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef _KHOTKEYSGLOBAL_H_
#define _KHOTKEYSGLOBAL_H_

#define KHOTKEYS_VERSION "2.1"
#define KHOTKEYS_CONFIG_FILE "khotkeysrc"

#include <KLocalizedString>
#include <kdemacros.h>

#include <QtCore/QPointer>

class QObject;

namespace KHotKeys
{

class WindowsHandler;
class ShortcutsHandler;

KDE_EXPORT extern QPointer<ShortcutsHandler> keyboard_handler;
extern QPointer<WindowsHandler> windows_handler;

// CHECKME hmms :(
KDE_EXPORT bool khotkeys_active();
KDE_EXPORT void khotkeys_set_active( bool active_P );

QString get_menu_entry_from_path( const QString& path_P );

KDE_EXPORT void init_global_data( bool active_P, QObject* owner_P );

const char* const MENU_EDITOR_ENTRIES_GROUP_NAME = I18N_NOOP( "Menu Editor entries" );

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
