/*
 *   Copyright © 2010 Andreas Demmer <ademmer@opensuse.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; see the file COPYING.  if not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */

#ifndef KWIN_DASHBOARD_CONFIG_H
#define KWIN_DASHBOARD_CONFIG_H

#include <KCModule>
#include "ui_dashboard_config.h"

namespace KWin
{

class DashboardEffectConfig : public KCModule
{
    Q_OBJECT

public:
    explicit DashboardEffectConfig(QWidget *parent = 0, const QVariantList& args = QVariantList());
    ~DashboardEffectConfig();

    void save();

private:
    bool isBlurEffectAvailable();
    long net_wm_dashboard;
    ::Ui::DashboardEffectConfig ui;

};

} // namespace KWin

#endif

