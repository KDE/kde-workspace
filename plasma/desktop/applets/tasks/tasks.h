/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright (C) 2012-2013 by Eike Hein <hein@kde.org>                   *
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

#ifndef TASKS_H
#define TASKS_H

#include "ui_tasksConfig.h"

#include <Plasma/Applet>

namespace Plasma {
    class DeclarativeWidget;
}

namespace TaskManager {
    class TasksModel;
}

class GroupManager;

class Tasks : public Plasma::Applet
{
    Q_OBJECT

    public:
        Tasks(QObject *parent, const QVariantList &args);
        ~Tasks();

        void init();

        void constraintsEvent(Plasma::Constraints constraints);

    signals:
        void settingsChanged();

    public slots:
        void configChanged();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    private slots:
        void activateItem(int id, bool toggle);
        void itemContextMenu(int id);
        void itemHovered(int id, bool hovered);
        void itemMove(int id, int newIndex);
        void itemGeometryChanged(int id, int x, int y, int width, int height);
        void itemNeedsAttention(bool needs);
        void presentWindows(int groupParentId);

        void handleActiveWindowChanged(WId activeWindow);

        void changeSizeHint();
        void optimumCapacityChanged();
        void configAccepted();
        void dialogGroupingChanged(int index);

    private:
        GroupManager *m_groupManager;
        TaskManager::TasksModel *m_tasksModel;

        Plasma::DeclarativeWidget *m_declarativeWidget;

        Ui::tasksConfig m_ui;

        bool m_highlightWindows;
        WId m_lastViewId;
};

K_EXPORT_PLASMA_APPLET(tasks, Tasks)

#endif
