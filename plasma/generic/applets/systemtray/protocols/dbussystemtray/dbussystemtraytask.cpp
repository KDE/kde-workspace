/***************************************************************************
 *                                                                         *
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

#include "dbussystemtraytask.h"

#include "dbussystemtrayprotocol.h"

#include <QtCore/QMetaEnum>
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtGui/QMenu>
#include <QtGui/QIcon>

#include <KDE/KJob>
#include <KDE/KIconLoader>
#include <KDE/Plasma/ServiceJob>
#include <KDE/Plasma/ToolTipManager>
#include <KDE/Plasma/Applet>

namespace SystemTray
{

DBusSystemTrayTask::DBusSystemTrayTask(const QString &serviceName, Plasma::DataEngine *dataEngine, QObject *parent)
    : Task(parent),
      m_serviceName(serviceName),
      m_typeId(serviceName),
      m_name(serviceName),
      m_dataEngine(dataEngine),
      m_service(dataEngine->serviceForSource(serviceName)),
      m_isMenu(false),
      m_valid(false)
{
    kDebug();
    m_service->setParent(this);

    m_dataEngine->connectSource(serviceName, this);
}

DBusSystemTrayTask::~DBusSystemTrayTask()
{
}

QGraphicsWidget* DBusSystemTrayTask::createWidget(Plasma::Applet */*host*/)
{
    return 0;  // d-bus tasks don't have widgets but provide info for GUI;
}

bool DBusSystemTrayTask::isValid() const
{
    return m_valid;
}

bool DBusSystemTrayTask::isEmbeddable() const
{
    return false; // this task cannot be embed because it only provides information to GUI part
}

bool DBusSystemTrayTask::isWidget() const
{
    return false; // isn't a widget
}

void DBusSystemTrayTask::setShortcut(QString text) {
    if (m_shortcut != text) {
        m_shortcut = text;
        emit changedShortcut();
    }
}

QString DBusSystemTrayTask::name() const
{
    return m_name;
}

QString DBusSystemTrayTask::typeId() const
{
    return m_typeId;
}

QIcon DBusSystemTrayTask::icon() const
{
    return m_icon;
}

void DBusSystemTrayTask::activate1(int x, int y) const
{
    KConfigGroup params;
    if (m_isMenu) {
        params = m_service->operationDescription("ContextMenu");
    } else {
        params = m_service->operationDescription("Activate");
    }
    params.writeEntry("x", x);
    params.writeEntry("y", y);
    KJob *job = m_service->startOperationCall(params);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(_onContextMenu(KJob*)));
}

void DBusSystemTrayTask::activate2(int x, int y) const
{
    KConfigGroup params = m_service->operationDescription("SecondaryActivate");
    params.writeEntry("x", x);
    params.writeEntry("y", y);
    m_service->startOperationCall(params);
}

void DBusSystemTrayTask::activateHorzScroll(int delta) const
{
    _activateScroll(delta, "Horizontal");
}

void DBusSystemTrayTask::activateVertScroll(int delta) const
{
    _activateScroll(delta, "Vertical");
}

void DBusSystemTrayTask::_activateScroll(int delta, QString direction) const
{
    KConfigGroup params = m_service->operationDescription("Scroll");
    params.writeEntry("delta", delta);
    params.writeEntry("direction", direction);
    m_service->startOperationCall(params);
}

void DBusSystemTrayTask::activateContextMenu(int x, int y) const
{
    KConfigGroup params = m_service->operationDescription("ContextMenu");
    params.writeEntry("x", x);
    params.writeEntry("y", y);
    KJob *job = m_service->startOperationCall(params);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(_onContextMenu(KJob*)));
}

void DBusSystemTrayTask::_onContextMenu(KJob *job)
{
    if (QCoreApplication::closingDown()) {
        // apparently an edge case can be triggered due to the async nature of all this
        // see: https://bugs.kde.org/show_bug.cgi?id=251977
        return;
    }

    Plasma::ServiceJob *sjob = qobject_cast<Plasma::ServiceJob *>(job);
    if (!sjob) {
        return;
    }

    QMenu *menu = qobject_cast<QMenu *>(sjob->result().value<QObject *>());
    if (menu) {
        int x = sjob->parameters()["x"].toInt();
        int y = sjob->parameters()["y"].toInt();
        emit showContextMenu(x, y, QVariant::fromValue<QObject*>(menu));
    }
}

