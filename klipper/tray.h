/* This file is part of the KDE project
   Copyright (C) by Andrew Stanley-Jones <asj@cban.com>
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef TRAY_H
#define TRAY_H

#include <QtCore/QPointer>

#include <KStatusNotifierItem>

class KNotification;
class Klipper;

class KlipperTray : public KStatusNotifierItem
{
    Q_OBJECT

public:
    KlipperTray();

public slots:
    void slotSetToolTipFromHistory();
    void slotPassivePopup(const QString& caption, const QString& text);

private:
    Klipper* m_klipper;
    QPointer<KNotification> m_notification;
};

#endif
