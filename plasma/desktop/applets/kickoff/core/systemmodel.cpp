/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2007 Kevin Ottens <ervin@kde.org>
    Copyright 2011 Martin Gräßlin <mgraesslin@kde.org>

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

#include "systemmodel.h"

// Qt
#include <QHash>
#include <QTimer>

// KDE
#include <KAuthorized>
#include <KDebug>
#include <KDiskFreeSpaceInfo>
#include <KIcon>
#include <KUrl>
#include <KSycoca>
#include <KFilePlacesModel>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>

// Local
#include "models.h"
#include "systemmodel.h"

using namespace Kickoff;

static const int OFFSET = 100;

struct UsageInfo {
    UsageInfo()
        : used(0),
          available(0)
     {}

    quint64 used;
    quint64 available;
};

class SystemModel::Private
{
public:
    Private(SystemModel *parent)
            : q(parent),
              placesModel(new KFilePlacesModel(parent)),
              currentPlacesModelUsageIndex(0)
    {
        q->setSourceModel(placesModel);

        connect(placesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                q, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
        connect(placesModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                q, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        connect(placesModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                q, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        connect(placesModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                q, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(placesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                q, SLOT(sourceRowsRemoved(QModelIndex,int,int)));

        connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), q, SLOT(reloadApplications()));
    }

    SystemModel * const q;
    KFilePlacesModel *placesModel;
    KService::List appsList;
    QMap<QString, UsageInfo> usageByMountpoint;
    int currentPlacesModelUsageIndex;
};

SystemModel::SystemModel(QObject *parent)
        : KickoffProxyModel(parent)
        , d(new Private(this))
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::DecorationRole] = "decoration";
    roles[Kickoff::SubTitleRole] = "subtitle";
    roles[Kickoff::UrlRole] = "url";
    roles[Kickoff::GroupNameRole] = "group";
    setRoleNames(roles);
    reloadApplications();
}

SystemModel::~SystemModel()
{
    delete d;
}

QModelIndex SystemModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }

    if (d->placesModel->isHidden(sourceIndex)) {
        // hidden items not in our model
        return QModelIndex();
    }
    if (d->placesModel->isDevice(sourceIndex)) {
        if (d->placesModel->data(sourceIndex, KFilePlacesModel::FixedDeviceRole).toBool()) {
            // fixed devices not in our list
            return QModelIndex();
        }
    }
    int count = d->appsList.size() -1;
    if (KAuthorized::authorize("run_command")) {
        ++count;
    }
    for (int i=0; i<d->placesModel->rowCount(); ++i) {
        if (i == sourceIndex.row()) {
            break;
        }
        const QModelIndex index = d->placesModel->index(i, 0);
        if (d->placesModel->isHidden(index)) {
            continue;
        }
        if (d->placesModel->isDevice(index)) {
            if (!d->placesModel->data(index, KFilePlacesModel::FixedDeviceRole).toBool()) {
                ++count;
            }
        } else {
            // visible place
            ++count;
        }
    }
    return index(count, 0);
}

QModelIndex SystemModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid()) {
        return QModelIndex();
    }
    if (proxyIndex.internalId() == 0) {
        // application index
        return QModelIndex();
    }
    return d->placesModel->index(proxyIndex.internalId() - OFFSET, 0);
}

QModelIndex SystemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    int count = d->appsList.size()-1;
    if (KAuthorized::authorize("run_command")) {
        ++count;
    }
    if (row <= count) {
        return createIndex(row, column, 0);
    }
    // find the source
    for (int i=0; i<d->placesModel->rowCount(); ++i) {
        const QModelIndex index = d->placesModel->index(i, 0);
        if (d->placesModel->isHidden(index)) {
            continue;
        }
        if (d->placesModel->isDevice(index)) {
            if (!d->placesModel->data(index, KFilePlacesModel::FixedDeviceRole).toBool()) {
                ++count;
            }
        } else {
            // visible place
            ++count;
        }
        if (row == count) {
            return createIndex(row, column, i + OFFSET);
        }
    }
    return QModelIndex();
}

QModelIndex SystemModel::parent(const QModelIndex &item) const
{
    Q_UNUSED(item)
    return QModelIndex();
}

int SystemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    int count = d->appsList.size();
    if (KAuthorized::authorize("run_command")) {
        ++count;
    }
    for (int i=0; i<d->placesModel->rowCount(); ++i) {
        const QModelIndex index = d->placesModel->index(i, 0);
        if (d->placesModel->isHidden(index)) {
            continue;
        }
        if (d->placesModel->isDevice(index)) {
            if (!d->placesModel->data(index, KFilePlacesModel::FixedDeviceRole).toBool()) {
                ++count;
            }
        } else {
            // visible place
            ++count;
        }
    }

    return count;
}

int SystemModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant SystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.internalId() == 0) {
        if (role == GroupNameRole) {
            return i18n("Applications");
        }
        if (d->appsList.count() < index.row()) {
            return QVariant();
        } else if (d->appsList.count() == index.row()) {
            // "Run Command"
            switch (role) {
                case Qt::DisplayRole:
                    return i18n("Run Command...");
                case Qt::DecorationRole:
                    return KIcon("system-run");
                case SubTitleRole:
                    return i18n("Run a command or a search query");
                case UrlRole:
                    return "run:/";
                default:
                    return QVariant();
            }
        }

        KService::Ptr service = d->appsList[index.row()];

        switch (role) {
        case Qt::DisplayRole:
            return service->name();
        case Qt::DecorationRole:
            return KIcon(service->icon());
        case SubTitleRole:
            return service->genericName();
        case UrlRole:
            return service->entryPath();
        default:
            return QVariant();
        }
    }

    if (role == UrlRole) {
        return d->placesModel->url(mapToSource(index)).url();
    }
    if (role == SubTitleRole) {
        QModelIndex sourceIndex = mapToSource(index);

        if (d->placesModel->isDevice(sourceIndex)) {
            Solid::Device dev = d->placesModel->deviceForIndex(sourceIndex);
            Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();

            if (access) {
                return access->filePath();
            }
        } else {
            KUrl url = d->placesModel->url(sourceIndex);
            return url.isLocalFile() ? url.toLocalFile() : url.prettyUrl();
        }

        return QVariant();
    }
    if (role == GroupNameRole) {
        if (d->placesModel->isDevice(mapToSource(index))) {
            return i18n("Removable Storage");
        } else {
            return i18n("Places");
        }
    }
    return d->placesModel->data(mapToSource(index), role);
}

QVariant SystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section != 0) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return i18n("Computer");
        break;
    default:
        return QVariant();
    }
}

void SystemModel::refreshUsageInfo()
{
    d->currentPlacesModelUsageIndex = 0;
    QTimer::singleShot(100, this, SLOT(refreshNextUsageInfo()));
}

void SystemModel::stopRefreshingUsageInfo()
{
    d->currentPlacesModelUsageIndex = d->placesModel->rowCount();
}

void SystemModel::refreshNextUsageInfo()
{
    if (d->currentPlacesModelUsageIndex >= d->placesModel->rowCount()) {
        return;
    }

    QModelIndex sourceIndex = d->placesModel->index(d->currentPlacesModelUsageIndex, 0);
    if (d->placesModel->isDevice(sourceIndex)) {
        Solid::Device dev = d->placesModel->deviceForIndex(sourceIndex);
        Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();

        if (access && !access->filePath().isEmpty()) {
            KDiskFreeSpaceInfo freeSpace = KDiskFreeSpaceInfo::freeSpaceInfo(access->filePath());
            if (freeSpace.isValid()) {
                UsageInfo info;
                info.used = freeSpace.used() / 1024;
                info.available = freeSpace.available() / 1024;

                d->usageByMountpoint[freeSpace.mountPoint()] = info;
                QModelIndex index = mapFromSource(sourceIndex);
                emit dataChanged(index, index);
            }
        }
    }

    ++d->currentPlacesModelUsageIndex;
    QTimer::singleShot(0, this, SLOT(refreshNextUsageInfo()));
}

void SystemModel::reloadApplications()
{
    const QStringList apps = Kickoff::systemApplicationList();
    d->appsList.clear();

    foreach (const QString &app, apps) {
        KService::Ptr service = KService::serviceByStorageId(app);

        if (service) {
            d->appsList << service;
        }
    }
}

void Kickoff::SystemModel::sourceDataChanged(const QModelIndex &start, const QModelIndex &end)
{
    if (start.parent().isValid()) return;

    QModelIndex ourStart = QModelIndex();
    QModelIndex ourEnd = QModelIndex();
    // find our start
    for (int i=start.row(); i<=end.row(); ++i) {
        const QModelIndex ourIndex = mapFromSource(d->placesModel->index(i, 0));
        if (ourIndex.isValid()) {
            ourStart = ourIndex;
            break;
        }
    }
    if (!ourStart.isValid()) {
        return;
    }
    // find our end
    for (int i=end.row(); i>=start.row(); --i) {
        const QModelIndex ourIndex = mapFromSource(d->placesModel->index(i, 0));
        if (ourIndex.isValid()) {
            ourEnd = ourIndex;
            break;
        }
    }
    if (ourEnd.isValid()) {
        emit dataChanged(ourStart, ourEnd);
    }
}

void Kickoff::SystemModel::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    // TODO: can we update without resetting the model?
    beginResetModel();
}

void Kickoff::SystemModel::sourceRowsInserted(const QModelIndex &parent, int /*start*/, int /*end*/)
{
    Q_UNUSED(parent)
    // TODO: can we update without resetting the model?
    endResetModel();
}

void Kickoff::SystemModel::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    // TODO: can we update without resetting the model?
    beginResetModel();
}

void Kickoff::SystemModel::sourceRowsRemoved(const QModelIndex &parent, int /*start*/, int /*end*/)
{
    Q_UNUSED(parent)
    // TODO: can we update without resetting the model?
    endResetModel();
}

#include "systemmodel.moc"
