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
#include "../../host.h"
#include "debug.h"

#include <QtCore/QMetaEnum>
#include <kdeclarative/qmlobject.h>

#include <KPluginInfo>

#include <QLoggingCategory>

#include <Plasma/Applet>
#include <Plasma/PluginLoader>

namespace SystemTray
{

PlasmoidTask::PlasmoidTask(QQuickItem* rootItem, const QString &packageName, const QString &systrayPackageRoot, Plasma::Containment *cont, QObject *parent)
    : Task(parent),
      m_taskId(packageName),
      m_taskItem(0),
      m_rootItem(rootItem),
      m_valid(true)
{
    qCDebug(SYSTEMTRAY) << "Loading applet: " << packageName;
    //m_taskItem = new PlasmoidInterface(packageName, systrayPackageRoot, m_rootItem);

    m_taskItem = cont->createApplet(packageName);//Plasma::PluginLoader::self()->loadApplet(packageName);
    m_taskItem->init();
    m_taskGraphicsObject = m_taskItem->property("graphicObject").value<QQuickItem *>();

    if (m_taskGraphicsObject) {
        QMetaObject::invokeMethod(m_taskGraphicsObject, "init", Qt::DirectConnection);
        qWarning()<<m_taskGraphicsObject->property("compactRepresentationItem");
        qWarning()<<m_taskGraphicsObject->property("fullRepresentationItem");
        
        //old syntax, because we are connecting blindly
        connect(m_taskGraphicsObject, SIGNAL(expandedChanged(bool)),
                this, SLOT(syncExpanded(bool)));
    }



    if (!m_taskItem) {
        qCDebug(SYSTEMTRAY) << "Invalid applet taskitem";
        m_valid = false;
        return;
    }
    connect(m_taskItem, &Plasma::Applet::statusChanged, this, &PlasmoidTask::updateStatus);
   // connect(m_taskItem, &PlasmoidInterface::defaultRepresentationChanged, this, &PlasmoidTask::taskItemExpandedChanged);

    if (pluginInfo().isValid()) {
        setName(pluginInfo().name());
    } else {
        qWarning() << "Invalid Plasmoid: " << packageName;
    }
    updateStatus();
}

PlasmoidTask::~PlasmoidTask()
{
}

KPluginInfo PlasmoidTask::pluginInfo() const
{
    if (!m_taskItem) {
        return KPluginInfo();
    }
    return m_taskItem->pluginInfo();
}

void PlasmoidTask::updateStatus()
{
    if (!m_taskItem || !pluginInfo().isValid()) {
        return;
    }
    const Plasma::Types::ItemStatus ps = m_taskItem->status();
    if (ps == Plasma::Types::UnknownStatus) {
        setStatus(Task::UnknownStatus);
    } else if (ps == Plasma::Types::PassiveStatus) {
        setStatus(Task::Passive);
    } else if (ps == Plasma::Types::NeedsAttentionStatus) {
        setStatus(Task::NeedsAttention);
    } else {
        setStatus(Task::Active);
    }
}

void PlasmoidTask::expandApplet(bool expanded)
{
    qCDebug(SYSTEMTRAY) << "ST2P expandApplet() " << expanded;
    if (m_taskGraphicsObject) {
        m_taskGraphicsObject->setProperty("expanded", expanded);
    }
    //if (m_taskItem->isExpanded() != expanded) {
/*    if (m_taskItem && m_taskItem->isExpanded() != expanded) {
        qCDebug(SYSTEMTRAY) << "ST2P set plasmoid.expand = " << expanded;
        m_taskItem->setExpanded(expanded);
        //m_taskItem->setCollapsed();
    }*/
}

bool PlasmoidTask::isValid() const
{
    return m_valid && pluginInfo().isValid();
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

void PlasmoidTask::setLocation(Plasma::Types::Location loc)
{
    if (m_taskItem) {
//        m_taskItem->setLocation(loc);
    }
}

QString PlasmoidTask::taskId() const
{
    return m_taskId;
}

QQuickItem* PlasmoidTask::taskItem()
{
    if (m_taskGraphicsObject) {
        qWarning()<<"BBBBB"<<m_taskGraphicsObject<<m_taskGraphicsObject->property("title");
        return m_taskGraphicsObject;
    }
    return new QQuickItem();//m_taskItem;
}

QQuickItem* PlasmoidTask::taskItemExpanded()
{
    if (!m_taskItem) {
        return 0;
    }

    if (m_taskGraphicsObject && m_taskGraphicsObject->property("fullRepresentationItem").value<QQuickItem *>()) {
        return m_taskGraphicsObject->property("fullRepresentationItem").value<QQuickItem *>();
    }
    return new QQuickItem();//m_taskItem->defaultRepresentation();
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

bool PlasmoidTask::expanded() const
{
    if (m_taskGraphicsObject) {
        return m_taskGraphicsObject->property("expanded").toBool();
    } else {
        return false;
    }
}

void PlasmoidTask::syncExpanded(bool expanded)
{
    emit expandedChanged(expanded);
}

}

#include "plasmoidtask.moc"
