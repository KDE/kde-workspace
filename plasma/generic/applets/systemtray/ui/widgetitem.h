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


#ifndef __SYSTEMTRAY__WIDGETITEM_H
#define __SYSTEMTRAY__WIDGETITEM_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includess
#include "../core/task.h"

#include <QtDeclarative/QDeclarativeItem>

#include <KDE/Plasma/Applet>

namespace SystemTray
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class WidgetItem
/** @class WidgetItem
 * Represents declarative item containing an specified graphics widget.
 */
class WidgetItem: public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QObject* applet READ applet WRITE setApplet) ///< host applet
    Q_PROPERTY(QObject* task READ task WRITE setTask NOTIFY changedTask) ///< task
public:
    explicit WidgetItem(QDeclarativeItem *parent = 0);
    virtual ~WidgetItem();

public:
    QObject *task() const { return m_task.data(); }
    void setTask(QObject *task);

    /**
     * @return applet as a host for widget
     */
    QObject *applet() const { return m_applet; }
    void setApplet(QObject *applet);

signals:
    void changedTask();

private Q_SLOTS:
    void afterWidthChanged();
    void afterHeightChanged();

private:
    void bind();
    void unbind();

    Plasma::Applet *m_applet;
    QWeakPointer<Task> m_task;
};

} //namespace SystemTray

#endif // __SYSTEMTRAY__WIDGETITEM_H
