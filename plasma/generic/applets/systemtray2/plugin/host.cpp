/***************************************************************************
 *                                                                         *
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


#include "host.h"
#include "manager.h"
#include "task.h"

#include <klocalizedstring.h>

#include <Plasma/Package>
#include <Plasma/PluginLoader>

#include <QDebug>
#include <QQuickItem>
#include <QTimer>
#include <QVariant>
#include <QQmlContext>
#include <QQmlEngine>

namespace SystemTray
{

Manager *SystemTray::Host::s_manager = 0;
int SystemTray::Host::s_managerUsage = 0;

class HostPrivate {
public:
    Host *q;
    QList<SystemTray::Task*> tasks;
    QmlObject* notificationsPlasmoid = 0;
};

Host::Host(QObject* parent) :
    QObject(parent)
{
    d = new HostPrivate;
    init();
}

Host::~Host()
{
    delete d;
}

QmlObject* Host::loadPlasmoid(const QString &plugin, const QVariantHash &args, QQuickItem* parent)
{
    // Set up the runtime: security, url-based schemes, etc
    QmlObject* m_qmlObject = new QmlObject(parent);
    //qDebug() << " rootitem: " << rootItem->objectName();
    m_qmlObject->setInitializationDelayed(true);
    //use our own custom network access manager that will access Plasma packages and to manage security (i.e. deny access to remote stuff when the proper extension isn't enabled
    QQmlEngine *engine = m_qmlObject->engine();
//         QQmlNetworkAccessManagerFactory *factory = engine->networkAccessManagerFactory();
//         engine->setNetworkAccessManagerFactory(0);
//         delete factory;
//         engine->setNetworkAccessManagerFactory(new PackageAccessManagerFactory(m_appletScriptEngine->package()));

    //Hook generic url resolution to the applet package as well
    //TODO: same thing will have to be done for every qqmlengine: PackageUrlInterceptor is material for plasmaquick?
//         engine->setUrlInterceptor(new PackageUrlInterceptor(engine, m_appletScriptEngine->package()));

    // Load the package
    //m_qmlObject->setSource(QUrl::fromLocalFile(m_appletScriptEngine->mainScript()));
    //m_qmlObject->setSource(QUrl("/home/sebas/kf5/install/share/plasma/plasmoids/org.kde.systrayplasmoidtest/contents/ui/main.qml"));

    Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Applet");

    //QString p = findPackageRoot("org.kde.microblog-qml", "plasma/plasmoids/");
    //pkg.setDefaultPackageRoot(d->packageRoot);

    pkg.setPath(plugin);
    const QString mainScript = pkg.filePath("mainscript");
    m_qmlObject->setSource(QUrl::fromLocalFile(mainScript));

    KPluginInfo i = pkg.metadata();
    if (!i.isValid()) {
        qDebug() << (i18n("Error: Can't find plugin metadata: %1", plugin));
    }
    qDebug() << (i18n("Showing info for package: %1", plugin));
    qDebug() << (i18n("      Name : %1", i.name()));
    qDebug() << (i18n("   Comment : %1", i.comment()));
    qDebug() << (i18n("    Plugin : %1", i.pluginName()));
    qDebug() << (i18n("    Author : %1", i.author()));
    qDebug() << (i18n("      Path : %1", pkg.path()));
    qDebug() << "mainScript:" << mainScript;




    if (!m_qmlObject->engine() || !m_qmlObject->engine()->rootContext() || !m_qmlObject->engine()->rootContext()->isValid() || m_qmlObject->mainComponent()->isError()) {
        QString reason;
        foreach (QQmlError error, m_qmlObject->mainComponent()->errors()) {
            reason += error.toString()+'\n';
            qDebug() << error.toString();
        }
        reason = i18n("Error loading QML file: %1", reason);

        //m_qmlObject->setSource(QUrl::fromLocalFile(applet()->containment()->corona()->package().filePath("appleterror")));
        m_qmlObject->completeInitialization();


        //even the error message QML may fail
        if (m_qmlObject->mainComponent()->isError()) {
            return 0;
        } else {
            m_qmlObject->rootObject()->setProperty("reason", reason);
        }

        //m_appletScriptEngine->setLaunchErrorMessage(reason);
        qDebug() << "ERROR: " << reason;
    }

    //m_qmlObject->engine()->rootContext()->setContextProperty("plasmoid", this);

    //initialize size, so an useless resize less
    QVariantHash initialProperties;
    //initialProperties["width"] = width();
    //initialProperties["height"] = height();
    m_qmlObject->completeInitialization(initialProperties);

    //m_qmlObject->rootObject()->setParent(rootItem);
    //m_taskItem->setProperty("parent", QVariant::fromValue(rootItem));
    qDebug() << " Parent object : " << parent->objectName() << parent;
    qDebug() << " Plasmoidobject: " << m_qmlObject->rootObject();
    return m_qmlObject;
}

QQuickItem* Host::notificationsPlasmoid()
{
    if (!d->notificationsPlasmoid) {
        const QString plugin = QStringLiteral("org.kde.notifications");
        //const QString plugin = QStringLiteral("org.kde.systrayplasmoidtest");
        d->notificationsPlasmoid = Host::loadPlasmoid(plugin, QVariantHash(),
                                                      s_manager->rootItem());
        if (!d->notificationsPlasmoid) {
            return new QQuickItem(s_manager->rootItem());
        }
    }
    return qobject_cast<QQuickItem*>(d->notificationsPlasmoid->rootObject());
}


void Host::init()
{
    if (!s_manager) {
        s_manager = new SystemTray::Manager();
        connect(s_manager, SIGNAL(tasksChanged()), this, SIGNAL(tasksChanged()));
    }
    ++s_managerUsage;
    emit tasksChanged();
}

void Host::setRootItem(QQuickItem* rootItem)
{
    qDebug() << "Set root item";
    s_manager->setRootItem(rootItem);
}


QQmlListProperty<SystemTray::Task> Host::tasks()
{
    if (s_manager) {
        // We need to keep a reference to the list we're passing to the runtime
        d->tasks = s_manager->tasks();
        QQmlListProperty<SystemTray::Task> l(this, d->tasks);
        return l;
    }
    QQmlListProperty<SystemTray::Task> l;
    return l;
}

} // namespace

#include "host.moc"
