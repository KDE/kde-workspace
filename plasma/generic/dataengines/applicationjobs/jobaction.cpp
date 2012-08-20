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

#include "jobaction.h"
#include "kuiserverengine.h"

#include <kdebug.h>

void JobAction::start()
{
    kDebug() << "Trying to perform the action" << operationName();

    if (!m_jobView) {
        setErrorText(i18nc("%1 is the subject (can be anything) upon which the job is performed",
                           "The JobView for %1 cannot be found", destination()));
        setError(-1);
        emitResult();
        return;
    }

    //TODO: check with capabilities before performing actions.
    if (operationName() == "resume") {
        m_jobView->requestStateChange(JobView::Running);
    } else if (operationName() == "suspend") {
        m_jobView->requestStateChange(JobView::Suspended);
    } else if (operationName() == "stop") {
        m_jobView->requestStateChange(JobView::Stopped);
        //in case the app crashed and won't call terminate on the jobview.
        m_jobView->terminate(i18n("Job canceled by user."));
    } else if (operationName() == "startInteraction") {
        m_jobView->requestInteraction();
    }

    emitResult();
}

#include "jobaction.moc"

