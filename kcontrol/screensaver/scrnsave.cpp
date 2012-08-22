/*
* scrnsave.cpp
* Copyright 1997       Matthias Hoelzer
* Copyright 1996,1999,2002    Martin R. Jones
* Copyright 2004       Chris Howells
* Copyright 2007-2008  Benjamin Meyer <ben@meyerhome.net>
* Copyright 2007-2008  Hamish Rodda <rodda@kde.org>
* Copyright 2009       Dario Andres Rodriguez <andresbajotierra@gmail.com>
* Copyright 2009       Davide Bettio
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config-workspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QTreeWidget>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QTextStream>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QtDBus/QtDBus>

#include <QDesktopWidget>

#include <kmacroexpander.h>
#include <KShell>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KApplication>
#include <KDebug>
#include <KDialog>
#include <KAboutData>
#include <KIcon>
#include <KNumInput>
#include <KProcess>
//#include <KServiceGroup>
#include <KPluginFactory>
#include <KPluginLoader>

#include <kscreensaver_interface.h>
#include <kworkspace/screenpreviewwidget.h>

#include <X11/Xlib.h>
#include <fixx11h.h>

#include "scrnsave.h"
#include <QX11Info>


const uint widgetEventMask =                 // X event mask
(uint)(
       ExposureMask |
       PropertyChangeMask |
       StructureNotifyMask
      );

//===========================================================================
// DLL Interface for kcontrol
K_PLUGIN_FACTORY(KSSFactory,
        registerPlugin<KScreenSaver>();
)
K_EXPORT_PLUGIN(KSSFactory("kcmscreensaver"))


static QString findExe(const QString &exe) {
    QString result = KStandardDirs::locate("exe", exe);
    if (result.isEmpty())
        result = KStandardDirs::findExe(exe);
    return result;
}

KScreenSaver::KScreenSaver(QWidget *parent, const QVariantList&)
    : KCModule(KSSFactory::componentData(), parent)
{
    mTesting = false;

    setQuickHelp( i18n("<h1>Screen Saver</h1> <p>This module allows you to enable and"
       " configure a screen saver. Note that you can enable a screen saver"
       " even if you have power saving features enabled for your display.</p>"
       " <p>Besides providing an endless variety of entertainment and"
       " preventing monitor burn-in, a screen saver also gives you a simple"
       " way to lock your display if you are going to leave it unattended"
       " for a while. If you want the screen saver to lock the session, make sure you enable"
       " the \"Require password\" feature of the screen saver; if you do not, you can still"
       " explicitly lock the session using the desktop's \"Lock Session\" action.</p>"));

    setButtons( KCModule::Help |  KCModule::Apply );


    setupUi(this);

    readSettings();

    mEnabledCheckBox->setChecked(mEnabled);
    connect(mEnabledCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEnable(bool)));

    mWaitEdit->setRange(1, INT_MAX);
    mWaitEdit->setSuffix(ki18ncp("unit of time. minutes until the screensaver is triggered",
                                 " minute", " minutes"));
    mWaitEdit->setValue(mTimeout/60);
    mWaitEdit->setEnabled(mEnabled);
    connect(mWaitEdit, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimeoutChanged(int)));

    mLockCheckBox->setEnabled( mEnabled );
    mLockCheckBox->setChecked( mLock );
    connect( mLockCheckBox, SIGNAL(toggled(bool)),
             this, SLOT(slotLock(bool)) );

    mWaitLockEdit->setRange(1, 300);
    mWaitLockEdit->setSuffix(ki18np(" second", " seconds"));
    mWaitLockEdit->setValue(mLockTimeout/1000);
    mWaitLockEdit->setEnabled(mEnabled && mLock);
    connect(mWaitLockEdit, SIGNAL(valueChanged(int)),
            this, SLOT(slotLockTimeoutChanged(int)));

    mPlasmaCheckBox->setChecked(mPlasmaEnabled);
    connect(mPlasmaCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotEnablePlasma(bool)));

    mPlasmaSetup->setEnabled(mPlasmaEnabled);
    connect(mPlasmaSetup, SIGNAL(clicked()), this, SLOT(slotPlasmaSetup()));

    if (mImmutable)
    {
       setButtons(buttons() & ~Default);
       mSettingsGroup->setEnabled(false);
    }

    // finding the savers can take some time, so defer loading until
    // we've started up.
    mNumLoaded = 0;
    mLoadTimer = new QTimer( this );
    connect( mLoadTimer, SIGNAL(timeout()), SLOT(findSavers()) );
    mLoadTimer->start( 100 );
    mChanged = false;
    emit changed(false);

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmscreensaver"), 0, ki18n("KDE Screen Saver Control Module"),
                  0, KLocalizedString(), KAboutData::License_GPL,
                  ki18n("(c) 1997-2002 Martin R. Jones\n"
                  "(c) 2003-2004 Chris Howells"));
    about->addAuthor(ki18n("Chris Howells"), KLocalizedString(), "howells@kde.org");
    about->addAuthor(ki18n("Martin R. Jones"), KLocalizedString(), "jones@kde.org");

    setAboutData( about );

}

//---------------------------------------------------------------------------
//
KScreenSaver::~KScreenSaver()
{
}

//---------------------------------------------------------------------------
//
void KScreenSaver::load()
{
    readSettings();

    updateValues();
    mChanged = false;

    emit changed(false);
}

//------------------------------------------------------------After---------------
//
void KScreenSaver::readSettings()
{
    KConfigGroup config( KSharedConfig::openConfig( "kscreensaverrc"), "ScreenSaver" );

    mImmutable = config.isImmutable();

    mEnabled = config.readEntry("Enabled", false);
    mTimeout = config.readEntry("Timeout", 300);
    mLockTimeout = config.readEntry("LockGrace", 60000);
    mLock = config.readEntry("Lock", false);
    mPlasmaEnabled = config.readEntry("PlasmaEnabled", false);

    if (mTimeout < 60) mTimeout = 60;
    if (mLockTimeout < 0) mLockTimeout = 0;
    if (mLockTimeout > 300000) mLockTimeout = 300000;

    mChanged = false;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::updateValues()
{
    if (mEnabled)
    {
        mWaitEdit->setValue(mTimeout/60);
    }
    else
    {
        mWaitEdit->setValue(0);
    }

    mWaitLockEdit->setValue(mLockTimeout/1000);
    mLockCheckBox->setChecked(mLock);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::defaults()
{
    if (mImmutable) return;

    slotTimeoutChanged( 5 );
    slotLockTimeoutChanged( 60 );
    slotLock( false );
    mEnabledCheckBox->setChecked(false);
    mPlasmaCheckBox->setChecked(false);
    mPlasmaSetup->setEnabled(false);

    updateValues();

    emit changed(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::save()
{
    if ( !mChanged )
        return;

    KConfigGroup config(KSharedConfig::openConfig( "kscreensaverrc"), "ScreenSaver" );

    config.writeEntry("Enabled", mEnabled);
    config.writeEntry("Timeout", mTimeout);
    config.writeEntry("LockGrace", mLockTimeout);
    config.writeEntry("Lock", mLock);
    config.writeEntry("PlasmaEnabled", mPlasmaEnabled);

    config.sync();

    org::kde::screensaver kscreensaver("org.kde.screensaver", "/ScreenSaver", QDBusConnection::sessionBus());
    kscreensaver.configure();

    mChanged = false;
    emit changed(false);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotEnable(bool e)
{
    mEnabled = e;
    mWaitEdit->setEnabled( e );
    mLockCheckBox->setEnabled( e );
    mWaitLockEdit->setEnabled( e && mLock );
    mChanged = true;
    emit changed(true);
}

void KScreenSaver::slotEnablePlasma(bool enable)
{
    mPlasmaEnabled = enable;
    //FIXME even though the button's enabled, plasma isn't until the user hits apply
    //so the button will just show the screensaver, no plasma.
    //what should I do about this?
    mPlasmaSetup->setEnabled(mPlasmaEnabled);
    mChanged = true;
    emit changed(true);
}

void KScreenSaver::slotPlasmaSetup()
{
    org::kde::screensaver kscreensaver("org.kde.screensaver", "/ScreenSaver", QDBusConnection::sessionBus());
    kscreensaver.setupPlasma();
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotTimeoutChanged(int to )
{
    mTimeout = to * 60;
    mChanged = true;
    emit changed(true);
}

//-----------------------------------------------------------------------
//
void KScreenSaver::slotLockTimeoutChanged(int to )
{
    mLockTimeout = to * 1000;
    mChanged = true;
    emit changed(true);
}


//---------------------------------------------------------------------------
//
void KScreenSaver::slotLock( bool l )
{
    mLock = l;
    mWaitLockEdit->setEnabled( l );
    mChanged = true;
    emit changed(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotSetupDone()
{
    emit changed(true);
}


#include "scrnsave.moc"
