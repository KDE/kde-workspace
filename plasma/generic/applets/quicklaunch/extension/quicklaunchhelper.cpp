/***************************************************************************
 *   Copyright (C) 2012 by Ingomar Wesp <ingomar@wesp.name>                *
 *   Copyright (C) 2008 - 2009 by Lukas Appelhans <l.appelhans@gmx.de>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************/
#include "quicklaunchhelper.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>

#include <KOpenWithDialog>
#include <KPropertiesDialog>
#include <KUrl>

QuicklaunchHelper::QuicklaunchHelper(QObject *parent)
    : QObject(parent)
{}

QString QuicklaunchHelper::showAddLauncherDialog()
{
    QPointer<KOpenWithDialog> appChooseDialog = new KOpenWithDialog(0);
    appChooseDialog->hideRunInTerminal();
    appChooseDialog->setSaveNewApplications(true);

    const bool appChooseDialogAccepted = appChooseDialog->exec();

    if (!appChooseDialog || !appChooseDialogAccepted) {
        delete appChooseDialog;
        return QString::null;
    }

    QString programPath = appChooseDialog->service()->entryPath();
    QString programIcon = appChooseDialog->service()->icon();

    delete appChooseDialog;

    if (programIcon.isEmpty()) {
        // If the program chosen doesn't have an icon, then we give
        // it a default icon and open up its properties in a dialog
        // so the user can change it's icon and name etc
        KConfig kc(programPath, KConfig::SimpleConfig);
        KConfigGroup kcg = kc.group("Desktop Entry");
        kcg.writeEntry("Icon","system-run");
        kc.sync();

        QPointer<KPropertiesDialog> propertiesDialog =
            new KPropertiesDialog(KUrl(programPath), 0);

        const bool propertiesDialogAccepted = propertiesDialog->exec();

        if (!propertiesDialog || !propertiesDialogAccepted) {
            delete propertiesDialog;
            return QString::null;
        }

        // In case the name changed
        programPath = propertiesDialog->kurl().path();
        delete propertiesDialog;
    }

    return programPath;
}

QString QuicklaunchHelper::showEditLauncherDialog(const QString &launcherUrl)
{
    // If the launcher does not point to a desktop file, create one,
    // so that user can change url, icon, text and description.
    /* bool desktopFileCreated = false;

    if (!url.isLocalFile() || !KDesktopFile::isDesktopFile(url.toLocalFile())) {

        QString desktopFilePath = determineNewDesktopFilePath("launcher");

        KConfig desktopFile(desktopFilePath);
        KConfigGroup desktopEntry(&desktopFile, "Desktop Entry");

        desktopEntry.writeEntry("Name", launcherData.name());
        desktopEntry.writeEntry("Comment", launcherData.description());
        desktopEntry.writeEntry("Icon", launcherData.icon());
        desktopEntry.writeEntry("Type", "Link");
        desktopEntry.writeEntry("URL", launcherData.url());

        desktopEntry.sync();

        url = KUrl::fromPath(desktopFilePath);
        desktopFileCreated = true;
    }

    QPointer<KPropertiesDialog> propertiesDialog = new KPropertiesDialog(url);

    if (propertiesDialog->exec() == QDialog::Accepted) {
        url = propertiesDialog->kurl();
        QString path = url.toLocalFile();

        // If the user has renamed the file, make sure that the new
        // file name has the extension ".desktop".
        if (!path.endsWith(QLatin1String(".desktop"))) {
            QFile::rename(path, path+".desktop");
            path += ".desktop";
            url = KUrl::fromLocalFile(path);
        }

        LauncherData newLauncherData(url);

        // TODO: This calls for a setLauncherDataAt method...
        if (m_contextMenuTriggeredOnPopup) {
            PopupLauncherList *popupLauncherList = m_popup->launcherList();
            popupLauncherList->insert(m_contextMenuLauncherIndex, newLauncherData);
            popupLauncherList->removeAt(m_contextMenuLauncherIndex+1);
        } else {
            m_launcherGrid->insert(m_contextMenuLauncherIndex, newLauncherData);
            m_launcherGrid->removeAt(m_contextMenuLauncherIndex+1);
        }

    } else {
        if (desktopFileCreated) {
            // User didn't save the data, delete the temporary desktop file.
            QFile::remove(url.toLocalFile());
        }
    }

    delete propertiesDialog; */
    return QString::null;
}

#include "quicklaunchhelper.moc"
