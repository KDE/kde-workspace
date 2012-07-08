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

#include <QAbstractListModel>
#include <QRectF>

class VirtualDesktopModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum RectangleRoles {
        WidthRole = Qt::UserRole + 1,
        HeightRole,
        XRole,
        YRole,
    };

    VirtualDesktopModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void append(const QRectF &rect);
    void setList(const QList<QRectF>& list);
    void clear();
    QRectF &operator[](int i);

private:
    int m_currentIndex;
    QList<QRectF> m_rects;
};

#endif // MODEL_H
