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
#include <QtCore/QObject>
#include <QtCore/QSet>

#include <KDE/Plasma/Plasma>
#include <KDE/Plasma/Applet>


namespace SystemTray
{
class Task;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Plasmoid
// Makes applet be accessible from QML code, like global object "plasmoid" for declarative applets
class Plasmoid: public QObject
{
    Q_OBJECT

    Q_ENUMS(FormFactor)
    Q_ENUMS(Location)

    Q_PROPERTY(FormFactor formFactor READ formFactor NOTIFY changedFormFactor)
    Q_PROPERTY(Location location READ location NOTIFY changedLocation)
    Q_PROPERTY(unsigned int id READ id CONSTANT)
    Q_PROPERTY(QObject* applet READ applet CONSTANT) ///< return pointer to applet
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

    static FormFactor ToFormFactor(Plasma::FormFactor form) { return static_cast<FormFactor>(form); }
    static Location ToLocation(Plasma::Location loc) { return static_cast<Location>(loc); }

    explicit Plasmoid(Plasma::Applet *parent);
    virtual ~Plasmoid();

    FormFactor formFactor() const;
    void setFormFactor(FormFactor form_factor);
    Location   location() const;
    void setLocation(Location loc);
    unsigned int id() const;
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

    Q_INVOKABLE QVariant createShortcutAction(QString action_id) const;
    Q_INVOKABLE void updateShortcutAction(QVariant action, QString shortcut) const;
    Q_INVOKABLE void showMenu(QVariant menu, int x, int y, QVariant item) const;
    Q_INVOKABLE QPoint popupPosition(QVariant item, QSize size = QSize(0, 0), int align = Qt::AlignLeft) const;
    Q_INVOKABLE void destroyShortcutAction(QVariant action) const;
    Q_INVOKABLE void hideFromTaskbar(qulonglong win_id) const;
    Q_INVOKABLE QString getUniqueId(QObject *obj) const;

signals:
    void changedFormFactor();
    void changedLocation();
    void activated(); ///< If a plasmoid has been activated

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
    Plasma::Applet* m_applet;
    Plasmoid::FormFactor m_form;
    Plasmoid::Location   m_location;
    QSet<Task*> m_tasks;
};

} // namespace SystemTray

#endif // __SYSTEMTRAY__PLASMOID_H

