/***************************************************************************
 *   Copyright (C) 2012 by Ingomar Wesp <ingomar@wesp.name>                *
 *   Portions copyright (C) 2008 by Lukas Appelhans <l.appelhans@gmx.de>   *
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
#include "launcherlistmodel.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QStandardItemModel>

#include <KConfig>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KEMailSettings>
#include <KMimeTypeTrader>
#include <KMimeType>
#include <KService>
#include <KShell>
#include <KStandardDirs>
#include <KUrl>

LauncherListModel::LauncherListModel(QObject *parent)
:
    QStandardItemModel(parent)
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(Qt::DisplayRole, "display");
    roleNames.insert(DescriptionRole, "description");
    roleNames.insert(IconSourceRole, "iconSource");
    roleNames.insert(URLRole, "url");

    setRoleNames(roleNames);
}

void LauncherListModel::addLauncher(int index, const QString &url)
{
    if (index < 0) {
        index = 0;
    }
    else if (index > rowCount()) {
        index = rowCount();
    }

    QStandardItem *item = itemForUrl(KUrl(url).url());
    insertRow(index, item);
}

void LauncherListModel::removeLauncher(int index)
{
    removeRow(index);
}

QStandardItem *LauncherListModel::itemForUrl(const KUrl &url)
{
    QString name;
    QString description;
    QString icon;

    if (url.isLocalFile() &&
        KDesktopFile::isDesktopFile(url.toLocalFile())) {

        KDesktopFile f(url.toLocalFile());

        name = f.readName();
        description = f.readGenericName();
        icon = f.readIcon();
    } else {
        icon = KMimeType::iconNameForUrl(url);
    }

    if (name.isNull()) {
        name = url.fileName();
    }

    if (icon.isNull()) {
        icon = QString::fromAscii("unknown");
    }

    QStandardItem *item = new QStandardItem();
    item->setData(name, Qt::DisplayRole);
    item->setData(description, DescriptionRole);
    item->setData(icon, IconSourceRole);
    item->setData(url.url(), URLRole);

    return item;
}

void LauncherListModel::clear() {
    QStandardItemModel::clear();
}

void LauncherListModel::restoreDefaultLaunchers()
{
    QStandardItemModel::clear();

    QStringList defaultLauncherPaths;

    defaultLauncherPaths << defaultBrowserPath();
    defaultLauncherPaths << defaultFileManagerPath();
    defaultLauncherPaths << defaultEmailClientPath();

    // Some people use the same program as browser and file manager.
    defaultLauncherPaths.removeDuplicates();

    int index = 0;
    Q_FOREACH(const QString &path, defaultLauncherPaths) {
        if (!path.isEmpty() && QDir::isAbsolutePath(path)) {
            addLauncher(index++, KUrl::fromPath(path).url());
        }
    }
}

QString LauncherListModel::defaultBrowserPath()
{
    KConfigGroup globalConfigGeneral(KGlobal::config(), "General");

    if (globalConfigGeneral.hasKey("BrowserApplication")) {
        QString browser =
            globalConfigGeneral.readPathEntry("BrowserApplication", QString());

        if (!browser.isEmpty()) {
            if (browser.startsWith('!')) { // Literal command

                browser = browser.mid(1);

                // Strip away command line arguments, so we can treat this
                // as a file name.
                QStringList browserCmdArgs(
                    KShell::splitArgs(browser, KShell::AbortOnMeta));

                if (!browserCmdArgs.isEmpty()) {
                    browser = browserCmdArgs.at(0);
                } else {
                    browser.clear();
                }

                if (!browser.isEmpty()) {
                    QFileInfo browserFileInfo(browser);

                    if (browserFileInfo.isAbsolute()) {
                        if (browserFileInfo.isExecutable()) {
                            return browser;
                        }
                    } else { // !browserFileInfo.isAbsolute()
                        browser = KStandardDirs::findExe(browser);
                        if (!browser.isEmpty()) {
                            return browser;
                        }
                    }
                }
            } else {
                KService::Ptr service = KService::serviceByStorageId(browser);
                if (service && service->isValid()) {
                    return service->entryPath();
                }
            }
        }
    }

    // No global browser configured or configuration is invalid. Falling
    // back to MIME type association.
    KService::Ptr service;
    service = KMimeTypeTrader::self()->preferredService("text/html");
    if (service && service->isValid()) {
        return service->entryPath();
    }

    service = KMimeTypeTrader::self()->preferredService("application/xml+xhtml");
    if (service && service->isValid()) {
        return service->entryPath();
    }

    // Fallback to konqueror.
    service = KService::serviceByStorageId("konqueror");
    if (service && service->isValid()) {
        return service->entryPath();
    }

    // Give up.
    return QString();
}

QString LauncherListModel::defaultFileManagerPath()
{
    KService::Ptr service;
    service = KMimeTypeTrader::self()->preferredService("inode/directory");
    if (service && service->isValid()) {
        return service->entryPath();
    }

    // Fallback to dolphin.
    service = KService::serviceByStorageId("dolphin");
    if (service && service->isValid()) {
        return service->entryPath();
    }

    // Give up.
    return QString();
}

QString LauncherListModel::defaultEmailClientPath()
{
    KEMailSettings emailSettings;
    QString mua = emailSettings.getSetting(KEMailSettings::ClientProgram);

    if (!mua.isEmpty()) {

        // Strip away command line arguments, so we can treat this
        // as a file name.
        QStringList muaCmdArgs(KShell::splitArgs(mua, KShell::AbortOnMeta));

        if (!muaCmdArgs.isEmpty()) {
            mua = muaCmdArgs.at(0);
        } else {
            mua.clear();
        }

        if (!mua.isEmpty()) {
            // Strictly speaking, this is incorrect, but it's much better to
            // find the service than just the plain command, so we'll search
            // for services that have the same name as the executable and
            // hope for the best.
            KService::Ptr service = KService::serviceByStorageId(mua);

            if (service && service->isValid()) {
                return service->entryPath();
            }

            // Fallback to the exectuable.
            QFileInfo muaFileInfo(mua);

            if (muaFileInfo.isAbsolute()) {
                if (muaFileInfo.isExecutable()) {
                    return mua;
                }
            } else { // !muaFileInfo.isAbsolute()
                mua = KStandardDirs::findExe(mua);

                if (!mua.isEmpty()) {
                    return mua;
                }
            }
        }
    }

    // Fallback to kmail (if it is installed).
    KService::Ptr service = KService::serviceByStorageId("kmail");
    if (service && service->isValid()) {
        return service->entryPath();
    }

    // Give up.
    return QString();
}

#include "launcherlistmodel.moc"
