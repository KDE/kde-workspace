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
#include <qdir.h>

SvgViewer::SvgViewer(QWidget* parent)
    : KDialog(parent)
    , m_currentTheme(0)
{
    setWindowTitle(i18n("Plasma SVG Viewer"));

    QWidget* mainWidget = new QWidget(this);
    setMainWidget(mainWidget);
    setupUi(mainWidget);

    m_dataModel = new QStandardItemModel(this);

    connect(m_themeSelector, SIGNAL(currentIndexChanged(QString)), this, SLOT(loadTheme(QString)));

    m_svgFilesTree->setModel(m_dataModel);
    m_svgFilesTree->setWordWrap(true);

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

    delete m_currentTheme;
    m_currentTheme = new Plasma::Theme(themeName, this);

    QString filePath = m_themeMap.value(themeName).entryPath();

    kDebug() << "loaded theme has entry path: " << filePath;

    QFileInfo fileInfo = QFileInfo(filePath);
    QString directoryName = fileInfo.dir().dirName();

    kDebug() << "searching for resources/elements in dir: " << directoryName;

    QStringList themeElementList = KGlobal::dirs()->findAllResources("data", "desktoptheme/" + directoryName + "/*", KStandardDirs::Recursive);
    kDebug() << "$$$" << themeElementList;

//    m_dataModel->clear();
//    m_dataModel->setColumnCount(4);
//    QStringList headers;
//    headers << i18n("DataSource") << i18n("Key") << i18n("Value") << i18n("Type");
//    m_dataModel->setHorizontalHeaderLabels(headers);
//    m_engine = 0;
//    m_sourceCount = 0;
//
//    if (!m_engineName.isEmpty()) {
//        m_engineManager->unloadEngine(m_engineName);
//    }
//
//    m_engineName = name;
//    if (m_engineName.isEmpty()) {
//        updateTitle();
//        return;
//    }
//
//    m_engine = m_engineManager->loadEngine(m_engineName);
//    if (!m_engine) {
//        m_engineName.clear();
//        updateTitle();
//        return;
//    }
//
//    m_sourceRequesterButton->setEnabled(true);
//    m_updateInterval->setEnabled(true);
//    m_sourceRequester->setEnabled(true);
//    m_sourceRequester->setFocus();
//    m_serviceRequester->setEnabled(true);
//    m_serviceRequesterButton->setEnabled(true);
//    updateTitle();
}

#include "svgviewer.moc"
