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

#include <KIconLoader>
#include <KIconTheme>
#include <KMenu>
#include <KStandardAction>
#include <KStringHandler>
#include <KAction>

EngineExplorer::EngineExplorer(QWidget* parent)
    : KDialog(parent),
{
    setWindowTitle(i18n("Plasma Engine Explorer"));
    QWidget* mainWidget = new QWidget(this);
    setMainWidget(mainWidget);
    setupUi(mainWidget);

    m_dataModel = new QStandardItemModel(this);
    KIcon pix("plasma");
    int size = IconSize(KIconLoader::Dialog);
    m_title->setPixmap(pix.pixmap(size, size));

    connect(m_engines, SIGNAL(activated(QString)), this, SLOT(showEngine(QString)));

    connect(m_sourceRequesterButton, SIGNAL(clicked(bool)), this, SLOT(requestSource()));
    connect(m_serviceRequesterButton, SIGNAL(clicked(bool)), this, SLOT(requestServiceForSource()));
    m_data->setModel(m_dataModel);
    m_data->setWordWrap(true);

    m_searchLine->setTreeView(m_data);
    m_searchLine->setClickMessage(i18n("Search"));

   addAction(KStandardAction::quit(qApp, SLOT(quit()), this));

    connect(m_data, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showDataContextMenu(QPoint)));
    m_data->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

EngineExplorer::~EngineExplorer()
{
}

void EngineExplorer::setEngine(const QString &engine)
{
    //find the engine in the combo box
    int index = m_engines->findText(engine);
    if (index != -1) {
        kDebug() << QString("Engine %1 found!").arg(engine);
        m_engines->setCurrentIndex(index);
        showEngine(engine);
    }
}

void EngineExplorer::listEngines()
{
    m_engines->clear();
    KPluginInfo::List engines = m_engineManager->listEngineInfo(m_app);
    qSort(engines);

    foreach (const KPluginInfo engine, engines) {
        m_engines->addItem(KIcon(engine.icon()), engine.pluginName());
    }

    m_engines->setCurrentIndex(-1);
}

void EngineExplorer::showEngine(const QString& name)
{
    m_sourceRequester->setEnabled(false);
    m_sourceRequesterButton->setEnabled(false);
    m_serviceRequester->setEnabled(false);
    m_serviceRequesterButton->setEnabled(false);

    m_dataModel->clear();
    m_dataModel->setColumnCount(4);
    QStringList headers;
    headers << i18n("DataSource") << i18n("Key") << i18n("Value") << i18n("Type");
    m_dataModel->setHorizontalHeaderLabels(headers);
    m_engine = 0;
    m_sourceCount = 0;

    if (!m_engineName.isEmpty()) {
        m_engineManager->unloadEngine(m_engineName);
    }

    m_engineName = name;
    if (m_engineName.isEmpty()) {
        updateTitle();
        return;
    }

    m_engine = m_engineManager->loadEngine(m_engineName);
    if (!m_engine) {
        m_engineName.clear();
        updateTitle();
        return;
    }

    m_sourceRequesterButton->setEnabled(true);
    m_updateInterval->setEnabled(true);
    m_sourceRequester->setEnabled(true);
    m_sourceRequester->setFocus();
    m_serviceRequester->setEnabled(true);
    m_serviceRequesterButton->setEnabled(true);
    updateTitle();
}

void EngineExplorer::addSource(const QString& source)
{
    kDebug() << "adding" << source;
    QList<QStandardItem*> items = m_dataModel->findItems(source, 0);
    if (!items.isEmpty()) {
        kDebug() << "er... already there?";
        return;
    }

    QStandardItem* parent = new QStandardItem(source);
    m_dataModel->appendRow(parent);

    kDebug() << "getting data for source " << source;

    if (!m_requestingSource || m_sourceRequester->text() != source) {
        kDebug() << "connecting up now";
        m_engine->connectSource(source, this);
    }

    ++m_sourceCount;
    updateTitle();

    enableButton(KDialog::User1, true);
    enableButton(KDialog::User2, true);
}

void EngineExplorer::removeSource(const QString& source)
{
    QList<QStandardItem*> items = m_dataModel->findItems(source, 0);

    if (items.count() < 1) {
        return;
    }

    foreach (QStandardItem* item, items) {
        m_dataModel->removeRow(item->row());
    }

    --m_sourceCount;
    m_engine->disconnectSource(source, this);
    updateTitle();
}

void EngineExplorer::showDataContextMenu(const QPoint &point)
{
    QModelIndex index = m_data->indexAt(point);
    if (index.isValid()) {
        if (index.parent().isValid()) {
            index = index.parent();
        }

        if (index.column() != 0) {
            index = m_dataModel->index(index.row(), 0);
        }

        const QString source = index.data().toString();
        KMenu menu;
        menu.addTitle(source);
        QAction *service = menu.addAction(i18n("Get associated service"));
        QAction *update = menu.addAction(i18n("Update source now"));
        QAction *remove = menu.addAction(i18n("Remove source"));

        QAction *activated = menu.exec(m_data->viewport()->mapToGlobal(point));
        if (activated == service) {
            ServiceViewer *viewer = new ServiceViewer(m_engine, source);
            viewer->show();
        } else if (activated == update) {
            m_engine->connectSource(source, this);
            Plasma::DataEngine::Data data = m_engine->query(source);
        } else if (activated == remove) {
            removeSource(source);
        }
    }
}

void EngineExplorer::updateTitle()
{
    if (!m_engine) {
        m_title->setPixmap(KIcon("plasma").pixmap(IconSize(KIconLoader::Dialog)));
        m_title->setText(i18n("Plasma DataEngine Explorer"));
        return;
    }

    m_title->setText(ki18ncp("The name of the engine followed by the number of data sources",
                             "%1 Engine - 1 data source", "%1 Engine - %2 data sources")
                              .subs(KStringHandler::capwords(m_engine->name()))
                              .subs(m_sourceCount).toString());

    if (m_engine->icon().isEmpty()) {
        m_title->setPixmap(KIcon("plasma").pixmap(IconSize(KIconLoader::Dialog)));
    } else {
        //m_title->setPixmap(KIcon("alarmclock").pixmap(IconSize(KIconLoader::Dialog)));
        m_title->setPixmap(KIcon(m_engine->icon()).pixmap(IconSize(KIconLoader::Dialog)));
    }
}

#include "svgviewer.moc"

