/***************************************************************************
 *   Copyright (C) 2012 by Aleix Pol Gonzalez <aleixpol@kde.org>           *
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

#include "qmlrunnerview.h"

#include <KStandardDirs>
#include <KDialog>
#include <KPluginSelector>
#include <Plasma/RunnerManager>
#include <kdeclarative.h>
#include <kwindowsystem.h>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDeclarativeItem>
#include <qdeclarative.h>
#include <QLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QDebug>
#include "runnersettings.h"

//NOTE: this manager here is not going to be used, because the qml
//implementation uses the RunnerModel, we'll see how to fix that...
qmlrunnerView::qmlrunnerView(Plasma::RunnerManager* manager, QWidget *parent)
    : KRunnerDialog(manager, parent)
{
    view = new QDeclarativeView(this);
    view->setAttribute(Qt::WA_TranslucentBackground);
    view->setInteractive(true);
    view->setStyleSheet("background: transparent");
    QPalette pal = view->palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    view->setPalette(pal);
    
    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(view->engine());
    kdeclarative.initialize();
    //binds things like kconfig and icons
    kdeclarative.setupBindings();
    
    view->rootContext()->setContextProperty("app", this);
    QString src = KStandardDirs::locate("data", "qmlrunner/qml/main.qml");
    Q_ASSERT(!src.isEmpty());
    view->setSource(QUrl::fromLocalFile(src));
    view->setResizeMode(QDeclarativeView::SizeViewToRootObject);
    if(!view->errors().isEmpty())
        qDebug() << "initialization errors:" << view->errors();
    Q_ASSERT(view->errors().isEmpty());
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(view);
    
    connect(view->rootObject(), SIGNAL(heightChanged()), SLOT(changeGeometry()));
    connect(qobject_cast<QDeclarativeItem*>(view->rootObject()), SIGNAL(implicitHeightChanged()), SLOT(changeGeometry()));
    connect(qobject_cast<QDeclarativeItem*>(view->rootObject()), SIGNAL(implicitWidthChanged()), SLOT(changeGeometry()));
}

qmlrunnerView::~qmlrunnerView()
{}

void qmlrunnerView::configure()
{
    toggleConfigDialog();
}

void qmlrunnerView::closeSettings()
{
//     if(m_config->result()==QDialog::Accepted) {
//         m_config->m_sel->save();
//     }
//     m_config->deleteLater();
//     m_config = 0;
}

RunnerSettings* qmlrunnerView::settings()
{
    return 0;
}

void qmlrunnerView::clearHistory()
{}

void qmlrunnerView::display(const QString& term)
{
    KWindowSystem::setOnDesktop(winId(), KWindowSystem::currentDesktop());
    emit changeTerm(term);
    show();
    KWindowSystem::forceActiveWindow(winId());
}

void qmlrunnerView::setConfigWidget(QWidget* w)
{
    KDialog *d = new KDialog(this);
    connect(w, SIGNAL(destroyed(QObject*)), d, SLOT(deleteLater()));
    d->setMainWidget(w);
    d->setButtons(0);
    d->show();
}

void qmlrunnerView::changeGeometry()
{
    QDeclarativeItem* it = qobject_cast<QDeclarativeItem*>(view->rootObject());
    qDebug() << "sizeeeeee" << it->height() << it->width();
    int w = width();
    
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    resize(w, it->height()+top+bottom);
}
