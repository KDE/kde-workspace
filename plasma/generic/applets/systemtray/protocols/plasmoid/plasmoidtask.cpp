/***************************************************************************
 *   plasmoidtask.cpp                                                      *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
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
#include <fixx11h.h>

#include <KIcon>
#include <KIconLoader>

#include <Plasma/Applet>
#include <Plasma/PluginLoader>

#include <QQuickItem>
#include <QDebug>

namespace SystemTray
{

PlasmoidTask::PlasmoidTask(const QString &appletname, int id, QObject *parent, Plasma::Applet *host)
    : Task(parent),
      m_appletName(appletname),
      m_taskId(appletname),
      m_host(host),
      m_takenByParent(false)
{
    setName(appletname);
    setupApplet(appletname, id);
}


PlasmoidTask::~PlasmoidTask()
{
    emit taskDeleted(m_host, m_taskId);
}


bool PlasmoidTask::isEmbeddable() const
{
    return m_applet && !m_takenByParent;
}

bool PlasmoidTask::isValid() const
{
    return !m_appletName.isEmpty() && m_applet;
}


QString PlasmoidTask::taskId() const
{
    return m_taskId;
}


QIcon PlasmoidTask::icon() const
{
    return m_icon;
}

Plasma::Applet *PlasmoidTask::host() const
{
    return m_host;
}

bool PlasmoidTask::isWidget() const {
    return true;
}

QQuickItem* PlasmoidTask::createWidget(Plasma::Applet *host)
{
    if (host != m_host || !m_applet) {
        return 0;
    }

    Plasma::Applet *applet = m_applet.data();
    m_takenByParent = true;
    applet->setParent(host);
    //applet->setParentItem(host);
    KConfigGroup group = applet->config();
    group = group.parent();
    applet->restore(group);
    applet->init();
    applet->updateConstraints(Plasma::Types::StartupCompletedConstraint);
    applet->flushPendingConstraintsEvents();
    applet->updateConstraints(Plasma::Types::AllConstraints);
    applet->flushPendingConstraintsEvents();

    // make sure to record it in the configuration so that if we reload from the config,
    // this applet is remembered
    KConfigGroup dummy;
    applet->save(dummy);

    connect(applet, SIGNAL(newStatus(Plasma::Types::ItemStatus)), this, SLOT(newAppletStatus(Plasma::Types::ItemStatus)));

    newAppletStatus(applet->status());

    connect(applet, SIGNAL(configNeedsSaving()), host, SIGNAL(configNeedsSaving()));
    connect(applet, SIGNAL(releaseVisualFocus()), host, SIGNAL(releaseVisualFocus()));

    //return static_cast<QQuickItem*>(applet);
    return new QQuickItem();
}

void PlasmoidTask::forwardConstraintsEvent(Plasma::Types::Constraints constraints)
{
    Plasma::Applet *applet = m_applet.data();
    if (applet) {
        applet->updateConstraints(constraints);
        applet->flushPendingConstraintsEvents();
    }
}

int PlasmoidTask::id() const
{
    if (m_applet) {
        return m_applet.data()->id();
    }

    return 0;
}

void PlasmoidTask::setupApplet(const QString &plugin, int id)
{
    Plasma::Applet *applet = Plasma::PluginLoader::self()->loadApplet(plugin, id);
    m_applet = applet;

    if (!m_applet) {
        qDebug() << "Could not load applet" << plugin;
        return;
    }

    //FIXME: System Information should be system services, but battery and devicenotifier are both there. we would need multiple categories
    if (applet->pluginInfo().category() == "System Information" ||
        applet->pluginInfo().category() == "Network") {
        setCategory(Hardware);
    } else if (applet->pluginInfo().category() == "Online Services") {
        setCategory(Communications);
    }

    setName(applet->pluginInfo().name());

    m_icon = QIcon::fromTheme(applet->pluginInfo().icon());

    //applet->setFlag(QGraphicsItem::ItemIsMovable, false);

    connect(applet, SIGNAL(appletDestroyed(Plasma::Applet*)), this, SLOT(appletDestroyed(Plasma::Applet*)));
    //applet->setBackgroundHints(Plasma::Applet::NoBackground);
}

void PlasmoidTask::appletDestroyed(Plasma::Applet *)
{
    emit destroyed(this);
    forget(m_host);
    deleteLater();
}

void PlasmoidTask::newAppletStatus(Plasma::Types::ItemStatus status)
{
    Plasma::Applet *applet = m_applet.data();
    if (!applet) {
        return;
    }

    switch (status) {
    case Plasma::Types::PassiveStatus:
       if (Plasma::Applet *popupApplet = qobject_cast<Plasma::Applet *>(applet)) {
           //popupApplet->setExpanded(false);
       }
       setStatus(Task::Passive);
       break;

    case Plasma::Types::ActiveStatus:
       setStatus(Task::Active);
       break;

    case Plasma::Types::NeedsAttentionStatus:
        setStatus(Task::NeedsAttention);
        break;

    default:
    case Plasma::Types::UnknownStatus:
        setStatus(Task::UnknownStatus);
    }
}

}

#include "plasmoidtask.moc"
