/***************************************************************************
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
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

#include "plasmoidtask.h"
#include "plasmoidprotocol.h"
#include "plasmoidinterface.h"
#include "declarative/packageaccessmanager.h"
#include "declarative/packageaccessmanagerfactory.h"
#include "declarative/packageurlinterceptor.h"


#include "../../manager.h"

#include <Plasma/PluginLoader>
#include <kdeclarative/qmlobject.h>
#include <klocalizedstring.h>
#include <kplugintrader.h>

#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

namespace SystemTray
{

PlasmoidProtocol::PlasmoidProtocol(QObject *parent)
    : Protocol(parent),
      m_tasks()
{
}

PlasmoidProtocol::~PlasmoidProtocol()
{
}

void PlasmoidProtocol::init()
{
    //X-Plasma-NotificationArea
    KPluginInfo::List applets = Plasma::PluginLoader::self()->listAppletInfo(QString());
    const QString constraint = QString("[X-Plasma-NotificationArea] == 'true'");
    //KPluginTrader::applyConstraints(applets, constraint);

    QStringList ownApplets;

    QStringList blacklist;
    blacklist << "notifier";
    blacklist << "org.kde.notifications";
    blacklist << "org.kde.systrayplasmoidtest";

    QMap<QString, KPluginInfo> sortedApplets;
    foreach (const KPluginInfo &info, applets) {
        KService::Ptr service = info.service();
        if (!blacklist.contains(info.pluginName()) && service->property("X-Plasma-NotificationArea", QVariant::Bool).toBool()) {
            // if we already have a plugin with this exact name in it, then check if it is the
            // same plugin and skip it if it is indeed already listed
            if (sortedApplets.contains(info.name())) {

                bool dupe = false;
                // it is possible (though poor form) to have multiple applets
                // with the same visible name but different plugins, so we hve to check all values
                foreach (const KPluginInfo &existingInfo, sortedApplets.values(info.name())) {
                    if (existingInfo.pluginName() == info.pluginName()) {
                        dupe = true;
                        break;
                    }
                }

                if (dupe) {
                    continue;
                }
            }

            // insertMulti becase it is possible (though poor form) to have multiple applets
            // with the same visible name but different plugins
            sortedApplets.insertMulti(info.name(), info);
        }
    }

    foreach (const KPluginInfo &info, sortedApplets) {
        //qDebug() << " Adding applet: " << info.name();
        qDebug() << "\n\n ==========================================================================================";
        if (!blacklist.contains(info.pluginName())) {
            newTask(info.pluginName());
        }
    }

}

// QmlObject* PlasmoidProtocol::loadPlasmoid(const QString &plugin, const QVariantHash &args, QQuickItem* parent)
// {
//     // Set up the runtime: security, url-based schemes, etc
//     QmlObject* qmlObject = new QmlObject(parent);
//     //qDebug() << " rootitem: " << rootItem->objectName();
//     qmlObject->setInitializationDelayed(true);
//
//
//     // Load the package
//     Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Applet");
//     pkg.setPath(plugin);
//
//     //use our own custom network access manager that will access Plasma packages and to manage security (i.e. deny access to remote stuff when the proper extension isn't enabled
//     QQmlEngine *engine = qmlObject->engine();
//     QQmlNetworkAccessManagerFactory *factory = engine->networkAccessManagerFactory();
//     engine->setNetworkAccessManagerFactory(0);
//     delete factory;
//     engine->setNetworkAccessManagerFactory(new PackageAccessManagerFactory(pkg));
//
//
//     //m_qmlObject->setSource(QUrl::fromLocalFile(m_appletScriptEngine->mainScript()));
//     //m_qmlObject->setSource(QUrl("/home/sebas/kf5/install/share/plasma/plasmoids/org.kde.systrayplasmoidtest/contents/ui/main.qml"));
//
//     //Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Applet");
//
//     //QString p = findPackageRoot("org.kde.microblog-qml", "plasma/plasmoids/");
//     //pkg.setDefaultPackageRoot(d->packageRoot);
//
//     //pkg.setPath(plugin);
//
//     //Hook generic url resolution to the applet package as well
//     //TODO: same thing will have to be done for every qqmlengine: PackageUrlInterceptor is material for plasmaquick?
//     engine->setUrlInterceptor(new PackageUrlInterceptor(engine, pkg));
//
//
//     const QString mainScript = pkg.filePath("mainscript");
//     qmlObject->setSource(QUrl::fromLocalFile(mainScript));
//
//     KPluginInfo i = pkg.metadata();
//     if (!i.isValid()) {
//         qDebug() << (i18n("Error: Can't find plugin metadata: %1", plugin));
//         qmlObject->completeInitialization(args);
//         return qmlObject;
//     }
//     qDebug() << (i18n("Showing info for package: %1", plugin));
//     qDebug() << (i18n("      Name : %1", i.name()));
//     qDebug() << (i18n("   Comment : %1", i.comment()));
//     qDebug() << (i18n("    Plugin : %1", i.pluginName()));
//     qDebug() << (i18n("    Author : %1", i.author()));
//     qDebug() << (i18n("      Path : %1", pkg.path()));
//     qDebug() << "mainScript:" << mainScript;
//
//
//
//
//     if (!qmlObject->engine() || !qmlObject->engine()->rootContext() || !qmlObject->engine()->rootContext()->isValid() || qmlObject->mainComponent()->isError()) {
//         QString reason;
//         foreach (QQmlError error, qmlObject->mainComponent()->errors()) {
//             reason += error.toString()+'\n';
//             qDebug() << error.toString();
//         }
//         reason = i18n("Error loading QML file: %1", reason);
//
//         //m_qmlObject->setSource(QUrl::fromLocalFile(applet()->containment()->corona()->package().filePath("appleterror")));
//         qmlObject->completeInitialization();
//
//
//         //even the error message QML may fail
//         if (qmlObject->mainComponent()->isError()) {
//             return 0;
//         } else {
//             qmlObject->rootObject()->setProperty("reason", reason);
//         }
//
//         //m_appletScriptEngine->setLaunchErrorMessage(reason);
//         qDebug() << "ERROR: " << reason;
//     }

//    AppletInterface* plasmoid = new AppletInterface(plugin, 0, parent);
    //AppletInterface* plasmoid = new AppletInterface(parent, qobject_cast<QQuickItem*>(qmlObject->rootObject()));
//     qmlObject->engine()->rootContext()->setContextProperty("plasmoid", plasmoid);
//
//     //initialize size, so an useless resize less
//     QVariantHash initialProperties;
//     //initialProperties["width"] = width();
//     //initialProperties["height"] = height();
//     qmlObject->completeInitialization(initialProperties);
//
//     //m_qmlObject->rootObject()->setParent(rootItem);
//     //m_taskItem->setProperty("parent", QVariant::fromValue(rootItem));
//     qDebug() << " Parent object : " << parent->objectName() << parent;
//     qDebug() << " Plasmoidobject: " << qmlObject->rootObject();
//     if (!qmlObject->rootObject()) {
//         qDebug() << " PROBLEM!";
//         foreach (QQmlError error, qmlObject->mainComponent()->errors()) {
//             //reason += error.toString()+'\n';
//             qDebug() << " ERROR: " << error.toString();
//         }
//
//     }
//     return qmlObject;
// }


void PlasmoidProtocol::newTask(const QString &service)
{
    qDebug() << "ST new task " << service;
    if (m_tasks.contains(service)) {
        return;
    }

    Manager* m = qobject_cast<Manager*>(parent());
    QQuickItem* rootItem = m->rootItem();

    PlasmoidTask *task = new PlasmoidTask(rootItem, service, this);

    m_tasks[service] = task;
    emit taskCreated(task);
}

void PlasmoidProtocol::cleanupTask(const QString &service)
{
    PlasmoidTask *task = m_tasks.value(service);

    if (task) {
        m_tasks.remove(service);
        if (task->isValid()) {
            emit task->destroyed(task);
        }
        task->deleteLater();
    }
}

}

#include "plasmoidprotocol.moc"
