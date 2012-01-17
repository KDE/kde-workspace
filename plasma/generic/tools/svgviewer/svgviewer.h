/*****************************************************************************
 *  This file is part of the KDE libraries                                    *
 *  Copyright (C) 2012 by Shaun Reich <shaun.reich@kdemail.net>               *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU Lesser General Public License as published  *
 *  by the Free Software Foundation; either version 2 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; see the file COPYING.LIB.                        *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

#ifndef SVGVIEWER_H
#define SVGVIEWER_H

#include "ui_svgviewer.h"

class QStandardItemModel;
class QStandardItem;

class KPluginInfo;

namespace Plasma
{
    class DataEngineManager;
    class DataEngine;
}

class SvgViewer : public KDialog, public Ui::SvgViewer
{
    Q_OBJECT

public:
    explicit SvgViewer(QWidget *parent = 0);
    ~SvgViewer();

public slots:
    void loadTheme(const QString& themeName);
    QStringList themeNames();

private:
    QStandardItemModel* m_dataModel;

    // TODO: make it cache the values, possibly use a
    // hashmap so we can get other infos too..unsure how to
    // lay it out tho
    //KPluginInfo::List m_themeList;
};

#endif

