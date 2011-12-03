/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

#ifndef LEAVEMODEL_H
#define LEAVEMODEL_H

#include "kickoff_export.h"

#include <QStandardItemModel>

namespace Kickoff
{

class KICKOFF_EXPORT LeaveModel : public QStandardItemModel
{
    Q_OBJECT

public:
    LeaveModel(QObject *parent = NULL);
    ~LeaveModel();

    static QStandardItem* createStandardItem(const QString& url);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void updateModel();

private:
    class Private;
    Private * const d;
};

}

#endif // LEAVEMODEL_H
