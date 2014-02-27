/***************************************************************************
 *   Copyright 2013 Marco Martin <mart@kde.org>                            *
 *   Copyright 2014 Sebastian Kugler <sebas@kde.org>                       *
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

#include "kuserproxy.h"
#include <QUrl>

KUserProxy::KUserProxy (QObject *parent)
    : QObject(parent)
{
}

KUserProxy::~KUserProxy()
{
}

QString KUserProxy::fullName() const
{
    return m_user.property(KUser::FullName).toString();
}

QString KUserProxy::loginName() const
{
    return m_user.loginName();
}

QString KUserProxy::faceIconPath() const
{
    return QUrl::fromLocalFile(m_user.faceIconPath()).toString();
}

QString KUserProxy::os() const
{
    return "Debian Wheezy"; // FIXME
}

QString KUserProxy::host() const
{
    return "monet"; // FIXME
}

