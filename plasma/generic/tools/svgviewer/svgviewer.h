/*****************************************************************************
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

#include <KDialog>
#include <KDirWatch>

//internal
class PlasmaView;

class QLabel;
class QStandardItemModel;
class QStandardItem;
class QTreeView;
class QWidget;
class QScrollArea;
class QModelIndex;

class KTabWidget;
class KComboBox;
class KPluginInfo;

namespace Plasma {
    class Theme;
    class Svg;
}

class SvgViewer : public KDialog
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

private Q_SLOTS:
    /**
     * Watches the desktopthemes/ dir for any changes.
     * Upon changes, the cache is flushed and the theme
     * is reloaded
     */
    void themesDirty(const QString& file);

    void restartDirWatch();

private:
    void clearThemeCache();

//NOT NEEDED    QLabel *m_svgPreviewLabel;
    QLabel *m_svgFilesLabel;

    KComboBox *m_themeSelector;

    //------------- tab widget ------------
    KTabWidget *m_tabWidget;
    // first tab (embedded Plasma shell)
    QWidget *m_shellContainer;

    //-- second tab (tree view of theme elements)
    QTreeView *m_svgFilesTree;
    QWidget *m_scrollAreaContainer;
    QScrollArea *m_scrollArea;
    // a label that contains the pixmap of the current svg
    QLabel *m_svgPreviewImage;
    //-------------------------------------

    QStandardItemModel* m_dataModel;

    // FIXME: might not even need this map
    // or may need more added to it. we'll see.
    QMap <QString, KPluginInfo> m_themeMap;

    Plasma::Theme* m_currentTheme;
    Plasma::Svg* m_currentSvg;


    PlasmaView *m_plasmaView;

    KDirWatch *m_dirWatch;
};

#endif