void DBusSystemTrayTask::dataUpdated(const QString &taskName, const Plasma::DataEngine::Data &properties)
{
    Q_UNUSED(taskName);

    QString id = properties["Id"].toString();
    bool become_valid = false;
    if (!id.isEmpty() && id != m_typeId) {
        m_typeId = id;
        m_valid = true;
        become_valid = true;
        setObjectName(QString("SystemTray-%1").arg(m_typeId));
    }

    QString cat = properties["Category"].toString();
    if (!cat.isEmpty()) {
        int index = metaObject()->indexOfEnumerator("Category");
        int key = metaObject()->enumerator(index).keyToValue(cat.toLatin1());

        if (key != -1) {
            setCategory((Task::Category)key);
        }
    }

    if (properties["TitleChanged"].toBool() || become_valid) {
        QString title = properties["Title"].toString();
        if (!title.isEmpty()) {
            bool is_title_changed = (m_name != title);
            m_name = title;
            if (is_title_changed)
                emit changedTitle();
        }
    }

    if (properties["IconsChanged"].toBool() || become_valid) {
        syncIcons(properties);
        emit changedIcons();
    }

    if (properties["StatusChanged"].toBool() || become_valid) {
        syncStatus(properties["Status"].toString());
    }

    if (properties["ToolTipChanged"].toBool() || become_valid) {
        syncToolTip(properties["ToolTipTitle"].toString(),
                    properties["ToolTipSubTitle"].toString(),
                    properties["ToolTipIcon"].value<QIcon>());
    }

    bool is_menu = properties["ItemIsMenu"].toBool();
    if (is_menu != m_isMenu) {
        m_isMenu = is_menu;
        emit changedIsMenu();
    }

    if (become_valid) {
        DBusSystemTrayProtocol *protocol = qobject_cast<DBusSystemTrayProtocol*>(parent());
        if (protocol) {
            protocol->initedTask(this);
        }
    }
}

void DBusSystemTrayTask::syncIcons(const Plasma::DataEngine::Data &properties)
{
    m_icon = properties["Icon"].value<QIcon>();
    m_attentionIcon = properties["AttentionIcon"].value<QIcon>();

    QString icon_name            = properties["IconName"].toString();
    QString att_icon_name        = properties["AttentionIconName"].toString();
    QString movie_path           = properties["AttentionMovieName"].toString();
    QString overlay_icon_name    = properties["OverlayIconName"].value<QString>();
    bool is_icon_name_changed           = false;
    bool is_att_icon_name_changed       = false;
    bool is_movie_path_changed          = false;
    bool is_overlay_icon_name_changed   = false;

    if (icon_name != m_iconName) {
        m_iconName = icon_name;
        is_icon_name_changed = true;
    }

    if (att_icon_name != m_attentionIconName) {
        m_attentionIconName = att_icon_name;
        is_att_icon_name_changed = true;
    }

    if (!movie_path.isEmpty() && !QDir::isAbsolutePath(movie_path)) {
        movie_path = KIconLoader::global()->moviePath(movie_path, KIconLoader::Panel);
    }

    if (movie_path != m_moviePath) {
        m_moviePath = movie_path;
        is_movie_path_changed = true;
    }

    if (overlay_icon_name != m_overlayIconName) {
        m_overlayIconName = overlay_icon_name;
        is_overlay_icon_name_changed = true;
    }

    // emit signals
    if (is_icon_name_changed) {
        emit changedIconName();
    }
    if (is_att_icon_name_changed) {
        emit changedAttIconName();
    }
    if (is_movie_path_changed) {
        emit changedMoviePath();
    }
    if (is_overlay_icon_name_changed) {
        emit changedOverlayIconName();
    }
}


//toolTip

void DBusSystemTrayTask::syncToolTip(const QString &title, const QString &subTitle, const QIcon &toolTipIcon)
{
    if (title != m_tooltipTitle) {
        m_tooltipTitle = title;
        emit changedTooltipTitle();
    }

    if (subTitle != m_tooltipText) {
        m_tooltipText = subTitle;
        emit changedTooltipText();
    }

    bool is_icon_name_changed = (m_tooltipIcon.name() != toolTipIcon.name());

    m_tooltipIcon = toolTipIcon;
    emit changedTooltip();

    if (is_icon_name_changed) {
        emit changedTooltipIconName();
    }
}


//Status

void DBusSystemTrayTask::syncStatus(QString newStatus)
{
    Task::Status status = (Task::Status)metaObject()->enumerator(metaObject()->indexOfEnumerator("Status")).keyToValue(newStatus.toLatin1());

    if (this->status() == status) {
        return;
    }

    setStatus(status);
}

}

#include "dbussystemtraytask.moc"
