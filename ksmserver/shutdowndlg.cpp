/*****************************************************************
ksmserver - the KDE session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>
Copyright 2007 Urs Wolfer <uwolfer @ kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "shutdowndlg.h"
#include "plasma/framesvg.h"
#include "plasma/theme.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QQuickItem>
#include <QTimer>
#include <QFile>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingCall>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlPropertyMap>
#include <QPainter>
#include <QStandardPaths>
#include <QtX11Extras/qx11info_x11.h>

#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kuser.h>
#include <Solid/PowerManagement>
#include <kwindowsystem.h>
#include <netwm.h>
#include <kdeclarative/kdeclarative.h>

#include <stdio.h>
#include <kxerrorhandler.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <kworkspace/kdisplaymanager.h>

#include <config-workspace.h>

//#include "logouteffect.h"
#include "shutdowndlg.moc"

#include <kjob.h>
#include <qstandardpaths.h>

#define FONTCOLOR "#bfbfbf"

KSMShutdownFeedback * KSMShutdownFeedback::s_pSelf = 0L;

KSMShutdownFeedback::KSMShutdownFeedback()
 : QWidget( 0L, Qt::Popup ),
    m_currentY( 0 ), initialized( false )
{
    setObjectName( QStringLiteral("feedbackwidget") );
    setAttribute( Qt::WA_NoSystemBackground );
    setAttribute( Qt::WA_PaintOnScreen );
    setGeometry( QApplication::desktop()->geometry() );
    m_pixmap = QPixmap( size() );
    QTimer::singleShot( 10, this, SLOT(slotPaintEffect()) );
}


void KSMShutdownFeedback::paintEvent( QPaintEvent* )
{
    if ( !initialized )
        return;

    QPainter painter( this );
    painter.setCompositionMode( QPainter::CompositionMode_Source );
    painter.drawPixmap( 0, 0, m_pixmap );
}

void KSMShutdownFeedback::slotPaintEffect()
{
//    effect = LogoutEffect::create(this, &m_pixmap);
//    connect(effect, SIGNAL(initialized()),
//            this,   SLOT  (slotPaintEffectInitialized()));

//    effect->start();
}

void KSMShutdownFeedback::slotPaintEffectInitialized()
{
    initialized = true;
}

void KSMShutdownFeedback::start()
{
    if( KWindowSystem::compositingActive()) {
        // HACK do properly
        Display* dpy = QX11Info::display();
        char net_wm_cm_name[ 100 ];
        sprintf( net_wm_cm_name, "_NET_WM_CM_S%d", DefaultScreen( dpy ));
        Atom net_wm_cm = XInternAtom( dpy, net_wm_cm_name, False );
        Window sel = XGetSelectionOwner( dpy, net_wm_cm );
        Atom hack = XInternAtom( dpy, "_KWIN_LOGOUT_EFFECT", False );
        bool wmsupport = false;
        if( sel != None ) {
            KXErrorHandler handler;
            int cnt;
            Atom* props = XListProperties( dpy, sel, &cnt );
            if( !handler.error( false ) && props != NULL && qFind( props, props + cnt, hack ) != props + cnt )
                wmsupport = true;
            if( props != NULL )
                XFree( props );
        }
        if( wmsupport ) {
            // Announce that the user MAY be logging out (Intended for the compositor)
            Atom announce = XInternAtom(dpy, "_KDE_LOGGING_OUT", False);
            unsigned char dummy = 0;
            XChangeProperty(dpy, QX11Info::appRootWindow(), announce, announce, 8, PropModeReplace,
                &dummy, 1);

            // Don't show our own effect
            return;
        }
    }
    s_pSelf = new KSMShutdownFeedback();
    s_pSelf->show();
}

void KSMShutdownFeedback::stop()
{
    delete s_pSelf;
    s_pSelf = NULL;
}

void KSMShutdownFeedback::logoutCanceled()
{
    if( KWindowSystem::compositingActive()) {
        // We are no longer logging out, announce (Intended for the compositor)
        Display* dpy = QX11Info::display();
        Atom announce = XInternAtom(dpy, "_KDE_LOGGING_OUT", False);
        XDeleteProperty(QX11Info::display(), QX11Info::appRootWindow(), announce);
    }
}

////////////

Q_DECLARE_METATYPE(Solid::PowerManagement::SleepState)
#include <QVBoxLayout>

KSMShutdownDlg::KSMShutdownDlg( QWidget* parent,
                                bool maysd, bool choose, KWorkSpace::ShutdownType sdtype,
                                const QString& theme)
  : QDialog( parent/*, Qt::Popup */) //krazy:exclude=qclasses
    // this is a WType_Popup on purpose. Do not change that! Not
    // having a popup here has severe side effects.
{
    winId(); // workaround for Qt4.3 setWindowRole() assert
    setWindowRole( QStringLiteral("logoutdialog") );

    // Qt doesn't set this on unmanaged windows
    //FIXME: or does it?
    XChangeProperty( QX11Info::display(), winId(),
        XInternAtom( QX11Info::display(), "WM_WINDOW_ROLE", False ), XA_STRING, 8, PropModeReplace,
        (unsigned char *)"logoutdialog", strlen( "logoutdialog" ));

    KDialog::centerOnScreen(this, -3);

     setMinimumSize(300, 200);
    //kDebug() << "Creating QML view";
    QVBoxLayout *vbox = new QVBoxLayout(this);
    m_view = new QQuickView( );
    QWidget *windowContainer = QWidget::createWindowContainer(m_view, this);
    vbox->addWidget(windowContainer);
    windowContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QQmlContext *context = m_view->rootContext();
    context->setContextProperty(QStringLiteral("maysd"), maysd);
    context->setContextProperty(QStringLiteral("choose"), choose);
    context->setContextProperty(QStringLiteral("sdtype"), sdtype);

    QQmlPropertyMap *mapShutdownType = new QQmlPropertyMap(this);
    mapShutdownType->insert(QStringLiteral("ShutdownTypeDefault"), QVariant::fromValue((int)KWorkSpace::ShutdownTypeDefault));
    mapShutdownType->insert(QStringLiteral("ShutdownTypeNone"), QVariant::fromValue((int)KWorkSpace::ShutdownTypeNone));
    mapShutdownType->insert(QStringLiteral("ShutdownTypeReboot"), QVariant::fromValue((int)KWorkSpace::ShutdownTypeReboot));
    mapShutdownType->insert(QStringLiteral("ShutdownTypeHalt"), QVariant::fromValue((int)KWorkSpace::ShutdownTypeHalt));
    mapShutdownType->insert(QStringLiteral("ShutdownTypeLogout"), QVariant::fromValue((int)KWorkSpace::ShutdownTypeLogout));
    context->setContextProperty(QStringLiteral("ShutdownType"), mapShutdownType);

    QQmlPropertyMap *mapSpdMethods = new QQmlPropertyMap(this);
    QSet<Solid::PowerManagement::SleepState> spdMethods = Solid::PowerManagement::supportedSleepStates();
    mapSpdMethods->insert(QStringLiteral("StandbyState"), QVariant::fromValue(spdMethods.contains(Solid::PowerManagement::StandbyState)));
    mapSpdMethods->insert(QStringLiteral("SuspendState"), QVariant::fromValue(spdMethods.contains(Solid::PowerManagement::SuspendState)));
    mapSpdMethods->insert(QStringLiteral("HibernateState"), QVariant::fromValue(spdMethods.contains(Solid::PowerManagement::HibernateState)));
    context->setContextProperty(QStringLiteral("spdMethods"), mapSpdMethods);

    QString bootManager = KConfig(QStringLiteral(KDE_CONFDIR "/kdm/kdmrc"), KConfig::SimpleConfig)
                          .group("Shutdown")
                          .readEntry("BootManager", "None");
    context->setContextProperty(QStringLiteral("bootManager"), bootManager);

    QStringList options;
    int def, cur;
    if ( KDisplayManager().bootOptions( rebootOptions, def, cur ) ) {
        if ( cur > -1 ) {
            def = cur;
        }
    }
    QQmlPropertyMap *rebootOptionsMap = new QQmlPropertyMap(this);
    rebootOptionsMap->insert(QStringLiteral("options"), QVariant::fromValue(rebootOptions));
    rebootOptionsMap->insert(QStringLiteral("default"), QVariant::fromValue(def));
    context->setContextProperty(QStringLiteral("rebootOptions"), rebootOptionsMap);

    setModal( true );

    // window stuff
//     setFlags(Qt::X11BypassWindowManagerHint);
//     windowContainer->setFrameShape(QFrame::NoFrame);
    windowContainer->setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet(QStringLiteral("background:transparent;"));
    QPalette pal = windowContainer->palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    windowContainer->setPalette(pal);
//    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // engine stuff
    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(m_view->engine());
    kdeclarative.initialize();
    kdeclarative.setupBindings();
    windowContainer->installEventFilter(this);

    QString fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("ksmserver/themes/%1/main.qml").arg(theme));
    if (QFile::exists(fileName)) {
        //kDebug() << "Using QML theme" << fileName;
        m_view->setSource(QUrl::fromLocalFile(fileName));
    }
    QQuickItem *rootObject = m_view->rootObject();
    connect(rootObject, SIGNAL(logoutRequested()), SLOT(slotLogout()));
    connect(rootObject, SIGNAL(haltRequested()), SLOT(slotHalt()));
    connect(rootObject, SIGNAL(suspendRequested(int)), SLOT(slotSuspend(int)) );
    connect(rootObject, SIGNAL(rebootRequested()), SLOT(slotReboot()));
    connect(rootObject, SIGNAL(rebootRequested2(int)), SLOT(slotReboot(int)) );
    connect(rootObject, SIGNAL(cancelRequested()), SLOT(reject()));
    connect(rootObject, SIGNAL(lockScreenRequested()), SLOT(slotLockScreen()));

    adjustSize();
}

