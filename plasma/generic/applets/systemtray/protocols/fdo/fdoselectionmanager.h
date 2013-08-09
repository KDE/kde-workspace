/***************************************************************************
 *   fdoselectionmanager.h                                                 *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#ifndef FDOSELECTIONMANAGER_H
#define FDOSELECTIONMANAGER_H

#include <QtGui/QWidget>

namespace SystemTray
{

class Notification;
class Task;
class X11EmbedPainter;
class FdoSelectionManagerPrivate;

class FdoSelectionManager : public QWidget
{
    Q_OBJECT

public:
    static FdoSelectionManager *manager();
    static X11EmbedPainter *painter();

    FdoSelectionManager();
    ~FdoSelectionManager();

    void addDamageWatch(QWidget *container, WId client);
    void removeDamageWatch(QWidget *container);
    bool haveComposite() const;

Q_SIGNALS:
    void taskCreated(SystemTray::Task *task);
    void notificationCreated(SystemTray::Notification *notification);

protected:
    bool x11Event(XEvent *event);

private Q_SLOTS:
    void initSelection();
    void cleanupTask(WId winId);

private:
    friend class FdoSelectionManagerPrivate;
    FdoSelectionManagerPrivate* const d;
};

}

#endif
