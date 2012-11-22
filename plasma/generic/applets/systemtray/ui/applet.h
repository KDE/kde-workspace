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

#include <KDE/Plasma/Applet>

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

class Applet : public Plasma::Applet
{
    Q_OBJECT

    Q_ENUMS(FormFactor)
    Q_ENUMS(Location)
    Q_ENUMS(VisibilityPreference)

    Q_PROPERTY(bool firstRun READ isFirstRun)

    // TODO: remove these properties in the future (they will be supported in Plasma::Applet)
    Q_PROPERTY(int formFactor READ formFactor NOTIFY formFactorChanged)
    Q_PROPERTY(int location READ location NOTIFY locationChanged)

public:
    // Form factor
    enum FormFactor
    {
        Planar       = Plasma::Planar,
        MediaCenter  = Plasma::MediaCenter,
        Horizontal   = Plasma::Horizontal,
        Vertical     = Plasma::Vertical
    };

    // Location
    enum Location
    {
        Floating    = Plasma::Floating,
        Desktop     = Plasma::Desktop,
        FullScreen  = Plasma::FullScreen,
        TopEdge     = Plasma::TopEdge,
        BottomEdge  = Plasma::BottomEdge,
        LeftEdge    = Plasma::LeftEdge,
        RightEdge   = Plasma::RightEdge
    };

    /// User's preference of visibility of task
    enum VisibilityPreference {
        AutoVisibility = 0,
        AlwaysHidden,
        AlwaysShown
    };

    explicit Applet(QObject *parent, const QVariantList &arguments = QVariantList());
    ~Applet();

    void init();
    void constraintsEvent(Plasma::Constraints constraints);
    Manager *manager() const;
    QSet<Task::Category> shownCategories() const;
    bool isFirstRun();

    Q_INVOKABLE int getVisibilityPreference(QObject *task) const;
    Q_INVOKABLE QAction* createShortcutAction(QString action_id) const;
    Q_INVOKABLE void updateShortcutAction(QAction *action, QString shortcut) const;
    Q_INVOKABLE void destroyShortcutAction(QAction *action) const;
    Q_INVOKABLE void showMenu(QObject *menu, int x, int y, QObject *item) const;
    Q_INVOKABLE void hideFromTaskbar(qulonglong win_id) const;
    Q_INVOKABLE QString getUniqueId(QObject *obj) const;
    Q_INVOKABLE QPoint popupPosition(QObject *item, QSize size = QSize(0, 0), int align = Qt::AlignLeft) const;

protected:
    void createConfigurationInterface(KConfigDialog *parent);
    void configChanged();

    void mousePressEvent(QGraphicsSceneMouseEvent *event) { Q_UNUSED(event); }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) { Q_UNUSED(event); }
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) { Q_UNUSED(event); }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) { Q_UNUSED(event); }

signals:
    void formFactorChanged();
    void locationChanged();
    void visibilityPreferenceChanged();  ///< If user has changed his preference on visibility of tasks

    /**
     * This signal is emmited for each new task
     * @param task a new task
     */
    void newTask(QObject *task);

    /**
     * This signal is emmited before task is deleted
     * @param task a task that is being deleted
     */
    void deletedTask(QObject *task);

private Q_SLOTS:
    void configAccepted();
    void unlockContainment();
    void propogateSizeHintChange(Qt::SizeHint which);
    void checkDefaultApplets();

    void _onAddedTask(SystemTray::Task*);
    void _onRemovedTask(SystemTray::Task*);
    void _onStatusChangedTask();

    void _onWidgetCreationFinished();

private:
    QString _getActionName(Task *task) const;

private:
    static SystemTray::Manager *s_manager;
    static int s_managerUsage;

    QWeakPointer<QWidget> m_autoHideInterface;
    QWeakPointer<QWidget> m_visibleItemsInterface;
    QSet<Task::Category> m_shownCategories;
    QSet<QString> m_hiddenTypes;
    QSet<QString> m_alwaysShownTypes;
    QDateTime m_lastActivity;
    Plasma::DeclarativeWidget *m_widget;

    Ui::AutoHideConfig m_autoHideUi;
    Ui::VisibleItemsConfig m_visibleItemsUi;

    QWeakPointer<QStandardItemModel> m_visibleItemsSourceModel;

    bool m_firstRun;
};

}


#endif
