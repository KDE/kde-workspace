/* Copyright 2009  <Jan Gerrit Marker> <jangerrit@weiler-marker.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KILLRUNNERCONFIG_H
#define KILLRUNNERCONFIG_H

//Project-Includes
#include "ui_killrunner_config.h"
//KDE-Includes
#include <KCModule>
//Qt

static const char CONFIG_USE_TRIGGERWORD[] = "useTriggerWord";
static const char CONFIG_TRIGGERWORD[] = "triggerWord";
static const char CONFIG_SORTING[] = "sorting";

class KillRunnerConfigForm : public QWidget, public Ui::KillRunnerConfigUi
{
    Q_OBJECT

public:
    explicit KillRunnerConfigForm(QWidget* parent);
};

class KillRunnerConfig : public KCModule
{
    Q_OBJECT

public:
    explicit KillRunnerConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());
    
    /** Possibilities to sort */
    enum Sort {NONE = 0, CPU, CPUI};

public Q_SLOTS:
    void save();
    void load();
    void defaults();

private:
    KillRunnerConfigForm* m_ui;
};
#endif
