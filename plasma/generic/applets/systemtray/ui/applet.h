/***************************************************************************
 *   applet.h                                                              *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>                   *
 *   Copyright (C) 2012 Dmitry Ashkadov <dmitry.ashkadov@gmail.com>        *
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

#ifndef APPLET_H
#define APPLET_H

#include <plasma/popupapplet.h>

#include "ui_autohide.h"
#include "ui_visibleitems.h"

#include "../core/task.h"

namespace Plasma
{
class ExtenderItem;
class TabBar;
class Dialog;
class DeclarativeWidget;
}

class QStandardItemModel;

namespace SystemTray
{

class Manager;
class TaskArea;
class Plasmoid;

class Applet : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(bool firstRun READ isFirstRun)

public:
    explicit Applet(QObject *parent, const QVariantList &arguments = QVariantList());
    ~Applet();

    void init();
    void constraintsEvent(Plasma::Constraints constraints);
    Manager *manager() const;
    QSet<Task::Category> shownCategories() const;
    bool isFirstRun();

protected:
    void createConfigurationInterface(KConfigDialog *parent);
    void configChanged();

    void mousePressEvent(QGraphicsSceneMouseEvent *event) { Q_UNUSED(event); }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) { Q_UNUSED(event); }
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) { Q_UNUSED(event); }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) { Q_UNUSED(event); }

private Q_SLOTS:
    void configAccepted();
    void unlockContainment();
    void propogateSizeHintChange(Qt::SizeHint which);
    void themeChanged();
    void checkDefaultApplets();

private:
    static SystemTray::Manager *s_manager;
    static int s_managerUsage;

    QWeakPointer<QWidget> m_autoHideInterface;
    QWeakPointer<QWidget> m_visibleItemsInterface;
    QSet<Task::Category> m_shownCategories;
    QDateTime m_lastActivity;
    Plasmoid *m_plasmoid;
    Plasma::DeclarativeWidget *m_widget;

    Ui::AutoHideConfig m_autoHideUi;
    Ui::VisibleItemsConfig m_visibleItemsUi;

    QWeakPointer<QStandardItemModel> m_visibleItemsSourceModel;

    bool m_firstRun;
};

}


#endif
