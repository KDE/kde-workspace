/*
 *   Copyright 2008 David Edmundson <kde@davidedmundson.co.uk>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef PLACESRUNNER_H
#define PLACESRUNNER_H


#include <plasma/abstractrunner.h>
#include <kfileplacesmodel.h>

class PlacesRunner;

class PlacesRunnerHelper : public QObject
{
    Q_OBJECT

public:
    PlacesRunnerHelper(PlacesRunner *runner);

public Q_SLOTS:
    void match(Plasma::RunnerContext *context);

private:
    KFilePlacesModel m_places;
};

class PlacesRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    PlacesRunner(QObject* parent, const QVariantList &args);
    ~PlacesRunner();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action);

Q_SIGNALS:
    void doMatch(Plasma::RunnerContext *context);

private Q_SLOTS:
    void setupComplete(QModelIndex, bool);

private:
    PlacesRunnerHelper *m_helper;
};

K_EXPORT_PLASMA_RUNNER(placesrunner, PlacesRunner)

#endif
