/*
 *   Copyright © 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#include "jobcontrol.h"
#include "jobaction.h"
#include "kuiserverengine.h"

JobControl::JobControl(QObject* parent, JobView *jobView)
    : Plasma::Service(parent),
      m_jobView(jobView)
{
    setName("applicationjobs");
    setDestination(jobView->objectName());
}

Plasma::ServiceJob* JobControl::createJob(const QString& operation,
                                          QMap<QString,QVariant>& parameters)
{
    return new JobAction(m_jobView, operation, parameters, this);
}

#include "jobcontrol.moc"

