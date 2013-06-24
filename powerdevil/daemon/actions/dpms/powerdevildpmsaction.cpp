/***************************************************************************
 *   Copyright (C) 2010 by Dario Freddi <drf@kde.org>                      *
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


#include "powerdevildpmsaction.h"

#include <powerdevilcore.h>

#include <config-workspace.h>

#include <QtGui/QX11Info>

#include <KConfigGroup>
#include <KDebug>
#include <KPluginFactory>

#include <X11/Xmd.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
extern "C" {
#include <X11/extensions/dpms.h>
    int __kde_do_not_unload = 1;

#ifndef HAVE_DPMSCAPABLE_PROTO
    Bool DPMSCapable(Display *);
#endif

#ifndef HAVE_DPMSINFO_PROTO
    Status DPMSInfo(Display *, CARD16 *, BOOL *);
#endif

int dropError(Display *, XErrorEvent *);
typedef int (*XErrFunc)(Display *, XErrorEvent *);
}

int dropError(Display *, XErrorEvent *)
{
    return 0;
}

class PowerDevilDPMSAction::Private
{
public:
    XErrorHandler defaultHandler;
};

K_PLUGIN_FACTORY(PowerDevilDPMSActionFactory, registerPlugin<PowerDevilDPMSAction>(); )
K_EXPORT_PLUGIN(PowerDevilDPMSActionFactory("powerdevildpmsaction"))

PowerDevilDPMSAction::PowerDevilDPMSAction(QObject* parent, const QVariantList &args)
    : Action(parent)
    , m_idleTime(0)
    , m_inhibitScreen(0)  // can't use PowerDevil::PolicyAgent enum because X11/X.h defines None as 0L
    , d(new Private)
{
    setRequiredPolicies(PowerDevil::PolicyAgent::ChangeScreenSettings);

    // We want to query for DPMS in the constructor, before anything else happens
    d->defaultHandler = XSetErrorHandler(dropError);

    // Since we are in the constructor, we should check for support manually
    if (!isSupported()) {
        XSetErrorHandler(d->defaultHandler);
        return;
    }

    // Is the action being loaded outside the core?
    if (args.size() > 0) {
        if (args.first().toBool()) {
            kDebug() << "Action loaded from outside the core, skipping early init";
            return;
        }
    }

    // Pretend we're unloading profiles here, as if the action is not enabled, DPMS should be switched off.
    onProfileUnload();

    // Listen to the policy agent
    connect(PowerDevil::PolicyAgent::instance(),
            SIGNAL(unavailablePoliciesChanged(PowerDevil::PolicyAgent::RequiredPolicies)),
            this,
            SLOT(onUnavailablePoliciesChanged(PowerDevil::PolicyAgent::RequiredPolicies)));
    // inhibitions persist over kded module unload/load
    m_inhibitScreen = PowerDevil::PolicyAgent::instance()->unavailablePolicies() & PowerDevil::PolicyAgent::ChangeScreenSettings;
}

PowerDevilDPMSAction::~PowerDevilDPMSAction()
{
    delete d;
}

bool PowerDevilDPMSAction::isSupported()
{
    Display *dpy = QX11Info::display();
    int dummy;

    return DPMSQueryExtension(dpy, &dummy, &dummy) && DPMSCapable(dpy);
}

void PowerDevilDPMSAction::onProfileUnload()
{
    using namespace PowerDevil;
    Display *dpy = QX11Info::display();
    if (!(PolicyAgent::instance()->unavailablePolicies() & PolicyAgent::ChangeScreenSettings)) {
        DPMSDisable(dpy);
    } else {
        kDebug() << "Not performing DPMS action due to inhibition";
    }
    DPMSSetTimeouts(dpy, 0, 0, 0);
}

void PowerDevilDPMSAction::onWakeupFromIdle()
{
    //
}

void PowerDevilDPMSAction::onIdleTimeout(int msec)
{
    Q_UNUSED(msec);
}

void PowerDevilDPMSAction::onProfileLoad()
{
    using namespace PowerDevil;
    Display *dpy = QX11Info::display();
    if (!(PolicyAgent::instance()->unavailablePolicies() & PolicyAgent::ChangeScreenSettings)) {
        DPMSEnable(dpy);
    } else {
        kDebug() << "Not performing DPMS action due to inhibition";
        return;
    }

    XFlush(dpy);
    XSetErrorHandler(d->defaultHandler);

    // An unloaded action will have m_idleTime = 0:
    // DPMS enabled with zeroed timeouts is effectively disabled.
    // So onProfileLoad is always safe
    DPMSSetTimeouts(dpy, (CARD16)m_idleTime, (CARD16)(m_idleTime * 1.5), (CARD16)(m_idleTime * 2));

    XFlush(dpy);
    XSetErrorHandler(d->defaultHandler);
}

void PowerDevilDPMSAction::triggerImpl(const QVariantMap& args)
{
    CARD16 dummy;
    BOOL enabled;
    Display *dpy = QX11Info::display();
    DPMSInfo(dpy, &dummy, &enabled);

    if (args["Type"].toString() == "TurnOff") {
        if (enabled) {
            DPMSForceLevel(dpy, DPMSModeOff);
        } else {
            DPMSEnable(dpy);
            DPMSForceLevel(dpy, DPMSModeOff);
        }
    } else if (args["Type"].toString() == "Standby") {
        if (enabled) {
            DPMSForceLevel(dpy, DPMSModeStandby);
        } else {
            DPMSEnable(dpy);
            DPMSForceLevel(dpy, DPMSModeStandby);
        }
    } else if (args["Type"].toString() == "Suspend") {
        if (enabled) {
            DPMSForceLevel(dpy, DPMSModeSuspend);
        } else {
            DPMSEnable(dpy);
            DPMSForceLevel(dpy, DPMSModeSuspend);
        }
    }

    // this leaves DPMS enabled but if it's meant to be disabled
    // then the timeouts will be zero and so effectively disabled
}

bool PowerDevilDPMSAction::loadAction(const KConfigGroup& config)
{
    m_idleTime = config.readEntry<int>("idleTime", -1);

    return true;
}

bool PowerDevilDPMSAction::onUnloadAction()
{
    m_idleTime = 0;
    return Action::onUnloadAction();
}

void PowerDevilDPMSAction::onUnavailablePoliciesChanged(PowerDevil::PolicyAgent::RequiredPolicies policies)
{
    // only take action if screen inhibit changed
    PowerDevil::PolicyAgent::RequiredPolicies oldPolicy = m_inhibitScreen;
    m_inhibitScreen = policies & PowerDevil::PolicyAgent::ChangeScreenSettings;
    if (oldPolicy == m_inhibitScreen) {
        return;
    }

    if (m_inhibitScreen) {
        // Inhibition triggered: disable DPMS
        kDebug() << "Disabling DPMS due to inhibition";
        Display *dpy = QX11Info::display();
        DPMSSetTimeouts(dpy, 0, 0, 0);
        DPMSDisable(dpy); // wakes the screen - do we want this?
    } else {
        // Inhibition removed: let's start again
        onProfileLoad();
        kDebug() << "Restoring DPMS features after inhibition release";
    }
}

#include "powerdevildpmsaction.moc"
