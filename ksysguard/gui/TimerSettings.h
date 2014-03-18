/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KSG_TIMERSETTINGS_H
#define KSG_TIMERSETTINGS_H

#include <QLabel>

#include <kdialog.h>

class QCheckBox;
class QDoubleSpinBox;

class Q_DECL_EXPORT TimerSettings : public KDialog
{
  Q_OBJECT

  public:
    explicit TimerSettings( QWidget *parent, const char *name = 0 );
    ~TimerSettings();

    void setUseGlobalUpdate( bool value );
    bool useGlobalUpdate() const;

    void setInterval( double interval );
    double interval() const;

  private Q_SLOTS:
    void globalUpdateChanged( bool );

  private:
    QCheckBox* mUseGlobalUpdate;
    QLabel* mLabel;
    QDoubleSpinBox* mInterval;
};

#endif
