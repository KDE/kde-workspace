/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

// Own
#include "core/leavemodel.h"

// Qt
#include <QFileInfo>

// KDE
#include <KAuthorized>
#include <KConfigGroup>
#include <KDebug>
#include <KIcon>
#include <Solid/PowerManagement>
#include <kworkspace/kworkspace.h>
#include <kworkspace/kdisplaymanager.h>

// Local
#include "core/models.h"

using namespace Kickoff;

class LeaveModel::Private
{
};

QStandardItem* LeaveModel::createStandardItem(const QString& url)
{
    //Q_ASSERT(KUrl(url).scheme() == "leave");
    QStandardItem *item = new QStandardItem();
    const QString basename = QFileInfo(url).baseName();
    if (basename == "logoutonly") {
        item->setText(i18n("Log out"));
        item->setIcon(KIcon("system-log-out"));
        item->setData(i18n("End session"), Kickoff::SubTitleRole);
    } else if (basename == "lock") {
        item->setText(i18n("Lock"));
        item->setIcon(KIcon("system-lock-screen"));
        item->setData(i18n("Lock screen"), Kickoff::SubTitleRole);
    } else if (basename == "switch") {
        item->setText(i18n("Switch user"));
        item->setIcon(KIcon("system-switch-user"));
        item->setData(i18n("Start a parallel session as a different user"), Kickoff::SubTitleRole);
    } else if (basename == "shutdown") {
        item->setText(i18n("Shut down"));
        item->setIcon(KIcon("system-shutdown"));
        item->setData(i18n("Turn off computer"), Kickoff::SubTitleRole);
    } else if (basename == "restart") {
        item->setText(i18nc("Restart computer", "Restart"));
        item->setIcon(KIcon("system-reboot"));
        item->setData(i18n("Restart computer"), Kickoff::SubTitleRole);
    } else if (basename == "savesession") {
        item->setText(i18n("Save Session"));
        item->setIcon(KIcon("document-save"));
        item->setData(i18n("Save current session for next login"), Kickoff::SubTitleRole);
    } else if (basename == "standby") {
        item->setText(i18nc("Puts the system on standby", "Standby"));
        item->setIcon(KIcon("system-suspend"));
        item->setData(i18n("Pause without logging out"), Kickoff::SubTitleRole);
    } else if (basename == "suspenddisk") {
        item->setText(i18n("Hibernate"));
        item->setIcon(KIcon("system-suspend-hibernate"));
        item->setData(i18n("Suspend to disk"), Kickoff::SubTitleRole);
    } else if (basename == "suspendram") {
        item->setText(i18n("Sleep"));
        item->setIcon(KIcon("system-suspend"));
        item->setData(i18n("Suspend to RAM"), Kickoff::SubTitleRole);
    } else {
        item->setText(basename);
        item->setData(url, Kickoff::SubTitleRole);
    }
    item->setData(url, Kickoff::UrlRole);
    return item;
}

LeaveModel::LeaveModel(QObject *parent)
        : QStandardItemModel(parent)
        , d(0)
{
}

QVariant LeaveModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section != 0) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return i18n("Leave");
        break;
    default:
        return QVariant();
    }
}

void LeaveModel::updateModel()
{
    clear();

    // Session Options
    QStandardItem *sessionOptions = new QStandardItem(i18n("Session"));

    // Logout
    const bool canLogout = KAuthorized::authorizeKAction("logout") && KAuthorized::authorize("logout");
    if (canLogout) {
        QStandardItem *logoutOption = createStandardItem("leave:/logoutonly");
        sessionOptions->appendRow(logoutOption);
    }

    // Lock
    if (KAuthorized::authorizeKAction("lock_screen")) {
        QStandardItem *lockOption = createStandardItem("leave:/lock");
        sessionOptions->appendRow(lockOption);
    }

    // Save Session
    if (canLogout) {
        KConfigGroup c(KSharedConfig::openConfig("ksmserverrc", KConfig::NoGlobals), "General");
        if (c.readEntry("loginMode") == "restoreSavedSession") {
            QStandardItem *saveSessionOption = createStandardItem("leave:/savesession");
            sessionOptions->appendRow(saveSessionOption);
        }
    }

    // Switch User
    if (KDisplayManager().isSwitchable() && KAuthorized::authorize(QLatin1String("switch_user")))  {
        QStandardItem *switchUserOption = createStandardItem("leave:/switch");
        sessionOptions->appendRow(switchUserOption);
    }

    // System Options
    QStandardItem *systemOptions = new QStandardItem(i18n("System"));
    bool addSystemSession = false;

//FIXME: the proper fix is to implement the KWorkSpace methods for Windows
#ifndef Q_WS_WIN
    QSet< Solid::PowerManagement::SleepState > spdMethods = Solid::PowerManagement::supportedSleepStates();
    if (spdMethods.contains(Solid::PowerManagement::StandbyState)) {
        QStandardItem *standbyOption = createStandardItem("leave:/standby");
        systemOptions->appendRow(standbyOption);
        addSystemSession = true;
    }

    if (spdMethods.contains(Solid::PowerManagement::SuspendState)) {
        QStandardItem *suspendramOption = createStandardItem("leave:/suspendram");
        systemOptions->appendRow(suspendramOption);
        addSystemSession = true;
    }

    if (spdMethods.contains(Solid::PowerManagement::HibernateState)) {
        QStandardItem *suspenddiskOption = createStandardItem("leave:/suspenddisk");
        systemOptions->appendRow(suspenddiskOption);
        addSystemSession = true;
    }

    if (canLogout) {
        if (KWorkSpace::canShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeReboot)) {
            // Restart
            QStandardItem *restartOption = createStandardItem("leave:/restart");
            systemOptions->appendRow(restartOption);
            addSystemSession = true;
        }

        if (KWorkSpace::canShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeHalt)) {
            // Shutdown
            QStandardItem *shutDownOption = createStandardItem("leave:/shutdown");
            systemOptions->appendRow(shutDownOption);
            addSystemSession = true;
        }
    }
#endif

    appendRow(sessionOptions);
    if (addSystemSession) {
        appendRow(systemOptions);
    } else {
        delete systemOptions;
    }
}

LeaveModel::~LeaveModel()
{
    delete d;
}

#include "leavemodel.moc"

