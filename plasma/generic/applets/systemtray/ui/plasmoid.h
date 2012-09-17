/***********************************************************************************************************************
 * System Tray (KDE Plasmoid)
 * Copyright (C) 2011-2012 ROSA  <support@rosalab.ru>
 * License: GPLv3+
 * Authors: Dmitry Ashkadov <dmitry.ashkadov@rosalab.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **********************************************************************************************************************/

#ifndef __SYSTEMTRAY__PLASMOID_H
#define __SYSTEMTRAY__PLASMOID_H


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
#include <QtCore/QObject>

#include <KDE/Plasma/Plasma>


namespace SystemTray
{

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
    Q_PROPERTY(QVariant applet READ applet CONSTANT) ///< return pointer to applet
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

    explicit Plasmoid(QObject *parent = 0);
    virtual ~Plasmoid();

    FormFactor formFactor() const;
    void setFormFactor(FormFactor form_factor);
    Location   location() const;
    void setLocation(Location loc);
    unsigned int id() const;
    QVariant applet() const;

    Q_INVOKABLE QVariant createShortcutAction(QString action_id) const;
    Q_INVOKABLE void updateShortcutAction(QVariant action, QString shortcut) const;
    Q_INVOKABLE void showMenu(QVariant menu, int x, int y, QVariant item) const;
    Q_INVOKABLE QPoint popupPosition(QVariant item, QSize size = QSize(0, 0), int align = Qt::AlignLeft) const;
    Q_INVOKABLE void destroyShortcutAction(QVariant action) const;

signals:
    void changedFormFactor();
    void changedLocation();
    void activated(); ///< If a plasmoid has been activated

private:
    struct _Private;
    _Private * const d;
};

} // namespace SystemTray

#endif // __SYSTEMTRAY__PLASMOID_H

