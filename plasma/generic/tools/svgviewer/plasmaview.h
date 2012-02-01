/*****************************************************************************
 *   Copyright (C) 2012 by Shaun Reich <shaun.reich@kdemail.net>              *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU General Public License as           *
 *   published by the Free Software Foundation; either version 2 of           *
 *   the License, or (at your option) any later version.                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/

#ifndef PLASMAVIEW_H
#define PLASMAVIEW_H

#include <Plasma/Applet>

#include <Plasma/Corona>

#include <QGraphicsView>

class QTimer;

class PlasmaView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit PlasmaView(QWidget *parent = 0);
    ~PlasmaView();

    void addApplet(const QString &name, const QString& containment, const QVariantList &args = QVariantList());

private Q_SLOTS:
    void sceneRectChanged(const QRectF &rect);

private:
    KConfigGroup storageGroup(Plasma::Applet *applet) const;
    bool hasStorageGroupFor(Plasma::Applet *applet) const;
    void storeCurrentApplet();

    Plasma::Corona m_corona;
    Plasma::FormFactor m_formfactor;
    Plasma::Location m_location;
    Plasma::Containment *m_containment;
    Plasma::Applet *m_applet;
};

#endif

