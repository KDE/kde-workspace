//-----------------------------------------------------------------------------
//
// KDE Display screen saver setup module
//
// Copyright (c)  Martin R. Jones 1996,1999,2002
//
// Converted to a kcc module by Matthias Hoelzer 1997
//


#include <config-workspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <kservicetypetrader.h>
#include <kstandarddirs.h>
#include <QCheckBox>
#include <Qt3Support/Q3Header>
#include <QLabel>
#include <QLayout>
#include <Qt3Support/Q3CheckListItem>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <kmacroexpander.h>
#include <kshell.h>

//Added by qt3to4:
#include <QPixmap>
#include <QTextStream>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>


#include <QtDBus/QtDBus>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <knuminput.h>
#include <k3process.h>
#include <kservicegroup.h>

#include <X11/Xlib.h>
#include <fixx11h.h>

#include "scrnsave.h"
#include <QX11Info>
#include <QDesktopWidget>
#include <kscreensaver_interface.h>
#include <KPluginFactory>
#include <KPluginLoader>

template class QList<SaverConfig*>;

const uint widgetEventMask =                 // X event mask
(uint)(
       ExposureMask |
       PropertyChangeMask |
       StructureNotifyMask
      );

//===========================================================================
// DLL Interface for kcontrol
K_PLUGIN_FACTORY(KSSFactory,
        registerPlugin<KScreenSaver>(); // K_EXPORT_COMPONENT_FACTORY (screensaver
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
    mSetupProc = 0;
    mPreviewProc = 0;
    mTestWin = 0;
    mTestProc = 0;
    mPrevSelected = -2;
    mMonitor = 0;
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

    setButtons( KCModule::Help | KCModule::Default | KCModule::Apply );



    readSettings();

    mSetupProc = new K3Process;
    connect(mSetupProc, SIGNAL(processExited(K3Process *)),
            this, SLOT(slotSetupDone(K3Process *)));

    mPreviewProc = new K3Process;
    connect(mPreviewProc, SIGNAL(processExited(K3Process *)),
            this, SLOT(slotPreviewExited(K3Process *)));

    QBoxLayout *topLayout = new QHBoxLayout(this);
    topLayout->setMargin(0);
    topLayout->setSpacing(KDialog::spacingHint());

    // left column
    QVBoxLayout *leftColumnLayout = new QVBoxLayout( );
    topLayout->addItem( leftColumnLayout );
    leftColumnLayout->setSpacing( KDialog::spacingHint() );

    mSaverGroup = new QGroupBox(i18n("Screen Saver"), this );
    QVBoxLayout *groupLayout = new QVBoxLayout( mSaverGroup );
    leftColumnLayout->addWidget(mSaverGroup);
    leftColumnLayout->setStretchFactor( mSaverGroup, 10 );

    mSaverListView = new Q3ListView( mSaverGroup );
    mSaverListView->setMinimumHeight( 120 );
    mSaverListView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    mSaverListView->addColumn("");
    mSaverListView->header()->hide();
    mSelected = -1;
    groupLayout->addWidget( mSaverListView, 10 );
    connect( mSaverListView, SIGNAL(doubleClicked ( Q3ListViewItem *)), this, SLOT( slotSetup()));
    mSaverListView->setWhatsThis( i18n("Select the screen saver to use.") );

    QBoxLayout* hlay = new QHBoxLayout();
    groupLayout->addLayout(hlay);
    mSetupBt = new QPushButton( i18n("&Setup..."), mSaverGroup );
    connect( mSetupBt, SIGNAL( clicked() ), SLOT( slotSetup() ) );
    mSetupBt->setEnabled(false);
    hlay->addWidget( mSetupBt );
    mSetupBt->setWhatsThis( i18n("Configure the screen saver's options, if any.") );

    mTestBt = new QPushButton( i18n("&Test"), mSaverGroup );
    connect( mTestBt, SIGNAL( clicked() ), SLOT( slotTest() ) );
    mTestBt->setEnabled(false);
    hlay->addWidget( mTestBt );
    mTestBt->setWhatsThis( i18n("Show a full screen preview of the screen saver.") );

    mSettingsGroup = new QGroupBox( i18n("Settings"), this );
    groupLayout = new QVBoxLayout( mSettingsGroup );
    leftColumnLayout->addWidget( mSettingsGroup );

    mEnabledCheckBox = new QCheckBox(i18n(
        "Start a&utomatically"), mSettingsGroup);
    mEnabledCheckBox->setChecked(mEnabled);
    mEnabledCheckBox->setWhatsThis( i18n(
        "Automatically start the screen saver after a period of inactivity.") );
    connect(mEnabledCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEnable(bool)));
    groupLayout->addWidget(mEnabledCheckBox);

    QBoxLayout *hbox = new QHBoxLayout();
    groupLayout->addLayout(hbox);
    hbox->addSpacing(30);
    mActivateLbl = new QLabel(i18n("After:"), mSettingsGroup);
    mActivateLbl->setEnabled(mEnabled);
    hbox->addWidget(mActivateLbl);
    mWaitEdit = new QSpinBox(mSettingsGroup);
    //mWaitEdit->setSteps(1, 10);
    mWaitEdit->setRange(1, INT_MAX);
    mWaitEdit->setSuffix(i18n(" min"));
    mWaitEdit->setValue(mTimeout/60);
    mWaitEdit->setEnabled(mEnabled);
    connect(mWaitEdit, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimeoutChanged(int)));
    mActivateLbl->setBuddy(mWaitEdit);
    hbox->addWidget(mWaitEdit);
    hbox->addStretch(1);
    QString wtstr = i18n(
        "The period of inactivity "
        "after which the screen saver should start.");
    mActivateLbl->setWhatsThis( wtstr );
    mWaitEdit->setWhatsThis( wtstr );

    mLockCheckBox = new QCheckBox( i18n(
        "&Require password to stop"), mSettingsGroup );
    mLockCheckBox->setEnabled( mEnabled );
    mLockCheckBox->setChecked( mLock );
    connect( mLockCheckBox, SIGNAL( toggled( bool ) ),
             this, SLOT( slotLock( bool ) ) );
    groupLayout->addWidget(mLockCheckBox);
    mLockCheckBox->setWhatsThis( i18n(
        "Prevent potential unauthorized use by requiring a password"
        " to stop the screen saver.") );
    hbox = new QHBoxLayout();
    groupLayout->addLayout(hbox);
    hbox->addSpacing(30);
    mLockLbl = new QLabel(i18n("After:"), mSettingsGroup);
    mLockLbl->setEnabled(mEnabled && mLock);
    mLockLbl->setWhatsThis( i18n(
        "The amount of time, after the screen saver has started, to ask for the unlock password.") );
    hbox->addWidget(mLockLbl);
    mWaitLockEdit = new QSpinBox(mSettingsGroup);
    //mWaitLockEdit->setSteps(1, 10);
    mWaitLockEdit->setRange(1, 300);
    mWaitLockEdit->setSuffix(i18n(" sec"));
    mWaitLockEdit->setValue(mLockTimeout/1000);
    mWaitLockEdit->setEnabled(mEnabled && mLock);
    if ( mWaitLockEdit->sizeHint().width() <
         mWaitEdit->sizeHint().width() ) {
        mWaitLockEdit->setFixedWidth( mWaitEdit->sizeHint().width() );
        mWaitEdit->setFixedWidth( mWaitEdit->sizeHint().width() );
    }
    else {
        mWaitEdit->setFixedWidth( mWaitLockEdit->sizeHint().width() );
        mWaitLockEdit->setFixedWidth( mWaitLockEdit->sizeHint().width() );
    }
    connect(mWaitLockEdit, SIGNAL(valueChanged(int)),
            this, SLOT(slotLockTimeoutChanged(int)));
    mLockLbl->setBuddy(mWaitLockEdit);
    hbox->addWidget(mWaitLockEdit);
    hbox->addStretch(1);
    QString wltstr = i18n(
        "Choose the period "
        "after which the display will be locked. ");
    mLockLbl->setWhatsThis( wltstr );
    mWaitLockEdit->setWhatsThis( wltstr );

    // right column
    QBoxLayout* rightColumnLayout = new QVBoxLayout();
    topLayout->addItem( rightColumnLayout );
    rightColumnLayout->setSpacing( KDialog::spacingHint() );

    mMonitorLabel = new QLabel( this );
    mMonitorLabel->setAlignment( Qt::AlignCenter );
    mMonitorLabel->setPixmap( QPixmap(KStandardDirs::locate("data",
                         "kcontrol/pics/monitor.png")));
    rightColumnLayout->addWidget(mMonitorLabel, 0);
    mMonitorLabel->setWhatsThis( i18n("A preview of the selected screen saver.") );

    QBoxLayout* advancedLayout = new QHBoxLayout();
    rightColumnLayout->addItem( advancedLayout );
    advancedLayout->setSpacing( 3 );
    advancedLayout->addWidget( new QWidget( this ) );
    QPushButton* advancedBt = new QPushButton(
        i18n( "Advanced &Options" ), this );
    advancedBt->setObjectName("advancedBtn");
    advancedBt->setSizePolicy( QSizePolicy(
        QSizePolicy::Fixed, QSizePolicy::Fixed) );
    connect( advancedBt, SIGNAL( clicked() ),
             this, SLOT( slotAdvanced() ) );
    advancedLayout->addWidget( advancedBt );
    advancedLayout->addWidget( new QWidget( this ) );

    rightColumnLayout->addStretch();

    if (mImmutable)
    {
       setButtons(buttons() & ~Default);
       mSettingsGroup->setEnabled(false);
       mSaverGroup->setEnabled(false);
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
void KScreenSaver::resizeEvent( QResizeEvent * )
{

  if (mMonitor)
    {
      mMonitor->setGeometry( (mMonitorLabel->width()-200)/2+23,
                 (mMonitorLabel->height()-186)/2+14, 151, 115 );
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::mousePressEvent( QMouseEvent *)
{
    if ( mTesting )
	slotStopTest();
}

//---------------------------------------------------------------------------
//
void KScreenSaver::keyPressEvent( QKeyEvent *)
{
    if ( mTesting )
	slotStopTest();
}
//---------------------------------------------------------------------------
//
KScreenSaver::~KScreenSaver()
{
    if (mPreviewProc)
    {
        if (mPreviewProc->isRunning())
        {
            mPreviewProc->kill( );
            mPreviewProc->wait( );
        }
        delete mPreviewProc;
    }

    delete mTestProc;
    delete mSetupProc;
    delete mTestWin;

    qDeleteAll(mSaverList);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::load()
{
    readSettings();

//with the following line, the Test and Setup buttons are not enabled correctly
//if no saver was selected, the "Reset" and the "Enable screensaver", it is only called when starting and when pressing reset, aleXXX
//    mSelected = -1;
    int i = 0;
    Q3ListViewItem *selectedItem = 0;
	Q_FOREACH( SaverConfig* saver, mSaverList ){
        if (saver->file() == mSaver)
        {
            selectedItem = mSaverListView->findItem ( saver->name(), 0 );
            if (selectedItem) {
                mSelected = i;
                break;
            }
        }
        i++;
    }
    if ( selectedItem )
    {
      mSaverListView->setSelected( selectedItem, true );
      mSaverListView->setCurrentItem( selectedItem );
      slotScreenSaver( selectedItem );
    }

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
    mSaver = config.readEntry("Saver");

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

    slotScreenSaver( 0 );

    Q3ListViewItem *item = mSaverListView->firstChild();
    if (item) {
        mSaverListView->setSelected( item, true );
        mSaverListView->setCurrentItem( item );
        mSaverListView->ensureItemVisible( item );
    }
    slotTimeoutChanged( 5 );
    slotLockTimeoutChanged( 60 );
    slotLock( false );
    mEnabledCheckBox->setChecked(false);

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

    if ( !mSaver.isEmpty() )
        config.writeEntry("Saver", mSaver);
    config.sync();

    org::kde::screensaver kscreensaver("org.kde.screensaver", "/ScreenSaver", QDBusConnection::sessionBus());
    kscreensaver.configure();

    mChanged = false;
    emit changed(false);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::findSavers()
{
    if ( !mNumLoaded ) {
	mSaverServices = KServiceTypeTrader::self()->query( "ScreenSaver");
        new Q3ListViewItem ( mSaverListView, i18n("Loading...") );
        if ( mSaverServices.isEmpty() )
            mLoadTimer->stop();
        else
            mLoadTimer->start( 50 );
    }
    for( KService::List::const_iterator it = mSaverServices.begin();
        it != mSaverServices.end(); it++,mNumLoaded++)
    {
      SaverConfig *saver = new SaverConfig;
      QString file = KStandardDirs::locate("services", (*it)->entryPath());
      if (saver->read(file)) {
	      mSaverList.append(saver);
        } else
            delete saver;
    }

    if ( mNumLoaded == mSaverServices.count() ) {
        Q3ListViewItem *selectedItem = 0;
        int categoryCount = 0;
        int indx = 0;

        mLoadTimer->stop();
        delete mLoadTimer;
		qSort(mSaverList.begin(), mSaverList.end());

        mSelected = -1;
        mSaverListView->clear();
        Q_FOREACH( SaverConfig *s, mSaverList )
        {
            Q3ListViewItem *item;
            if (s->category().isEmpty())
                item = new Q3ListViewItem ( mSaverListView, s->name(), '2' + s->name() );
            else
            {
                Q3ListViewItem *categoryItem = mSaverListView->findItem( s->category(), 0 );
                if ( !categoryItem ) {
                    categoryItem = new Q3ListViewItem ( mSaverListView, s->category(), '1' + s->category() );
                    categoryItem->setPixmap ( 0, SmallIcon ( "preferences-desktop-screensaver" ) );
                }
                item = new Q3ListViewItem ( categoryItem, s->name(), s->name() );
                categoryCount++;
            }
            if (s->file() == mSaver) {
                mSelected = indx;
                selectedItem = item;
            }
            indx++;
        }

        // Delete categories with only one item
        Q3ListViewItemIterator it ( mSaverListView );
        for ( ; it.current(); it++ )
            if ( it.current()->childCount() == 1 ) {
               Q3ListViewItem *item = it.current()->firstChild();
               it.current()->takeItem( item );
               mSaverListView->insertItem ( item );
               delete it.current();
               categoryCount--;
            }

        mSaverListView->setRootIsDecorated ( categoryCount > 0 );
        mSaverListView->setSorting ( 1 );

        if ( mSelected > -1 )
        {
            mSaverListView->setSelected(selectedItem, true);
            mSaverListView->setCurrentItem(selectedItem);
            mSaverListView->ensureItemVisible(selectedItem);
            mSetupBt->setEnabled(!mSaverList.at(mSelected)->setup().isEmpty());
            mTestBt->setEnabled(true);
        }

        connect( mSaverListView, SIGNAL( currentChanged( Q3ListViewItem * ) ),
                 this, SLOT( slotScreenSaver( Q3ListViewItem * ) ) );

        setMonitor();
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::setMonitor()
{
    if (mPreviewProc->isRunning())
    // CC: this will automatically cause a "slotPreviewExited"
    // when the viewer exits
    mPreviewProc->kill();
    else
    slotPreviewExited(mPreviewProc);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotPreviewExited(K3Process *)
{
    // Ugly hack to prevent continual respawning of savers that crash
    if (mSelected == mPrevSelected)
        return;

    if ( mSaverList.isEmpty() ) // safety check
        return;

    // Some xscreensaver hacks do something nasty to the window that
    // requires a new one to be created (or proper investigation of the
    // problem).
    delete mMonitor;

    mMonitor = new KSSMonitor(mMonitorLabel);
    QPalette palette;
    palette.setColor(mMonitor->backgroundRole(), Qt::black);
    mMonitor->setPalette(palette);
    mMonitor->setGeometry((mMonitorLabel->width()-200)/2+23,
                          (mMonitorLabel->height()-186)/2+14, 151, 115);
    mMonitor->show();
    // So that hacks can XSelectInput ButtonPressMask
    XSelectInput(QX11Info::display(), mMonitor->winId(), widgetEventMask );

    if (mSelected >= 0) {
        mPreviewProc->clearArguments();

        QString saver = mSaverList.at(mSelected)->saver();
        QHash<QChar, QString> keyMap;
        keyMap.insert('w', QString::number(mMonitor->winId()));
        *mPreviewProc << KShell::splitArgs(KMacroExpander::expandMacrosShellQuote(saver, keyMap));

        mPreviewProc->start();
    }

    mPrevSelected = mSelected;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotEnable(bool e)
{
    mEnabled = e;
    mActivateLbl->setEnabled( e );
    mWaitEdit->setEnabled( e );
    mLockCheckBox->setEnabled( e );
    mLockLbl->setEnabled( e && mLock );
    mWaitLockEdit->setEnabled( e && mLock );
    mChanged = true;
    emit changed(true);
}


//---------------------------------------------------------------------------
//
void KScreenSaver::slotScreenSaver(Q3ListViewItem *item)
{
    if (!item)
      return;

    int i = 0, indx = -1;
	Q_FOREACH( SaverConfig* saver , mSaverList ){
        if ( item->parent() )
        {
            if (  item->parent()->text( 0 ) == saver->category() && saver->name() == item->text (0))
            {
                indx = i;
                break;
            }
        }
        else
        {
            if (  saver->name() == item->text (0) )
            {
                indx = i;
                break;
            }
        }
		i++;
    }
    if (indx == -1) {
        mSelected = -1;
        return;
    }

    bool bChanged = (indx != mSelected);

    if (!mSetupProc->isRunning())
        mSetupBt->setEnabled(!mSaverList.at(indx)->setup().isEmpty());
    mTestBt->setEnabled(true);
    mSaver = mSaverList.at(indx)->file();

    mSelected = indx;
    setMonitor();
    if (bChanged)
    {
       mChanged = true;
       emit changed(true);
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotSetup()
{
    if ( mSelected < 0 )
    return;

    if (mSetupProc->isRunning())
    return;

    mSetupProc->clearArguments();

    QString saver = mSaverList.at(mSelected)->setup();
    if( saver.isEmpty())
        return;
    QTextStream ts(&saver, QIODevice::ReadOnly);

    QString word;
    ts >> word;
    bool kxsconfig = word == "kxsconfig";
    QString path = findExe(word);

    if (!path.isEmpty())
    {
        (*mSetupProc) << path;

        // Add caption and icon to about dialog
        if (!kxsconfig) {
            word = "-caption";
            (*mSetupProc) << word;
            word = mSaverList.at(mSelected)->name();
            (*mSetupProc) << word;
            word = "-icon";
            (*mSetupProc) << word;
            word = "kscreensaver";
            (*mSetupProc) << word;
        }

        while (!ts.atEnd())
        {
            ts >> word;
            (*mSetupProc) << word;
        }

        // Pass translated name to kxsconfig
        if (kxsconfig) {
          word = mSaverList.at(mSelected)->name();
          (*mSetupProc) << word;
        }

        mSetupBt->setEnabled( false );
        kapp->flush();

        mSetupProc->start();
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotAdvanced()
{
   KScreenSaverAdvancedDialog dlg( window() );
   if ( dlg.exec() ) {
       mChanged = true;
       emit changed(true);
  }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotTest()
{
    if ( mSelected == -1 )
        return;

    if (!mTestProc) {
        mTestProc = new K3Process;
    } else {
        mPreviewProc->kill();
        mPreviewProc->wait();
        mTestProc->clearArguments();
    }

    if (!mTestWin)
    {
        mTestWin = new TestWin();
        mTestWin->setAttribute(Qt::WA_NoSystemBackground, true);
        mTestWin->setAttribute(Qt::WA_PaintOnScreen, true);
        mTestWin->setGeometry(qApp->desktop()->geometry());
    }

    mTestWin->show();
    mTestWin->raise();
    mTestWin->setFocus();
	// So that hacks can XSelectInput ButtonPressMask
	XSelectInput(QX11Info::display(), mTestWin->winId(), widgetEventMask );

	grabMouse();
	grabKeyboard();

    mTestBt->setEnabled( false );

    QString saver = mSaverList.at(mSelected)->saver();
    QHash<QChar, QString> keyMap;
    keyMap.insert('w', QString::number(mTestWin->winId()));
    *mTestProc << KShell::splitArgs(KMacroExpander::expandMacrosShellQuote(saver, keyMap));

    mTestProc->start(K3Process::NotifyOnExit);

    mTesting = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotStopTest()
{
    if (mTestProc->isRunning()) {
        mTestProc->kill();
        mTestProc->wait();
    }
    releaseMouse();
    releaseKeyboard();
    mTestWin->hide();
    mTestBt->setEnabled(true);
    mPrevSelected = -1;
    setMonitor();
    mTesting = false;
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
    mLockLbl->setEnabled( l );
    mWaitLockEdit->setEnabled( l );
    mChanged = true;
    emit changed(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotSetupDone(K3Process *)
{
    mPrevSelected = -1;  // see ugly hack in slotPreviewExited()
    setMonitor();
    mSetupBt->setEnabled( true );
    emit changed(true);
}

#include "scrnsave.moc"
