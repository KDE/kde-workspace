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

#include <klocalizedstring.h>

#include <QtCore/QMetaEnum>
#include <QQmlContext>
#include <QQmlEngine>
#include <kdeclarative/qmlobject.h>

#include <QDebug>

namespace SystemTray
{

PlasmoidTask::PlasmoidTask(QQuickItem* rootItem, const QString &packageName, QObject *parent)
    : Task(parent),
      m_taskId(packageName),
      m_taskItem(0),
      m_rootItem(rootItem),
      m_valid(false)
{
    //qDebug();
#if 0
        QmlObject *qmlobject = new QmlObject(m_rootItem);
        qmlobject->rootObject()->setParent(m_rootItem);
        qmlobject->setInitializationDelayed(true);
        qmlobject->setSource(QUrl("/home/sebas/kf5/install/share/plasma/plasmoids/org.kde.systrayplasmoidtest/contents/ui/main.qml"));
        QObject *myObject = qmlobject->mainComponent()->create();
        m_taskItem = qobject_cast<QQuickItem*>(myObject);
        QString oname = m_taskItem->objectName();

        qDebug() << " :) Created: " << oname;
#endif
    m_qmlObject = new QmlObject(rootItem);
    qDebug() << " rootitem: " << rootItem->objectName();
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


    //m_qmlObject->setSource(QUrl::fromLocalFile(m_appletScriptEngine->mainScript()));
    m_qmlObject->setSource(QUrl("/home/sebas/kf5/install/share/plasma/plasmoids/org.kde.systrayplasmoidtest/contents/ui/main.qml"));

    if (!m_qmlObject->engine() || !m_qmlObject->engine()->rootContext() || !m_qmlObject->engine()->rootContext()->isValid() || m_qmlObject->mainComponent()->isError()) {
        QString reason;
        foreach (QQmlError error, m_qmlObject->mainComponent()->errors()) {
            reason += error.toString()+'\n';
        }
        reason = i18n("Error loading QML file: %1", reason);

        //m_qmlObject->setSource(QUrl::fromLocalFile(applet()->containment()->corona()->package().filePath("appleterror")));
        m_qmlObject->completeInitialization();


        //even the error message QML may fail
        if (m_qmlObject->mainComponent()->isError()) {
            return;
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

    qDebug() << " rootitem: " << rootItem->objectName();
    //m_qmlObject->rootObject()->setParent(rootItem);
    //m_taskItem->setProperty("parent", QVariant::fromValue(rootItem));
    qDebug() << "ST m_rootItem :: " << m_rootItem->objectName() << m_qmlObject->rootObject();

    //QObject *myObject = m_qmlObject->mainComponent()->create();
    m_taskItem = qobject_cast<QQuickItem*>(m_qmlObject->rootObject());
    //m_taskItem = m_qmlObject->rootObject();
    QString oname = m_taskItem->objectName();
    qDebug() << " ONAME: " << oname << m_taskItem;
    //qDebug() << "Done .";
    emit taskItemChanged();
}

PlasmoidTask::~PlasmoidTask()
{
}

bool PlasmoidTask::isValid() const
{
    return m_valid;
}

bool PlasmoidTask::isEmbeddable() const
{
    return false; // this task cannot be embed because it only provides information to GUI part
}

bool PlasmoidTask::isWidget() const
{
    return false; // isn't a widget
}

void PlasmoidTask::setShortcut(QString text) {
    if (m_shortcut != text) {
        m_shortcut = text;
        emit changedShortcut();
    }
}


QString PlasmoidTask::taskId() const
{
    return m_taskId;
}

QQuickItem* PlasmoidTask::taskItem()
{
    if (!m_taskItem) {

    }
    return m_taskItem;
}

QIcon PlasmoidTask::icon() const
{
    return m_icon;
}
//Status

void PlasmoidTask::syncStatus(QString newStatus)
{
    Task::Status status = (Task::Status)metaObject()->enumerator(metaObject()->indexOfEnumerator("Status")).keyToValue(newStatus.toLatin1());

    if (this->status() == status) {
        return;
    }

    setStatus(status);
}

}

#include "plasmoidtask.moc"
