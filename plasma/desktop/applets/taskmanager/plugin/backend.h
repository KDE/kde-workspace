/***************************************************************************
 *   Copyright (C) 2013 by Eike Hein <hein@kde.org>                        *
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

#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>

#include <taskmanager/groupmanager.h>
#include <taskmanager/tasksmodel.h>

#include <KWindowSystem>

class QQuickItem;

class Backend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* groupManager READ groupManager CONSTANT)
    Q_PROPERTY(QObject* tasksModel READ tasksModel CONSTANT)
    Q_PROPERTY(QQuickItem* taskManagerItem READ taskManagerItem WRITE setTaskManagerItem NOTIFY taskManagerItemChanged)
    Q_PROPERTY(int activeWindowId READ activeWindowId NOTIFY activeWindowIdChanged)
    Q_PROPERTY(bool anyTaskNeedsAttention READ anyTaskNeedsAttention)
    Q_PROPERTY(bool highlightWindows READ highlightWindows WRITE setHighlightWindows NOTIFY highlightWindowsChanged)
    Q_PROPERTY(int groupingStrategy READ groupingStrategy WRITE setGroupingStrategy)
    Q_PROPERTY(int sortingStrategy READ sortingStrategy WRITE setSortingStrategy)

    public:
        Backend(QObject *parent = 0);
        ~Backend();

        TaskManager::GroupManager *groupManager() const;
        TaskManager::TasksModel *tasksModel() const;

        QQuickItem* taskManagerItem() const;
        void setTaskManagerItem(QQuickItem *item);

        qulonglong activeWindowId() const;

        bool anyTaskNeedsAttention() const;

        bool highlightWindows() const;
        void setHighlightWindows(bool highlight);

        int groupingStrategy() const;
        void setGroupingStrategy(int groupingStrategy);

        int sortingStrategy() const;
        void setSortingStrategy(int sortingStrategy);

    public Q_SLOTS:
        void activateItem(int id, bool toggle);
        void itemContextMenu(QQuickItem *item, int id, QObject *configAction);
        void itemHovered(int id, bool hovered);
        void itemMove(int id, int newIndex);
        void itemGeometryChanged(QQuickItem *item, int id);

        void handleActiveWindowChanged(WId activeWindow);

    Q_SIGNALS:
        void taskManagerItemChanged(QQuickItem*);
        void activeWindowIdChanged(qulonglong);
        void highlightWindowsChanged(bool);

    private:
        TaskManager::GroupManager *m_groupManager;
        TaskManager::TasksModel *m_tasksModel;
        QQuickItem* m_taskManagerItem;
        WId m_lastWindowId;
        qulonglong m_activeWindowId;
        bool m_highlightWindows;
};

#endif
