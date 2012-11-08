/***********************************************************************************************************************
 * KDE System Tray (Plasmoid)
 *
 * Copyright (C) 2012 ROSA  <support@rosalab.ru>
 * License: GPLv2+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 **********************************************************************************************************************/

#ifndef __SYSTEMTRAY__PLASMOID_H
#define __SYSTEMTRAY__PLASMOID_H


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include "applet.h"

#include <QtCore/QObject>
#include <QtCore/QSet>


namespace SystemTray
{
class Task;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Plasmoid
// Makes applet be accessible from QML code, like global object "plasmoid" for declarative applets
class Plasmoid: public QObject
{
    Q_OBJECT

    Q_ENUMS(VisibilityPreference)

    Q_PROPERTY(QObject* applet READ applet CONSTANT) ///< return pointer to applet
public:
    /// User's preference of visibility of task
    enum VisibilityPreference {
        AutoVisibility = 0,
        AlwaysHidden,
        AlwaysShown
    };


    explicit Plasmoid(SystemTray::Applet *parent);
    virtual ~Plasmoid();
    QObject* applet() const { return m_applet; }

    /**
     * Adds task to QML code
     * @param task a new task
     */
    void addTask(Task *task);

    /**
     * Removes old task from QML code
     * @param task a task to be removed
     */
    void removeTask(Task *task);

    /**
     * @return true if task is added to QML code
     */
    bool hasTask(Task *task);

    void updateVisibilityPreference();

    Q_INVOKABLE QVariant createShortcutAction(QString action_id) const;
    Q_INVOKABLE void updateShortcutAction(QVariant action, QString shortcut) const;
    Q_INVOKABLE void showMenu(QVariant menu, int x, int y, QVariant item) const;
    Q_INVOKABLE QPoint popupPosition(QVariant item, QSize size = QSize(0, 0), int align = Qt::AlignLeft) const;
    Q_INVOKABLE void destroyShortcutAction(QVariant action) const;
    Q_INVOKABLE void hideFromTaskbar(qulonglong win_id) const;
    Q_INVOKABLE QString getUniqueId(QObject *obj) const;
    Q_INVOKABLE int getTaskVisibilityPreference(QObject *task) const;

signals:
    void activated(); ///< If a plasmoid has been activated
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

private:
    SystemTray::Applet* m_applet;
    QSet<Task*> m_tasks;
};

} // namespace SystemTray

#endif // __SYSTEMTRAY__PLASMOID_H

