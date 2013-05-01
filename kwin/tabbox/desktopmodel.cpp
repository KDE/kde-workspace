/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2009 Martin Gräßlin <mgraesslin@kde.org>

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
// own
#include "desktopmodel.h"
// tabbox
#include "clientmodel.h"
#include "tabboxconfig.h"
#include "tabboxhandler.h"

#include <math.h>

namespace KWin
{
namespace TabBox
{

DesktopModel::DesktopModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(Qt::DisplayRole, "display");
    roleNames.insert(DesktopNameRole, "caption");
    roleNames.insert(DesktopRole, "desktop");
    roleNames.insert(ClientModelRole, "client");
    setRoleNames(roleNames);
}

DesktopModel::~DesktopModel()
{
}

QVariant DesktopModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.column() != 0)
        return QVariant();

    if (index.parent().isValid()) {
        // parent is valid -> access to Client
        ClientModel *model = m_clientModels[ m_desktopList[ index.internalId() - 1] ];
        return model->data(model->index(index.row(), 0), role);
    }

    const int desktopIndex = index.row();
    if (desktopIndex >= m_desktopList.count())
        return QVariant();
    switch(role) {
    case Qt::DisplayRole:
    case DesktopNameRole:
        return tabBox->desktopName(m_desktopList[ desktopIndex ]);
    case DesktopRole:
        return m_desktopList[ desktopIndex ];
    case ClientModelRole:
        return qVariantFromValue((void*)m_clientModels[ m_desktopList[ desktopIndex ] ]);
    default:
        return QVariant();
    }
}

int DesktopModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

int DesktopModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        if (parent.internalId() != 0 || parent.row() >= m_desktopList.count()) {
            return 0;
        }
        const int desktop = m_desktopList.at(parent.row());
        const ClientModel *model = m_clientModels.value(desktop);
        return model->rowCount();
    }
    return m_desktopList.count();
}

QModelIndex DesktopModel::parent(const QModelIndex& child) const
{
    if (!child.isValid() || child.internalId() == 0) {
        return QModelIndex();
    }
    const int row = child.internalId() -1;
    if (row >= m_desktopList.count()) {
        return QModelIndex();
    }
    return createIndex(row, 0);
}

QModelIndex DesktopModel::index(int row, int column, const QModelIndex& parent) const
{
    if (column != 0) {
        return QModelIndex();
    }
    if (row < 0) {
        return QModelIndex();
    }
    if (parent.isValid()) {
        if (parent.row() < 0 || parent.row() >= m_desktopList.count() || parent.internalId() != 0) {
            return QModelIndex();
        }
        const int desktop = m_desktopList.at(parent.row());
        const ClientModel *model = m_clientModels.value(desktop);
        if (row >= model->rowCount()) {
            return QModelIndex();
        }
        return createIndex(row, column, parent.row() + 1);
    }
    if (row > m_desktopList.count() || m_desktopList.isEmpty())
        return QModelIndex();
    return createIndex(row, column);
}

QModelIndex DesktopModel::desktopIndex(int desktop) const
{
    if (desktop > m_desktopList.count())
        return QModelIndex();
    return createIndex(m_desktopList.indexOf(desktop), 0);
}

void DesktopModel::createDesktopList()
{
    beginResetModel();
    m_desktopList.clear();
    qDeleteAll(m_clientModels);
    m_clientModels.clear();

    switch(tabBox->config().desktopSwitchingMode()) {
    case TabBoxConfig::MostRecentlyUsedDesktopSwitching: {
        int desktop = tabBox->currentDesktop();
        do {
            m_desktopList.append(desktop);
            ClientModel* clientModel = new ClientModel(this);
            clientModel->createClientList(desktop);
            m_clientModels.insert(desktop, clientModel);
            desktop = tabBox->nextDesktopFocusChain(desktop);
        } while (desktop != tabBox->currentDesktop());
        break;
    }
    case TabBoxConfig::StaticDesktopSwitching: {
        for (int i = 1; i <= tabBox->numberOfDesktops(); i++) {
            m_desktopList.append(i);
            ClientModel* clientModel = new ClientModel(this);
            clientModel->createClientList(i);
            m_clientModels.insert(i, clientModel);
        }
        break;
    }
    }
    endResetModel();
}

} // namespace Tabbox
} // namespace KWin
