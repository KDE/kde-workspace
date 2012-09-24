/********************************************************************

Copyright (C) 2008 Lubos Lunak <l.lunak@suse.cz>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "randrmonitor.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ktoolinvocation.h>
#include <solid/powermanagement.h>

#include <qdbusservicewatcher.h>
#include <qdbusconnection.h>
#include <qdbusconnectioninterface.h>
#include <qtimer.h>
#include <qx11info_x11.h>
#include <qlayout.h>

K_PLUGIN_FACTORY(RandrMonitorModuleFactory,
                 registerPlugin<RandrMonitorModule>();
                )
K_EXPORT_PLUGIN(RandrMonitorModuleFactory("randrmonitor"))

RandrMonitorModule::RandrMonitorModule( QObject* parent, const QList<QVariant>& )
    : KDEDModule( parent )
    , have_randr( false )
    , dialog(0)
{
    m_inhibitionCookie = -1;
    setModuleName( "randrmonitor" );
    initRandr();

    QDBusReply <bool> re =  QDBusConnection::systemBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement");
    if (!re.value()) {
        kDebug() << "PowerManagement not loaded, waiting for it";
        QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher("org.kde.Solid.PowerManagement", QDBusConnection::sessionBus(),
                QDBusServiceWatcher::WatchForRegistration, this);
        connect(serviceWatcher, SIGNAL(serviceRegistered(QString)), this, SLOT(checkInhibition()));
        connect(serviceWatcher, SIGNAL(serviceRegistered(QString)), this, SLOT(checkResumeFromSuspend()));
        return;
    }

    checkInhibition();
    checkResumeFromSuspend();
}

RandrMonitorModule::~RandrMonitorModule()
{
    if( have_randr )
    {
        Display* dpy = QX11Info::display();
        XDestroyWindow( dpy, window );
        delete helper;
        have_randr = false;
    }
}

void RandrMonitorModule::initRandr()
{
    Display* dpy = QX11Info::display();
    if( !XRRQueryExtension( dpy, &randr_base, &randr_error ))
        return;
    int major = 1;
    int minor = 2;
    if( !XRRQueryVersion( dpy, &major, &minor ) || major < 1 || (major == 1 && minor < 2 ))
        return;
    have_randr = true;
    // It looks like we need a separate window for getting the events, so that we don't
    // change e.g. Qt's event mask.
    window = XCreateSimpleWindow( dpy, DefaultRootWindow( dpy ), 0, 0, 1, 1, 0, 0, 0 );
    XRRSelectInput( dpy, window, RROutputChangeNotifyMask );
#if 0 // xrandr apparently can't detect hw changes and on some systems polling freezes X :(
    // HACK: see poll()
    QTimer* timer = new QTimer( this );
    timer->start( 10000 ); // 10 s
    connect( timer, SIGNAL(timeout()), this, SLOT(poll()));
#endif
    helper = new RandrMonitorHelper( this );
    kapp->installX11EventFilter( helper );
    currentMonitors = connectedMonitors();
    KActionCollection* coll = new KActionCollection( this );
    KAction* act = coll->addAction( "display" );
    act->setText( i18n( "Switch Display" ));
    act->setGlobalShortcut( KShortcut( Qt::Key_Display ));
    connect( act, SIGNAL(triggered(bool)), SLOT(switchDisplay()));
}

void RandrMonitorModule::poll()
{
    // HACK: It seems that RRNotify/RRNotify_OutputChange event (i.e. detecting a newly
    // plugged or unplugged monitor) does not work without polling some randr functionality.
    int dummy;
    XRRGetScreenSizeRange( QX11Info::display(), window, &dummy, &dummy, &dummy, &dummy );
}

void RandrMonitorModule::processX11Event( XEvent* e )
{
    if( e->xany.type == randr_base + RRNotify )
    {
        XRRNotifyEvent* e2 = reinterpret_cast< XRRNotifyEvent* >( e );
        if( e2->subtype == RRNotify_OutputChange ) // TODO && e2->window == window )
        {
            kDebug() << "Monitor change detected";
            QStringList newMonitors = connectedMonitors();

            //If we are already inhibiting and we should stop it, do it
            checkInhibition();

            if( newMonitors == currentMonitors ) {
                kDebug() << "Same monitors";
                return;
            }
            if( QDBusConnection::sessionBus().interface()->isServiceRegistered(
                        "org.kde.internal.KSettingsWidget-kcm_randr" ))
            {   // already running
                return;
            }
            kapp->updateUserTimestamp(); // well, let's say plugging in a monitor is a user activity
            currentMonitors = newMonitors;

            // TODO make sure to be on right screen after a screen has been disconnected
            if (!dialog) {
                dialog = new KDialog();
                dialog->setCaption(i18n("Monitor setup has changed"));
                QLabel *icon = new QLabel();
                icon->setPixmap(KIcon("preferences-desktop-display").pixmap(QSize(64, 64), QIcon::Normal, QIcon::On));
                QString question =
                    (newMonitors.count() < currentMonitors.count())
                    ? i18n("A monitor output has been disconnected.")
                    : i18n("A new monitor output has been connected.")
                    + "\n\n" + i18n("Do you wish to run a configuration tool to adjust the monitor setup?");
                QLabel *label = new QLabel(question);
                QHBoxLayout *layout = new QHBoxLayout();
                layout->setSpacing(4);
                layout->addWidget(icon);
                layout->addWidget(label);
                QWidget *mainWidget = new QWidget(dialog);
                mainWidget->setLayout(layout);
                dialog->setMainWidget(mainWidget);
                dialog->setButtons(KDialog::Yes | KDialog::No | KDialog::Try);
                dialog->setDefaultButton(KDialog::Yes);
                dialog->setButtonText(KDialog::Try, i18nc("@Button: try to adjust screen configuration automatically", "Try Automatically"));
                connect(dialog, SIGNAL(yesClicked()), this, SLOT(showKcm()));
                connect(dialog, SIGNAL(tryClicked()), this, SLOT(tryAutoConfig()));
            }
            if (!dialog->isVisible()) {
                dialog->show();
            }
            dialog->raise();
            dialog->activateWindow();
        }
    }
}

void RandrMonitorModule::showKcm()
{
    KToolInvocation::kdeinitExec("kcmshell4", QStringList() << "display");
}

void RandrMonitorModule::tryAutoConfig()
{
    KProcess::execute(QStringList() << "xrandr" << "--auto");
}

QStringList RandrMonitorModule::connectedMonitors() const
{
    QStringList ret;
    Display* dpy = QX11Info::display();
    XRRScreenResources* resources = XRRGetScreenResources( dpy, window );
    for( int i = 0; i < resources->noutput; ++i )
    {
        XRROutputInfo* info = XRRGetOutputInfo( dpy, resources, resources->outputs[ i ] );
        QString name = QString::fromUtf8( info->name );
        if( info->connection == RR_Connected )
            ret.append( name );
        XRRFreeOutputInfo( info );
    }
    XRRFreeScreenResources( resources );
    return ret;
}

QStringList RandrMonitorModule::activeMonitors() const
{
    QStringList ret;
    Display* dpy = QX11Info::display();
    XRRScreenResources* resources = XRRGetScreenResources( dpy, window );
    for( int i = 0; i < resources->noutput; ++i )
    {
        XRROutputInfo* info = XRRGetOutputInfo( dpy, resources, resources->outputs[ i ] );
        QString name = QString::fromUtf8( info->name );
        if(info->crtc != None)
            ret.append( name );
        XRRFreeOutputInfo( info );
    }
    XRRFreeScreenResources( resources );
    return ret;
}

void RandrMonitorModule::checkInhibition()
{
    if (!have_randr) {
        kDebug() << "Can't check inhibition, XRandR minor to 1.2 detected";
        return;
    }

    if (!isLidPresent()) {
        kDebug() << "This feature is only for laptop, and there is no Lid present";
        return;
    }

    QStringList activeMonitorsList = activeMonitors();
    kDebug() << "Active monitor list";
    kDebug() << activeMonitorsList;

    bool inhibit = false;
    Q_FOREACH(const QString monitor, activeMonitorsList) {
        //LVDS is the default type reported by most drivers, default is needed because the
        //NVIDIA binary blob always report default as active monitor.
        if (!monitor.contains("LVDS") && !monitor.contains("default") && !monitor.contains("eDP")) {
            inhibit = true;
            break;
        }
    }

    if (m_inhibitionCookie > 0 && !inhibit) {
        kDebug() << "Stopping: " << m_inhibitionCookie;
        Solid::PowerManagement::stopSuppressingSleep(m_inhibitionCookie);
        m_inhibitionCookie = -1;
    } else if (m_inhibitionCookie < 0 && inhibit) { // If we are NOT inhibiting and we should, do it
        m_inhibitionCookie = Solid::PowerManagement::beginSuppressingSleep();
        kDebug() << "Inhibing: " << m_inhibitionCookie;
    }
}

bool RandrMonitorModule::isLidPresent()
{
    QDBusMessage call = QDBusMessage::createMethodCall("org.freedesktop.UPower",
                        "/org/freedesktop/UPower",
                        "org.freedesktop.DBus.Properties",
                        "Get");
    QList <QVariant> args;
    args.append(QVariant::fromValue<QString>(QString("org.freedesktop.UPower")));
    args.append(QVariant::fromValue<QString>(QString("LidIsPresent")));
    call.setArguments(args);

    QDBusMessage msg =  QDBusConnection::systemBus().call(call);
    QDBusReply<QDBusVariant> reply(msg);

    if (!reply.isValid()) {
        kDebug() << reply.error();
        return false;
    }

    return reply.value().variant().toBool();
}


void RandrMonitorModule::checkResumeFromSuspend()
{
    QDBusConnection::sessionBus().connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement", "org.kde.Solid.PowerManagement", "resumingFromSuspend", this, SLOT(resumedFromSuspend()));
}

void RandrMonitorModule::switchDisplay()
{
    QDBusMessage call = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                        "/org/kde/Solid/PowerManagement",
                        "org.kde.Solid.PowerManagement",
                        "isLidClosed");
    QDBusMessage msg =  QDBusConnection::sessionBus().call(call);
    QDBusReply<bool> reply(msg);

    if (reply.isValid() && reply.value()) {
        kDebug() << "Lid is closed, ignoring the event";
        //TODO: When we rewrite this, be sure that in this case LVDS is disabled instead of ignoring
        return;
    }

    QList< RandROutput* > outputs;
    RandRDisplay display;
    outputs = connectedOutputs( display );
    if( outputs.count() == 0 ) // nothing connected, do nothing
        return;
    if( outputs.count() == 1 ) // just one, enable it
    {
        enableOutput( outputs[0], true );
        for( int scr = 0; scr < display.numScreens(); ++scr )
        {
            foreach( RandROutput* output, display.screen( scr )->outputs())
            {
                if( !output->isConnected())
                    enableOutput( output, false ); // switch off every output that's not connected
            }
        }
        return;
    }
    if( outputs.count() == 2 ) // alternative between one, second, both
    {
        if( outputs[ 0 ]->isActive() && !outputs[ 1 ]->isActive())
        {
            enableOutput( outputs[ 1 ], true );
            enableOutput( outputs[ 0 ], false );
        }
        else if( !outputs[ 0 ]->isActive() && outputs[ 1 ]->isActive())
        {
            enableOutput( outputs[ 1 ], true );
            enableOutput( outputs[ 0 ], true );
        }
        else
        {
            enableOutput( outputs[ 0 ], true );
            enableOutput( outputs[ 1 ], false );
        }
        return;
    }
    // no idea what to do here
    KToolInvocation::kdeinitExec( "kcmshell4", QStringList() << "display" );
}

void RandrMonitorModule::resumedFromSuspend()
{
    RandRDisplay display;
    QList< RandROutput* > m_connectedOutputs, m_validCrtcOutputs;
    m_connectedOutputs = connectedOutputs( display );
    m_validCrtcOutputs = validCrtcOutputs( display );
    if( m_connectedOutputs.count() == 0 )
        return;
    // We have at least one connected output.
    // We check all outputs with valid crtc if they are still connected.
    // If not, we are going to disable them.
    QList<RandROutput*> outputsToDisable;
    foreach( RandROutput* output, m_validCrtcOutputs )
    {
        if( !output->isConnected() )
            outputsToDisable.append( output );
    }
    // If no active output is still connected we are going to enable the first connected output.
    if( outputsToDisable.size() == m_validCrtcOutputs.size() )
        enableOutput( m_connectedOutputs[0], true);
    // Now we can disable the disconnected outputs
    foreach( RandROutput* output, outputsToDisable)
    {
        enableOutput( output, false );
    }
}

void RandrMonitorModule::enableOutput( RandROutput* output, bool enable )
{   // a bit lame, but I don't know how to do this easily with this codebase :-/
    KProcess::execute( QStringList() << "xrandr" << "--output" << output->name() << ( enable ? "--auto" : "--off" ));
}

QList< RandROutput* > RandrMonitorModule::connectedOutputs( RandRDisplay &display )
{
    return outputs( display, true, false, false );
}

QList< RandROutput* > RandrMonitorModule::activeOutputs( RandRDisplay &display )
{
    return outputs( display, false, true, false );
}

QList< RandROutput* > RandrMonitorModule::validCrtcOutputs( RandRDisplay &display )
{
    return outputs( display, false, false, true );
}

QList< RandROutput* > RandrMonitorModule::outputs( RandRDisplay &display, bool connected, bool active, bool validCrtc )
{
    QList< RandROutput* > outputs;
    for( int scr = 0; scr < display.numScreens(); ++scr )
    {
        foreach( RandROutput* output, display.screen( scr )->outputs() )
        {
            if( !output->isConnected() && connected )
                continue;
            if( !output->isActive() && active )
                continue;
            if( !output->crtc()->isValid() && validCrtc )
                continue;
            if( !outputs.contains( output ) )
                outputs.append( output );
        }
    }
    return outputs;
}

bool RandrMonitorHelper::x11Event( XEvent* e )
{
    module->processX11Event( e );
    return QWidget::x11Event( e );
}

#include "randrmonitor.moc"
