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

#include <QtGui/QStandardItemModel>

#include <KDesktopFile>
#include <KMimeType>
#include <KUrl>

LauncherListModel::LauncherListModel(QObject *parent)
:
    QStandardItemModel(parent)
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(Qt::DisplayRole, "display");
    roleNames.insert(DescriptionRole, "description");
    roleNames.insert(IconRole, "icon");
    roleNames.insert(URLRole, "url");

    setRoleNames(roleNames);
}

void LauncherListModel::addLauncher(int index, const QString &url)
{
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
    item->setData(icon, IconRole);
    item->setData(url.url(), URLRole);

    return item;
}

#include "launcherlistmodel.moc"
