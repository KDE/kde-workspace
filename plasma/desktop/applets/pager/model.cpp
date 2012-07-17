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

void RectangleModel::clear()
{
    m_rects.clear();
}

void RectangleModel::append(const QRectF &rect)
{
    m_rects.append(rect);
}

QRectF &RectangleModel::rectAt(int index)
{
    return m_rects[index];
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


WindowModel::WindowModel(QObject *parent)
    : RectangleModel(parent)
{
    setRoleNames(roles());
}

QHash<int, QByteArray> WindowModel::roles() const
{
    QHash<int, QByteArray> rectRoles = RectangleModel::roles();
    rectRoles[IdRole] = "windowId";
    rectRoles[ActiveRole] = "active";
    return rectRoles;
}

void WindowModel::clear()
{
    beginResetModel();
    RectangleModel::clear();
    m_ids.clear();
    m_active.clear();
    endResetModel();
}

void WindowModel::append(WId windowId, const QRectF &rect, bool active)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_ids.append(windowId);
    RectangleModel::append(rect);
    m_active.append(active);
    endInsertRows();
}

WId WindowModel::idAt(int index) const
{
    return m_ids[index];
}
QVariant WindowModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    if (role >= RectangleModel::WidthRole && role < IdRole)
        return RectangleModel::data(index, role);
    else if (role == IdRole)
        return int(m_ids[index.row()]);
    else if (role == ActiveRole)
        return m_active[index.row()];

    return QVariant();
}


VirtualDesktopModel::VirtualDesktopModel(QObject *parent)
    : QAbstractListModel(parent)
{
    setRoleNames(roles());
}

WindowModel *VirtualDesktopModel::windowsAt(int index) const
{
    return qobject_cast<WindowModel *>(m_windows[index]);
}

QHash<int, QByteArray> VirtualDesktopModel::roles() const
{
    QHash<int, QByteArray> rectRoles = m_desktops.roles();
    rectRoles[WindowsRole] = "windows";
    return rectRoles;
}

void VirtualDesktopModel::clearDesktopRects()
{
    beginResetModel();
    m_desktops.clear();
    endResetModel();
}

void VirtualDesktopModel::appendDesktopRect(const QRectF &rect)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_desktops.append(rect);
    endInsertRows();
}

QRectF& VirtualDesktopModel::desktopRectAt(int index)
{
    return m_desktops.rectAt(index);
}

void VirtualDesktopModel::clearWindowRects()
{
    for (int i = 0; i < m_windows.count(); i++) {
        windowsAt(i)->clear();

        // remove the windows model if the number of desktop has decreased
        if (i >= rowCount())
            windowsAt(i)->deleteLater();
    }

    // append more windows model if the number of desktop has increased
    for (int i = m_windows.count(); i < rowCount(); i++)
        m_windows.append(new WindowModel(this));
}

void VirtualDesktopModel::appendWindowRect(int desktopId, WId windowId, const QRectF &rect, bool active)
{
    windowsAt(desktopId)->append(windowId, rect, active);

    QModelIndex i = index(desktopId);
    emit dataChanged(i, i);
}

QVariant VirtualDesktopModel::data(const QModelIndex &index, int role) const
{
    if (role >= RectangleModel::WidthRole && role < WindowsRole) {
        return m_desktops.data(index, role);
    } else if (role == WindowsRole) {
        if (index.row() >= 0 && index.row() < m_windows.count())
            return QVariant::fromValue(m_windows[index.row()]);
        else
            return QVariant();
    }

    return QVariant();
}

int VirtualDesktopModel::rowCount(const QModelIndex &index) const
{
    return m_desktops.rowCount(index);
}

#include <QDebug>
void VirtualDesktopModel::printModel() const
{
    qDebug() << Q_FUNC_INFO << "-- begin --";
    for (int i = 0; i < rowCount(); i++) {
        qDebug() << ">> Desktop" << i;
        QModelIndex ind = index(i);
        qDebug() << "x: " << data(ind, RectangleModel::XRole).toDouble() << "y: "
                 << data(ind, RectangleModel::YRole).toDouble();
        qDebug() << "width: " << data(ind, RectangleModel::WidthRole).toDouble()
                 << "height: " << data(ind, RectangleModel::HeightRole).toDouble();

        QObject *ob = data(ind, WindowsRole).value<QObject *>();
        if (!ob) {
            qDebug() << "empty (not expected)";
            continue;
        }
        QAbstractListModel *model = qobject_cast<QAbstractListModel *>(ob);

        for (int j = 0; j < model->rowCount(); j++) {
            qDebug() << "   >>> Window" << j;
            QModelIndex ind2 = index(j);
            qDebug() << "   x: " << model->data(ind2, RectangleModel::XRole).toDouble()
                     << "y: " << model->data(ind2, RectangleModel::YRole).toDouble();
            qDebug() << "   width: " << model->data(ind2, RectangleModel::WidthRole).toDouble()
                     << "height: " << model->data(ind2, RectangleModel::HeightRole).toDouble();
        }
    }
    qDebug() << Q_FUNC_INFO << "-- end --\n";
}
