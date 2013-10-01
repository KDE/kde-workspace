/***************************************************************************
 *                                                                         *
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#ifndef SYSTEMTRAYMANAGER__H
#define SYSTEMTRAYMANAGER__H

#include <kdeclarative/qmlobject.h>

#include <QQmlListProperty>
#include <QObject>

class QQuickItem;

namespace SystemTray {
    class Manager;
    class Task;
}

namespace SystemTray {

class HostPrivate;

class Host : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<SystemTray::Task> tasks READ tasks NOTIFY tasksChanged)
    Q_PROPERTY(QQuickItem* rootItem WRITE setRootItem)
    Q_PROPERTY(QQuickItem* notificationsPlasmoid READ notificationsPlasmoid)

public:
    Host(QObject* parent = 0);
    virtual ~Host();
    void setRootItem(QQuickItem* rootItem);

    QQuickItem* notificationsPlasmoid();

    static QmlObject* loadPlasmoid(const QString &plugin, const QVariantHash &args, QQuickItem* parent);

public Q_SLOTS:
    QQmlListProperty<SystemTray::Task> tasks();
    void init();


Q_SIGNALS:
    void tasksChanged();

private:
    static SystemTray::Manager *s_manager;
    static int s_managerUsage;
    HostPrivate* d;

};

} // namespace
#endif // SYSTEMTRAYMANAGER__H
