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

RectangleModel::RectangleModel(QObject *parent)
    : QAbstractListModel(parent)
{
    setRoleNames(roles());
}

QHash<int, QByteArray> RectangleModel::roles() const
{
    QHash<int, QByteArray> rectRoles;
    rectRoles[WidthRole] = "width";
    rectRoles[HeightRole] = "height";
    rectRoles[XRole] = "x";
    rectRoles[YRole] = "y";
    return rectRoles;
}

void RectangleModel::resetModel(const QList<QRectF> &list)
{
    beginResetModel();
    setList(list);
    endResetModel();
}

void RectangleModel::setList(const QList<QRectF> &list)
{
    m_rects.clear();
    m_rects = list;
}

int RectangleModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_rects.count();
}

QVariant RectangleModel::data(const QModelIndex &index, int role) const
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


VirtualDesktopModel::VirtualDesktopModel(QObject *parent)
    : RectangleModel(parent)
{
    setRoleNames(roles());
}

RectangleModel *VirtualDesktopModel::windowsAt(int index) const
{
    return qobject_cast<RectangleModel *>(m_windows[index]);
}

QHash<int, QByteArray> VirtualDesktopModel::roles() const
{
    QHash<int, QByteArray> rectRoles = RectangleModel::roles();
    rectRoles[WindowsRole] = "windows";
    return rectRoles;
}

void VirtualDesktopModel::resetModel(const QList<QRectF> &list)
{
    beginResetModel();

    RectangleModel::setList(list);
    for (int i = 0; i < list.count(); i++) {
        if (i < m_windows.count())
            windowsAt(i)->resetModel();
        else
            m_windows.append(new RectangleModel(this));
    }

    endResetModel();
}

QVariant VirtualDesktopModel::data(const QModelIndex &index, int role) const
{
    if (role >= RectangleModel::WidthRole && role < WindowsRole)
        return RectangleModel::data(index, role);
    else if (role == WindowsRole)
        return QVariant::fromValue(m_windows[index.row()]);

    return QVariant();
}

void VirtualDesktopModel::setWindows(const QList<QList<QPair<WId, QRectF> > >& windows)
{
    beginResetModel();

    for (int desktopId = 0; desktopId < windows.count(); desktopId++) {
        if (desktopId >= RectangleModel::rowCount())
            break;

        QList<QPair<WId, QRectF> > desktopWindows = windows[desktopId];
        QList<QRectF> rects;
        for(int i = 0; i < desktopWindows.count(); i++)
            rects.append(desktopWindows[i].second);

        windowsAt(desktopId)->resetModel(rects);
    }

    endResetModel();
}
