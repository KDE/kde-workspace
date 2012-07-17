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

#ifndef MODEL_H
#define MODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QRectF>
#include <QtGui/QWidgetList> // For WId

class RectangleModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum RectangleRoles {
        WidthRole = Qt::UserRole + 1,
        HeightRole,
        XRole,
        YRole,
    };

    RectangleModel(QObject *parent = 0);

    virtual QHash<int, QByteArray> roles() const;
    virtual void clear();
    void append(const QRectF &rect);
    QRectF &rectAt(int index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
    QList<QRectF> m_rects;
};


class WindowModel : public RectangleModel
{
    Q_OBJECT
public:
    enum WindowRole {
        IdRole = RectangleModel::YRole + 1,
        ActiveRole
    };

    WindowModel(QObject *parent = 0);

    QHash<int, QByteArray> roles() const;
    void clear();
    void append(WId windowId, const QRectF &rect, bool active);
    WId idAt(int index) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
    QList<WId> m_ids;
    QList<bool> m_active;
};


class VirtualDesktopModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum VirtualDesktopRoles {
        WindowsRole = RectangleModel::YRole + 1,
    };

    VirtualDesktopModel(QObject *parent = 0);

    QHash<int, QByteArray> roles() const;

    void clearDesktopRects();
    void appendDesktopRect(const QRectF &rect);
    QRectF &desktopRectAt(int index);

    void clearWindowRects();
    void appendWindowRect(int desktopId, WId window, const QRectF &rect, bool active);
    WindowModel *windowsAt(int index) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

private:
    void printModel() const;

    RectangleModel m_desktops;
    QList<QObject *> m_windows;
};

#endif // MODEL_H
