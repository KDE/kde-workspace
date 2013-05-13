/*
 * Copyright 2013 Martin Bříza <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sessiontest.h"
#include "kdisplaymanager.h"

#include <qtest_kde.h>

bool operator==(const SessEnt& lhs, const SessEnt& rhs)
{
    return lhs.display == rhs.display;
//            lhs.from == rhs.from && isn't necessarily needed to identify a session... and doesn't work all the time
           lhs.user == rhs.user &&
           lhs.session == rhs.from &&
           lhs.vt == rhs.vt &&
           lhs.self == rhs.self &&
           lhs.tty == rhs.tty;
}

QTEST_KDEMAIN ( SessionTest, NoGUI );

void SessionTest::initTestCase() {
}

void SessionTest::cleanupTestCase() {
    // TODO we should close the created sessions here... somehow...
}

void SessionTest::init() {
}

void SessionTest::cleanup() {
}

void SessionTest::testNewSession() {
#ifndef Q_WS_X11
    // the library doesn't do anything when not used on an X11 system
#else
    KDisplayManager manager;
    SessList sessions_first;
    SessList sessions_second;
    // find all current sessions
    QVERIFY(manager.localSessions(sessions_first));
    QVERIFY(manager.isSwitchable());
    manager.startReserve();
    QVERIFY(manager.localSessions(sessions_second));
    // remove all the previous sessions from the list, only the newly created one should remain
    Q_FOREACH(const SessEnt& ent, sessions_first) {
        QVERIFY(sessions_second.removeOne(ent));
    }
    QVERIFY(sessions_second.count() == 1);
    QVERIFY(!sessions_second.back().self); // the new session mustn't be considered "self"
#endif // Q_WS_X11
}

#include "sessiontest.moc"
