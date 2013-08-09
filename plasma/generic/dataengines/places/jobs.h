/*
 * Copyright 2008  Alex Merry <alex.merry@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */
#ifndef JOBS_H
#define JOBS_H

#include "modeljob.h"
#include <KUrl>

class AddEditPlaceJob : public ModelJob
{
public:
    AddEditPlaceJob(KFilePlacesModel* model,
                    QModelIndex index,
                    const QVariantMap& parameters,
                    QObject* parent = 0)
        : ModelJob(parent, model, index, (index.isValid() ? "Edit" : "Add"), parameters)
        , m_text(parameters["Name"].toString())
        , m_url(parameters["Url"].toUrl())
        , m_icon(parameters["Icon"].toString())
    {}

    void start()
    {
        if (m_index.isValid()) {
            m_model->editPlace(m_index, m_text, m_url, m_icon);
        } else {
            m_model->addPlace(m_text, m_url, m_icon);
        }
    }

private:
    QString m_text;
    KUrl m_url;
    QString m_icon;
};

class RemovePlaceJob : public ModelJob
{
public:
    RemovePlaceJob(KFilePlacesModel* model, const QModelIndex& index,
                   QObject* parent)
        : ModelJob(parent, model, index, "Remove")
    {}

    void start()
    {
        m_model->removePlace(m_index);
    }
};

class ShowPlaceJob : public ModelJob
{
public:
    ShowPlaceJob(KFilePlacesModel* model, const QModelIndex& index,
                 bool show = true, QObject* parent = 0)
        : ModelJob(parent, model, index, (show ? "Show" : "Hide"))
        , m_show(show)
    {}

    void start()
    {
        m_model->setPlaceHidden(m_index, m_show);
    }

private:
    bool m_show;
};

class TeardownDeviceJob : public ModelJob
{
public:
    TeardownDeviceJob(KFilePlacesModel* model, const QModelIndex& index,
                      QObject* parent = 0)
        : ModelJob(parent, model, index, "Teardown Device")
    {}

    void start()
    {
        m_model->requestTeardown(m_index);
    }
};

#include "setupdevicejob.h"

#endif // JOBS_H

