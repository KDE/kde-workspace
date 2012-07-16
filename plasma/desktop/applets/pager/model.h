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
    virtual void resetModel(const QList<QRectF>& list = QList<QRectF>());

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

protected:
    void setList(const QList<QRectF>& list);

private:
    QList<QRectF> m_rects;
};


class VirtualDesktopModel : public RectangleModel
{
    Q_OBJECT
public:
    enum VirtualDesktopRoles {
        WindowsRole = RectangleModel::YRole + 1,
    };

    VirtualDesktopModel(QObject *parent = 0);

    QHash<int, QByteArray> roles() const;
    void resetModel(const QList<QRectF>& list = QList<QRectF>());

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void setWindows(const QList<QList<QPair<WId, QRectF> > >& allWindows);

private:
    RectangleModel *windowsAt(int index) const;

    QList<QObject *> m_windows;
};

#endif // MODEL_H
