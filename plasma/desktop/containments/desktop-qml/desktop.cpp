/***************************************************************************
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>                    *
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

#include "desktop.h"

#include <QtDeclarative>
//#include <QDeclarativeItem>

#include <KDebug>
#include <KConfigDialog>
#include <KConfigGroup>

#include <Plasma/DeclarativeWidget>
#include <Plasma/Package>
#include <Plasma/PluginLoader>

Desktop::Desktop(QObject *parent, const QVariantList &args)
    : Plasma::Containment(parent, args),
    m_declarativeWidget(0)
{
    kDebug() << "Desktop Ctor";
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

Desktop::~Desktop()
{
}

void Desktop::init()
{
    setWallpaper("image");
    setDrawWallpaper(true);
    configChanged();

    const QString p = "org.kde.plasma.desktopcontainment";
    Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
    Plasma::Package package = Plasma::Package(QString(), "org.kde.plasma.desktopcontainment", structure);
    kDebug() << " path f package " << package.path();
    const QString qmlFile = package.filePath("ui", "Desktop.qml");
    //delete package;

    kDebug() << " Loading QML File from package: " << qmlFile;

    m_declarativeWidget = new Plasma::DeclarativeWidget(this);
    QGraphicsLinearLayout *l = new QGraphicsLinearLayout(this);
    l->addItem(m_declarativeWidget);


    setLayout(l);
//     connect(m_declarativeWidget->mainComponent(), SIGNAL(statusChanged(QDeclarativeComponent::Status)), SLOT(connectObjects(QDeclarativeComponent::Status)));

    // qmlRegisterType<DirectoryLister>("org.kde.plasma.desktopcontainment", 0, 1, "DirectoryLister");

    m_declarativeWidget->setQmlPath(qmlFile);
    m_declarativeWidget->setMinimumSize(220, 250);
    //QTimer::singleShot(1000, this, SLOT(connectObjects()));
}


void Desktop::configChanged()
{
    // ...
}

void Desktop::configAccepted()
{
}

void Desktop::createConfigurationInterface(KConfigDialog *parent)
{
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

#include "desktop.moc"
