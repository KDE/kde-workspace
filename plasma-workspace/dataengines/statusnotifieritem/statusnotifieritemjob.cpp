/*
 * Copyright 2008 Alain Boyer <alainboyer@gmail.com>
 * Copyright (C) 2009 Matthieu Gallien <matthieu_gallien@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License version 2 as
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "statusnotifieritemjob.h"
#include <iostream>

StatusNotifierItemJob::StatusNotifierItemJob(StatusNotifierItemSource *source, const QString &operation, QMap<QString, QVariant> &parameters, QObject *parent) :
    ServiceJob(source->objectName(), operation, parameters, parent),
    m_source(source)
{
    connect(source, SIGNAL(contextMenuReady(QMenu*)), this, SLOT(contextMenuReady(QMenu*)));
}

StatusNotifierItemJob::~StatusNotifierItemJob()
{
}

void StatusNotifierItemJob::start()
{
    if (operationName() == QString::fromLatin1("Activate")) {
        m_source->activate(parameters()["x"].toInt(), parameters()["y"].toInt());
        setResult(0);
    } else if (operationName() == QString::fromLatin1("SecondaryActivate")) {
        m_source->secondaryActivate(parameters()["x"].toInt(), parameters()["y"].toInt());
        setResult(0);
    } else if (operationName() == QString::fromLatin1("ContextMenu")) {
        m_source->contextMenu(parameters()["x"].toInt(), parameters()["y"].toInt());
    } else if (operationName() == QString::fromLatin1("Scroll")) {
        m_source->scroll(parameters()["delta"].toInt(), parameters()["direction"].toString());
        setResult(0);
    }
}

void StatusNotifierItemJob::contextMenuReady(QMenu *menu)
{
    if (operationName() == QString::fromLatin1("ContextMenu")) {
        setResult(qVariantFromValue((QObject*)menu));
    }
}

#include "statusnotifieritemjob.moc"
