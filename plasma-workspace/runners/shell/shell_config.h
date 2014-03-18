/***************************************************************************
 *   Copyright 2008 by Montel Laurent <montel@kde.org>                     *
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

#ifndef SHELLCONFIG_H
#define SHELLCONFIG_H

#include <QWidget>

#include <KConfigGroup>

#include "ui_shellOptions.h"

class ShellConfig : public QWidget
{
    Q_OBJECT
    public:
        explicit ShellConfig(const KConfigGroup &config, QWidget* parent = 0);
        ~ShellConfig();

        Ui::shellOptions m_ui;

    protected Q_SLOTS:
        void slotUpdateUser(bool);

    private:
        KConfigGroup m_config;
};

#endif