bool KSMShutdownDlg::eventFilter ( QObject * watched, QEvent * event )
{
    if (watched == m_view && event->type() == QEvent::Resize) {
        adjustSize();
    }
    return QDialog::eventFilter(watched, event);
}

void KSMShutdownDlg::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent( e );

    if( KWindowSystem::compositingActive()) {
        clearMask();
    } else {
        setMask(m_view->mask());
    }

    KDialog::centerOnScreen(this, -3);
}

void KSMShutdownDlg::slotLogout()
{
    m_shutdownType = KWorkSpace::ShutdownTypeNone;
    accept();
}

void KSMShutdownDlg::slotReboot()
{
    // no boot option selected -> current
    m_bootOption.clear();
    m_shutdownType = KWorkSpace::ShutdownTypeReboot;
    accept();
}

void KSMShutdownDlg::slotReboot(int opt)
{
    if (int(rebootOptions.size()) > opt)
        m_bootOption = rebootOptions[opt];
    m_shutdownType = KWorkSpace::ShutdownTypeReboot;
    accept();
}


void KSMShutdownDlg::slotLockScreen()
{
    m_bootOption.clear();
    QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.kde.screensaver"),
                                                       QStringLiteral("/ScreenSaver"),
                                                       QStringLiteral("org.freedesktop.ScreenSaver"),
                                                       QStringLiteral("Lock"));
    QDBusConnection::sessionBus().asyncCall(call);
    reject();
}

