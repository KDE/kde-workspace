/*
 *   Copyright 2013 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "interfaceApi.h"

#include <Plasma/Package>

InterfaceApi::InterfaceApi(QObject *parent)
    : QObject(parent)
{
}

void InterfaceApi::setPackage(Plasma::Package *package)
{
    m_package = package;
}

Plasma::Package *InterfaceApi::package() const
{
    return m_package;
}

void InterfaceApi::toggleConfig()
{
    //FIXME: show config
}

QStringList InterfaceApi::runners() const
{
    return m_runners;
}

void InterfaceApi::setRunners(const QStringList &runners)
{
    if (runners != m_runners) {
        m_runners = runners;
        emit runnersChanged();
    }
}

QString InterfaceApi::singleRunnerId() const
{
    return m_singleRunnerId;
}

void InterfaceApi::setSingleRunnerId(const QString &id)
{
    if (m_singleRunnerId != id) {
        m_singleRunnerId = id;
        emit singleRunnerIdChanged();
    }
}

QString InterfaceApi::filePath(const QString &fileType, const QString &filename) const
{
    if (!m_package) {
        return QString();
    }

    return m_package->filePath(fileType.toLatin1(), filename);
}

QString InterfaceApi::filePath(const QString &fileType) const
{
    if (!m_package) {
        return QString();
    }

    return m_package->filePath(fileType.toLatin1());
}

void InterfaceApi::signalClearHistory()
{
    emit clearHistory();
}

void InterfaceApi::query(const QString &query)
{
    emit startQuery(query);
}

#include <interfaceApi.moc>

