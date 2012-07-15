/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "model.h"

VirtualDesktopModel::VirtualDesktopModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[WidthRole] = "width";
    roles[HeightRole] = "height";
    roles[XRole] = "x";
    roles[YRole] = "y";
    roles[WindowsRole] = "windows";
    setRoleNames(roles);
}

int VirtualDesktopModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_rects.count();
}

QVariant VirtualDesktopModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > m_rects.count())
        return QVariant();

    const QPair<QRectF, QObject *> &pair = m_rects[index.row()];
    if (role == WidthRole)
        return pair.first.width();
    else if (role == HeightRole)
        return pair.first.height();
    else if (role == XRole)
        return pair.first.x();
    else if (role == YRole)
        return pair.first.y();
    else if (role == WindowsRole)
        return QVariant::fromValue(pair.second);

    return QVariant();
}

void VirtualDesktopModel::setList(const QList<QRectF> &list)
{
    beginResetModel();

    m_rects.clear();
    QObject *dummy = new VirtualDesktopModel(this);
    foreach(const QRectF &rect, list)
        m_rects.append(qMakePair(rect, dummy));

    endResetModel();
}

void VirtualDesktopModel::setWindows(const QList<QList<QPair<WId, QRectF> > >& windows)
{
    beginResetModel();

    for (int desktopId = 0; desktopId < windows.count(); desktopId++) {
        if (desktopId >= m_rects.count()) {
            break;
        }

        QList<QPair<WId, QRectF> > desktopWindows = windows[desktopId];
        QList<QRectF> rects;
        for(int i = 0; i < desktopWindows.count(); i++) {
            rects.append(desktopWindows[i].second);
        }

        if (m_rects[desktopId].second) {
            m_rects[desktopId].second->deleteLater();
        }

        VirtualDesktopModel *model = new VirtualDesktopModel(this);
        model->setList(rects);
        m_rects[desktopId].second = model;
    }

    endResetModel();
}
