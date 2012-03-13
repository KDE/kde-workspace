/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "sessionrunner.h"


#include <KAuthorized>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include "kworkspace/kworkspace.h"

#include "screensaver_interface.h"

SessionRunner::SessionRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)

    setObjectName( QLatin1String("Sessions" ));
    setPriority(LowPriority);
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File | 
                    Plasma::RunnerContext::NetworkLocation);

    m_canLogout = KAuthorized::authorizeKAction("logout") && KAuthorized::authorize("logout");
    if (m_canLogout) {
        addSyntax(Plasma::RunnerSyntax(i18nc("log out command", "logout"),
                  i18n("Logs out, exiting the current desktop session")));
        addSyntax(Plasma::RunnerSyntax(i18nc("shutdown computer command", "shutdown"),
                  i18n("Turns off the computer")));
    }

    if (KAuthorized::authorizeKAction("lock_screen") && m_canLogout) {
        addSyntax(Plasma::RunnerSyntax(i18nc("lock screen command", "lock"),
                  i18n("Locks the current sessions and starts the screen saver")));
    }

    Plasma::RunnerSyntax rebootSyntax(i18nc("restart computer command", "restart"), i18n("Reboots the computer"));
    rebootSyntax.addExampleQuery(i18nc("restart computer command", "reboot"));
    addSyntax(rebootSyntax);

    m_triggerWord = i18nc("switch user command", "switch");
    addSyntax(Plasma::RunnerSyntax(i18nc("switch user command", "switch :q:"),
                     i18n("Switches to the active session for the user :q:, "
                          "or lists all active sessions if :q: is not provided")));

    Plasma::RunnerSyntax fastUserSwitchSyntax(i18n("switch user"),
                                i18n("Starts a new session as a different user"));
    fastUserSwitchSyntax.addExampleQuery(i18n("new session"));
    addSyntax(fastUserSwitchSyntax);

    //"SESSIONS" should not be translated; it's used programmaticaly
    setDefaultSyntax(Plasma::RunnerSyntax("SESSIONS", i18n("Lists all sessions")));

}

SessionRunner::~SessionRunner()
{
}

void SessionRunner::matchCommands(QList<Plasma::QueryMatch> &matches, const QString& term)
{
    if (!m_canLogout) {
        return;
    }

    if (term.compare(i18nc("log out command","logout"), Qt::CaseInsensitive) == 0 ||
            term.compare(i18n("log out"), Qt::CaseInsensitive) == 0) {
        Plasma::QueryMatch match(this);
        match.setText(i18nc("log out command","Logout"));
        match.setIcon(KIcon("system-log-out"));
        match.setData(LogoutAction);
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setRelevance(0.9);
        matches << match;
    } else if (term.compare(i18nc("restart computer command", "restart"), Qt::CaseInsensitive) == 0 ||
            term.compare(i18nc("restart computer command", "reboot"), Qt::CaseInsensitive) == 0) {
        Plasma::QueryMatch match(this);
        match.setText(i18n("Restart the computer"));
        match.setIcon(KIcon("system-reboot"));
        match.setData(RestartAction);
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setRelevance(0.9);
        matches << match;
    } else if (term.compare(i18nc("shutdown computer command","shutdown"), Qt::CaseInsensitive) == 0) {
        Plasma::QueryMatch match(this);
        match.setText(i18n("Shutdown the computer"));
        match.setIcon(KIcon("system-shutdown"));
        match.setData(ShutdownAction);
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setRelevance(0.9);
        matches << match;
    } else if (term.compare(i18nc("lock screen command","lock"), Qt::CaseInsensitive) == 0) {
        if (KAuthorized::authorizeKAction("lock_screen")) {
            Plasma::QueryMatch match(this);
            match.setText(i18n("Lock the screen"));
            match.setIcon(KIcon("system-lock-screen"));
            match.setData(LockAction);
            match.setType(Plasma::QueryMatch::ExactMatch);
            match.setRelevance(0.9);
            matches << match;
        }
    }
}

void SessionRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    QString user;
    bool matchUser = false;

    QList<Plasma::QueryMatch> matches;

    if (term.size() < 3) {
        return;
    }

    // first compare with SESSIONS. this must *NOT* be translated (i18n)
    // as it is used as an internal command trigger (e.g. via d-bus),
    // not as a user supplied query. and yes, "Ugh, magic strings"
    bool listAll = term.compare("SESSIONS", Qt::CaseInsensitive) == 0 ||
                   term.compare(i18nc("User sessions", "sessions"), Qt::CaseInsensitive) == 0;

    if (!listAll) {
        //no luck, try the "switch" user command
        if (term.startsWith(m_triggerWord, Qt::CaseInsensitive)) {
            user = term.right(term.size() - m_triggerWord.length()).trimmed();
            listAll = user.isEmpty();
            matchUser = !listAll;
        } else {
            // we know it's not SESSION or "switch <something>", so let's
            // try some other possibilities
            matchCommands(matches, term);
        }
    }

    //kDebug() << "session switching to" << (listAll ? "all sessions" : term);
    bool switchUser = listAll ||
                      term.compare(i18n("switch user"), Qt::CaseInsensitive) == 0 ||
                      term.compare(i18n("new session"), Qt::CaseInsensitive) == 0;

    if (switchUser &&
        KAuthorized::authorizeKAction("start_new_session") &&
        dm.isSwitchable() &&
        dm.numReserve() >= 0) {
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setIcon(KIcon("system-switch-user"));
        match.setText(i18n("New Session"));
        matches << match;
    }

    // now add the active sessions
    if (listAll || matchUser) {
        SessList sessions;
        dm.localSessions(sessions);

        foreach (const SessEnt& session, sessions) {
            if (!session.vt || session.self) {
                continue;
            }

            QString name = KDisplayManager::sess2Str(session);
            Plasma::QueryMatch::Type type = Plasma::QueryMatch::NoMatch;
            qreal relevance = 0.7;

            if (listAll) {
                type = Plasma::QueryMatch::ExactMatch;
                relevance = 1;
            } else if (matchUser) {
                if (name.compare(user, Qt::CaseInsensitive) == 0) {
                    // we need an elif branch here because we don't
                    // want the last conditional to be checked if !listAll
                    type = Plasma::QueryMatch::ExactMatch;
                    relevance = 1;
                } else if (name.contains(user, Qt::CaseInsensitive)) {
                    type = Plasma::QueryMatch::PossibleMatch;
                }
            }

            if (type != Plasma::QueryMatch::NoMatch) {
                Plasma::QueryMatch match(this);
                match.setType(type);
                match.setRelevance(relevance);
                match.setIcon(KIcon("user-identity"));
                match.setText(name);
                match.setData(QString::number(session.vt));
                matches << match;
            }
        }
    }

    context.addMatches(term, matches);
}

void SessionRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);
    if (match.data().type() == QVariant::Int) {
        KWorkSpace::ShutdownType type = KWorkSpace::ShutdownTypeDefault;

        switch (match.data().toInt()) {
            case LogoutAction:
                type = KWorkSpace::ShutdownTypeNone;
                break;
            case RestartAction:
                type = KWorkSpace::ShutdownTypeReboot;
                break;
            case ShutdownAction:
                type = KWorkSpace::ShutdownTypeHalt;
                break;
            case LockAction:
                lock();
                return;
                break;
        }

        if (type != KWorkSpace::ShutdownTypeDefault) {
            KWorkSpace::ShutdownConfirm confirm = KWorkSpace::ShutdownConfirmDefault;
            KWorkSpace::requestShutDown(confirm, type);
            return;
        }
    }

    if (!match.data().toString().isEmpty()) {
        dm.lockSwitchVT(match.data().toString().toInt());
        return;
    }

    //TODO: this message is too verbose and too technical.
    int result = KMessageBox::warningContinueCancel(
            0,
            i18n("<p>You have chosen to open another desktop session.<br />"
                "The current session will be hidden "
                "and a new login screen will be displayed.<br />"
                "An F-key is assigned to each session; "
                "F%1 is usually assigned to the first session, "
                "F%2 to the second session and so on. "
                "You can switch between sessions by pressing "
                "Ctrl, Alt and the appropriate F-key at the same time. "
                "Additionally, the KDE Panel and Desktop menus have "
                "actions for switching between sessions.</p>",
                7, 8),
            i18n("Warning - New Session"),
            KGuiItem(i18n("&Start New Session"), "fork"),
            KStandardGuiItem::cancel(),
            ":confirmNewSession",
            KMessageBox::PlainCaption | KMessageBox::Notify);

    if (result == KMessageBox::Cancel) {
        return;
    }

    lock();
    dm.startReserve();
}

void SessionRunner::lock()
{
    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",
                                              QDBusConnection::sessionBus());
    if (screensaver.isValid()) {
        screensaver.Lock();
    }
}

#include "sessionrunner.moc"
