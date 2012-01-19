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

namespace Plasma {
    class Theme;
    class Svg;
}

class SvgViewer : public KDialog, public Ui::SvgViewer
{
    Q_OBJECT

public:
    explicit SvgViewer(QWidget *parent = 0);
    ~SvgViewer();

public slots:
    void loadTheme(const QString& themeName);

    void reloadThemeList();

    void modelIndexChanged(const QModelIndex& index);
    void modelSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

private:
    QStandardItemModel* m_dataModel;

    // FIXME: might not even need this map
    // or may need more added to it. we'll see.
    QMap <QString, KPluginInfo> m_themeMap;

    Plasma::Theme* m_currentTheme;
    Plasma::Svg* m_currentSvg;
};

#endif