void KSMShutdownDlg::slotHalt()
{
    m_bootOption.clear();
    m_shutdownType = KWorkSpace::ShutdownTypeHalt;
    accept();
}


void KSMShutdownDlg::slotSuspend(int spdMethod)
{
    m_bootOption.clear();
    switch (spdMethod) {
        case Solid::PowerManagement::StandbyState:
        case Solid::PowerManagement::SuspendState:
            Solid::PowerManagement::requestSleep(Solid::PowerManagement::SuspendState, 0, 0);
            break;
        case Solid::PowerManagement::HibernateState:
            Solid::PowerManagement::requestSleep(Solid::PowerManagement::HibernateState, 0, 0);
            break;
    }
    reject();
}

bool KSMShutdownDlg::confirmShutdown(
        bool maysd, bool choose, KWorkSpace::ShutdownType& sdtype, QString& bootOption,
        const QString& theme)
{
    KSMShutdownDlg* l = new KSMShutdownDlg( 0,
                                            //KSMShutdownFeedback::self(),
                                            maysd, choose, sdtype, theme );
    XClassHint classHint;
    classHint.res_name = const_cast<char*>("ksmserver");
    classHint.res_class = const_cast<char*>("ksmserver");

    XSetClassHint(QX11Info::display(), l->winId(), &classHint);
    bool result = l->exec();
    sdtype = l->m_shutdownType;
    bootOption = l->m_bootOption;

    delete l;

    return result;
}
