/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

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
#include "screenlocker.h"
#include "saverengine.h"
#include "workspace.h"
#include <fixx11h.h>
#include "effects.h"
// Qt
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
// KDE
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KAuthorized>
#include <KDE/KDebug>
#include <KDE/KShortcut>
#include <KDE/KLocalizedString>

namespace KWin
{
namespace ScreenLocker
{

ScreenLocker::ScreenLocker::ScreenLocker(QObject *parent)
    : QObject(parent)
    , m_saverEngine(new SaverEngine(this))
    , m_locked(false)
{
    // TODO: test whether lock file exists
}

ScreenLocker::~ScreenLocker()
{
}

void ScreenLocker::initShortcuts(KActionCollection *keys)
{
    if (KAuthorized::authorize(QLatin1String("lock_screen"))) {
        // first make krunner forget its old shortcut
        // we do this directly using the D-Bus interface, as KGlobalAccel/KAction has
        // no nice way of doing this (other than registering and deregistering the
        // krunner shortcut every time)
        QDBusInterface accelIface("org.kde.kglobalaccel", "/kglobalaccel", "org.kde.KGlobalAccel");
        QStringList krunnerShortcutId;
        krunnerShortcutId << QLatin1String("krunner") << QLatin1String("Lock Session") << "" << "";
        QDBusReply<QList<int> > reply = accelIface.call("shortcut", krunnerShortcutId);
        int shortcut = -1;
        if (reply.isValid() && reply.value().size() == 1) {
            shortcut = reply.value().at(0);
            kDebug(1212) << "Existing krunner shortcut for Lock Session found:" << KShortcut(shortcut).toString();
        }
        accelIface.call(QDBus::NoBlock, "unRegister", krunnerShortcutId);

        KAction *a = keys->addAction(QLatin1String("Lock Session"));
        a->setText(i18n("Lock Session"));
        a->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_L));
        if (shortcut >= 0) {
            // if there was a krunner shortcut, use that
            a->setGlobalShortcut(KShortcut(shortcut), KAction::ActiveShortcut, KAction::NoAutoloading);
        }
        connect(a, SIGNAL(triggered(bool)), this, SLOT(lock()));
    }
}

void ScreenLocker::lock()
{
    if (m_locked) {
        return;
    }
    // TODO: create lock file
    bool hasLock = false;
    if (Workspace::self()->compositingActive() && static_cast<EffectsHandlerImpl*>(effects)->provides(Effect::ScreenLocking)) {
        // try locking through an Effect
        hasLock = static_cast<EffectsHandlerImpl*>(effects)->lockScreen();
    }
    if (!hasLock) {
        // no Effect to lock the screen, try legacy X Screen Saver for locking
        hasLock = !m_saverEngine->doLock();
    }
    if (!hasLock) {
        // no working lock implementation
        // TODO: remove lock file
        return;
    }
    m_locked = true;
    emit locked();
}

void ScreenLocker::unlock()
{
    if (!m_locked) {
        return;
    }
    // TODO: remove lock file
    // TODO: if compositing was enforced, remove the blocking
    m_locked = false;
    emit unlocked();
}


} // namespace ScreenLocker
} // namespace KWin
