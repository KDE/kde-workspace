#ifndef DAEMON_H
#define DAEMON_H
/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdemacros.h"

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
namespace KHotKeys {

    namespace Daemon {

    Q_DECL_EXPORT bool isRunning();
    Q_DECL_EXPORT bool reload();
    Q_DECL_EXPORT bool start();
    Q_DECL_EXPORT bool stop();

    /**
     * Is the daemon enabled by the user?
     */
    Q_DECL_EXPORT bool isEnabled();
    }

} // namespace KHotKeys::Daemon

#endif /* #ifndef DAEMON_H */
