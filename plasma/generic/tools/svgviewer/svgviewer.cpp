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

#include "svgviewer.h"

#include <QApplication>
#include <QDir>
#include <QStandardItemModel>

#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>
#include <KIconLoader>
#include <KIconTheme>
#include <KMenu>
#include <KStandardAction>
#include <KStringHandler>
#include <KAction>

#include <Plasma/Svg>
#include <Plasma/Theme>

SvgViewer::SvgViewer(QWidget* parent)
    : KDialog(parent)
    , m_currentTheme(0)
    , m_currentSvg(0)
{
    setWindowTitle(i18n("Plasma SVG Viewer"));

    QWidget* mainWidget = new QWidget(this);
    setMainWidget(mainWidget);
    setupUi(mainWidget);

    m_currentTheme = new Plasma::Theme(this);
    m_currentSvg = new Plasma::Svg(this);

    m_dataModel = new QStandardItemModel(this);

    connect(m_themeSelector, SIGNAL(currentIndexChanged(QString)), this, SLOT(loadTheme(QString)));

    m_svgFilesTree->setModel(m_dataModel);
    m_svgFilesTree->setWordWrap(true);

    connect(m_svgFilesTree, SIGNAL(activated(QModelIndex)), this, SLOT(modelIndexChanged(QModelIndex)));

    setButtons(KDialog::Close);

    addAction(KStandardAction::quit(qApp, SLOT(quit()), this));

    //TODO: connect to a signal or something?
    reloadThemeList();

    // find all theme names we know, populate combobox
    m_themeSelector->addItems(m_themeMap.keys());

//    connect(m_data, SIGNAL(customContextMenuRequested(QPoint)),
//            this, SLOT(showDataContextMenu(QPoint)));

    //m_svgFilesTree->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

SvgViewer::~SvgViewer()
{
    delete m_currentTheme;
    delete m_currentSvg;
}

void SvgViewer::reloadThemeList()
{
    m_themeMap.clear();

    const KPluginInfo::List& infoList = Plasma::Theme::listThemeInfo();
    foreach(const KPluginInfo& info, infoList) {
        m_themeMap.insert(info.name(), info);
        kDebug() << "added theme name and kplugininfo to hash: " << info.name();
    }

    //kDebug() <<    KGlobal::dirs()->findDirs("data", "desktoptheme");
    //KGlobal::dirs()->findAllResources("data", "desktoptheme/*");
}

void SvgViewer::loadTheme(const QString& themeName)
{
    kDebug() << "begin loading theme: " << themeName;

    m_currentTheme->setThemeName(themeName);
    m_currentSvg->setTheme(m_currentTheme);

    // wipe model prepare to reload
    m_dataModel->clear();
    m_dataModel->setColumnCount(1);

    QStringList headers;
    headers << i18n("SVG Elements");
    m_dataModel->setHorizontalHeaderLabels(headers);

    QString filePath = m_themeMap.value(themeName).entryPath();

    kDebug() << "loaded theme has entry path: " << filePath;

    // only interested in the parent dir's name.
    // e.g. ascii, Air, etc.
    QFileInfo fileInfo = QFileInfo(filePath);
    QString directoryName = fileInfo.dir().dirName();

    kDebug() << "searching for resources/elements in dir: " << directoryName;

    const QStringList themeElementList =
        KGlobal::dirs()->findAllResources("data", "desktoptheme/" + directoryName + "/*/*.svgz", KStandardDirs::Recursive);

    kDebug() << "theme element list, begin populating: " << themeElementList;

    foreach (const QString& elementFullPath, themeElementList) {
        QFileInfo file = QFileInfo(elementFullPath);

        //TODO: add full path as a different header?
        // produces rows looking like "widgets/viewitem.svgz", "lancelot/..." etc.
        QStandardItem *item = new QStandardItem(file.dir().dirName() + '/' + file.baseName());

        m_dataModel->appendRow(item);
    }
}

void SvgViewer::modelIndexChanged(const QModelIndex& index)
{
    const QString& elementPath = m_dataModel->item(index.row())->text();

    kDebug() << "modelIndexChanged loading svg theme: " << m_currentTheme->themeName();
    m_currentSvg->setTheme(m_currentTheme);

    kDebug() << "modelIndexChanged, loading svg elementPath: " << elementPath;
    m_currentSvg->setImagePath(elementPath);
    m_svgPreview->setPixmap(m_currentSvg->pixmap());
}

#include "svgviewer.moc"
