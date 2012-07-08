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
    setRoleNames(roles);
    m_currentIndex = 0;
}

int VirtualDesktopModel::rowCount(const QModelIndex &parent) const
{
    return m_rects.count();
}

QVariant VirtualDesktopModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > m_rects.count())
        return QVariant();

    const QRectF &rect = m_rects[index.row()];
    if (role == WidthRole)
        return rect.width();
    else if (role == HeightRole)
        return rect.height();
    else if (role == XRole)
        return rect.x();
    else if (role == YRole)
        return rect.y();

    return QVariant();
}

void VirtualDesktopModel::append(const QRectF &rect)
{
    beginInsertRows(QModelIndex(), m_rects.count(), m_rects.count());
    m_rects << rect;
    endInsertRows();
}

void VirtualDesktopModel::setList(const QList<QRectF> &list)
{
    beginResetModel();
    m_rects = list;
    endResetModel();
}

void VirtualDesktopModel::clear()
{
    beginResetModel();
    return m_rects.clear();
    endResetModel();
}

QRectF &VirtualDesktopModel::operator[](int i)
{
    return m_rects[i];
}
