//-----------------------------------------------------------------------------
//
// KDE Display screen saver setup module
//
// Copyright (c)  Martin R. Jones 1996,1999
//
// Converted to a kcc module by Matthias Hoelzer 1997
//

#include <kprocess.h>

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <qhgroupbox.h> 
#include <qvgroupbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qlayout.h>
#include <qtextstream.h>
#include <ksimpleconfig.h>
#include <kapp.h>
#include <knuminput.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kwm.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <kiconloader.h>
#include <kcontrol.h>

#include "kcolordlg.h"
#include "scrnsave.h"
#include <klocale.h>
#include <kconfig.h>
#include "scrnsave.moc"

#define CORNER_SIZE		15

//===========================================================================
//
//
SaverConfig::SaverConfig()
{
}

bool SaverConfig::read(QString file)
{
    KSimpleConfig config(file, true);
    
    config.setDesktopGroup();
    mExec = config.readEntry("Exec");
    mSetup = config.readEntry("Exec-setup");
    mSaver = config.readEntry("Exec-kss");
    mName = config.readEntry("Name");
    
    int indx = file.findRev('/');
    if (indx >= 0) {
        mFile = file.mid(indx+1);
    }
    
    return !mSaver.isEmpty();
}

//===========================================================================
//
int SaverList::compareItems(QCollection::Item item1, QCollection::Item item2)
{
    SaverConfig *s1 = (SaverConfig *)item1;
    SaverConfig *s2 = (SaverConfig *)item2;

    return s1->name().compare(s2->name());
}

//===========================================================================
//
TestWin::TestWin()
    : QWidget(0, 0, WStyle_Customize | WStyle_NoBorder)
{
    setFocusPolicy(StrongFocus);
}

void TestWin::mousePressEvent(QMouseEvent *)
{
    emit stopTest();
}

void TestWin::keyPressEvent(QKeyEvent *)
{
    emit stopTest();
}


//===========================================================================
//
KScreenSaver::KScreenSaver( QWidget *parent, Mode m )
	: KDisplayModule( parent, m )
{
    mSetupProc = 0;
    mPreviewProc = 0;
    mTestWin = 0;
    mTestProc = 0;
    mPrevSelected = -1;
    mMonitor = 0;
     
    if (mode() == Init) 
        return;
    
    // Add non-KDE path
    KGlobal::dirs()->addResourceType("scrsav",
                                     KGlobal::dirs()->kde_default("apps") +
                                     "apps/ScreenSavers/");
    
    // Add KDE specific screensaver path
    KGlobal::dirs()->addResourceType("scrsav",
                                     KGlobal::dirs()->kde_default("apps") +
                                     "ScreenSavers/");
    
    readSettings();
	
    setName( i18n("Screen Saver").ascii() );
    
    mSetupProc = new KProcess;
    connect(mSetupProc, SIGNAL(processExited(KProcess *)),
            this, SLOT(slotSetupDone(KProcess *)));
    
    mPreviewProc = new KProcess;
    connect(mPreviewProc, SIGNAL(processExited(KProcess *)),
            this, SLOT(slotPreviewExited(KProcess *)));
    
    findSavers();
    
    QGridLayout *topLayout = new QGridLayout( this, 3, 2, 10 );
    
    topLayout->setRowStretch(0,0);
    topLayout->setRowStretch(1,70);
    topLayout->setRowStretch(2,30);

    mMonitorLabel = new QLabel( this );
    mMonitorLabel->setAlignment( AlignCenter );
    mMonitorLabel->setPixmap( QPixmap(locate("data",
					     "kcontrol/pics/monitor.png"))); 
    
    topLayout->addMultiCellWidget( mMonitorLabel, 0, 0, 0, 1 );
    
    QGroupBox *group = new QGroupBox(i18n("Screen Saver"), this );
	
    topLayout->addMultiCellWidget( group, 1, 2, 0, 0 );
	
    QBoxLayout *groupLayout = new QVBoxLayout( group, 10 );
    groupLayout->addSpacing(10);		
    
    mSaverListBox = new QListBox( group );
    mSaverListBox->insertItem( i18n("No screensaver"), 0 );
    mSaverListBox->setCurrentItem( 0 );
    
    SaverConfig *saver;
    mSelected = 0;
    for (saver = mSaverList.first(); saver != 0; saver = mSaverList.next())
    {
        mSaverListBox->insertItem(saver->name());
        if (saver->file() == mSaver && mEnabled) 
            mSelected = mSaverListBox->count()-1;
    }
    if (mSelected == 0) 
        mEnabled = false;
    
    mSaverListBox->setCurrentItem(mSelected);
    mSaverListBox->setTopItem(mSaverListBox->currentItem());
    mSelected = mSaverListBox->currentItem();
    connect( mSaverListBox, SIGNAL( highlighted( int ) ),
             this, SLOT( slotScreenSaver( int ) ) );
	
    groupLayout->addWidget( mSaverListBox, 10 );

    QBoxLayout* hlay = new QHBoxLayout(groupLayout, 10);
    
    mSetupBt = new QPushButton(  i18n("&Setup ..."), group );
    connect( mSetupBt, SIGNAL( clicked() ), SLOT( slotSetup() ) );
    mSetupBt->setEnabled(mEnabled &&
                         !mSaverList.at(mSelected-1)->setup().isEmpty());

    hlay->addWidget( mSetupBt );
    
    mTestBt = new QPushButton(  i18n("&Test"), group );
    connect( mTestBt, SIGNAL( clicked() ), SLOT( slotTest() ) );
    mTestBt->setEnabled(mEnabled);
    
    hlay->addWidget( mTestBt );

    QBoxLayout *stackLayout = new QVBoxLayout;
    topLayout->addLayout(stackLayout, 1, 1);

    group = new QGroupBox(  i18n("Settings"), this );
    stackLayout->addWidget( group );

    groupLayout = new QVBoxLayout( group, 10 );
    groupLayout->addSpacing(10);

    mWaitEdit = new KIntNumInput(i18n("&Wait for"), 1, 120, 1, mTimeout/60,
                                 i18n("min"), 10, false, group);
    mWaitEdit->setLabelAlignment(AlignCenter);
    connect( mWaitEdit, SIGNAL( valueChanged(int) ),
             SLOT( slotTimeoutChanged(int) ) );
    groupLayout->addWidget(mWaitEdit);

    mLockCheckBox = new QCheckBox( i18n("&Require password"), group );
    mLockCheckBox->setChecked( mLock );
    connect( mLockCheckBox, SIGNAL( toggled( bool ) ), 
	     this, SLOT( slotLock( bool ) ) );
    groupLayout->addWidget(mLockCheckBox);

    mStarsCheckBox = new QCheckBox( i18n("Show &password as stars"), group );
    mStarsCheckBox->setChecked(mPasswordStars);
    connect( mStarsCheckBox, SIGNAL( toggled( bool ) ), 
	     this, SLOT( slotStars( bool ) ) );
    groupLayout->addWidget(mStarsCheckBox);

   
    stackLayout = new QHBoxLayout;
    topLayout->addLayout(stackLayout, 2, 1); 

    group = new QGroupBox(  i18n("Priority"), this );
    stackLayout->addWidget( group );
    
    groupLayout = new QHBoxLayout( group, 15, 5 );
    
    QLabel* label = new QLabel( mPrioritySlider, i18n("High"), group );
    groupLayout->addWidget(label);

    mPrioritySlider = new QSlider(QSlider::Horizontal, group);
    mPrioritySlider->setRange(0, 19);
    mPrioritySlider->setSteps(1, 5);
    mPrioritySlider->setValue(mPriority);
    connect(mPrioritySlider, SIGNAL( valueChanged(int)),
	    SLOT(slotPriorityChanged(int)));
    groupLayout->addWidget(mPrioritySlider);

#ifndef HAVE_SETPRIORITY
    label->setEnabled(false);
    mPrioritySlider->setEnabled(false);
#endif
    
    label = new QLabel( i18n("Low"), group );
    groupLayout->addWidget(label);
    
#ifndef HAVE_SETPRIORITY
    label->setEnabled(false);
#endif

    topLayout->activate();

    setMonitor();
}

//---------------------------------------------------------------------------
//
void KScreenSaver::resizeEvent( QResizeEvent * )
{
  
  if (mMonitor)
    {
      mMonitor->setGeometry( (mMonitorLabel->width()-200)/2+20,
			     (mMonitorLabel->height()-160)/2+10, 157, 111 );
    }
}

//---------------------------------------------------------------------------
//
KScreenSaver::~KScreenSaver()
{
    if (mPreviewProc)
    {
        if (mPreviewProc->isRunning())
        {
            int pid = mPreviewProc->getPid();  
            mPreviewProc->kill( );
            waitpid(pid, (int *) 0,0);
        }
        delete mPreviewProc;
    }

    delete mTestProc;
    delete mSetupProc;
    delete mTestWin;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::readSettings( int )
{
	KConfig *config = new KConfig( "kdesktoprc");
	config->setGroup( "ScreenSaver" );
    
	mEnabled = config->readBoolEntry("Enabled", false);
	mLock = config->readBoolEntry("Lock", false);
	mTimeout = config->readNumEntry("Timeout", 300);
	mPriority = config->readNumEntry("Priority", 0);
	mPasswordStars = config->readBoolEntry("PasswordAsStars", true);
	mSaver = config->readEntry("Saver");
    
	if (mPriority < 0) mPriority = 0;
	if (mPriority > 19) mPriority = 19;
    if (mTimeout < 60) mTimeout = 60;

    mChanged = false;

	delete config;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::updateValues()
{
	mWaitEdit->setValue(mTimeout/60);
	mLockCheckBox->setChecked(mLock);
	mStarsCheckBox->setChecked(mPasswordStars);
	mPrioritySlider->setValue(mPriority);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::defaultSettings()
{
	slotScreenSaver( 0 );
	mSaverListBox->setCurrentItem( 0 );
	mSaverListBox->centerCurrentItem();
	slotTimeoutChanged( 1 );
	slotPriorityChanged( 0 );
	slotLock( false );
	slotStars( true );
	updateValues();
}

//---------------------------------------------------------------------------
//
void KScreenSaver::writeSettings()
{
	if ( !mChanged )
		return;

	KConfig *config = new KConfig( "kdesktoprc");
	config->setGroup( "ScreenSaver" );

	config->writeEntry("Enabled", mEnabled);
	config->writeEntry("Timeout", mTimeout);
	config->writeEntry("Lock", mLock);
	config->writeEntry("Priority", mPriority);
	config->writeEntry("PasswordAsStars", mPasswordStars);
    config->writeEntry("Saver", mSaver);
	config->sync();
    delete config;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::findSavers()
{
	QStringList saverFileList = KGlobal::dirs()->findAllResources("scrsav",
                                                    "*.desktop", false, true);
    
	QStringList::Iterator it = saverFileList.begin();
	for ( ; it != saverFileList.end(); ++it ) {
        SaverConfig *saver = new SaverConfig;
        if (saver->read(*it)) {
            mSaverList.append(saver);
        }
        else 
            delete saver;
	}

    mSaverList.sort();
}

//---------------------------------------------------------------------------
//
void KScreenSaver::setMonitor()
{
	if (mPreviewProc->isRunning())
	    // CC: this will automatically cause a "slotPreviewExited"
	    // when the viewer exits
	    mPreviewProc->kill( );
	else
	    slotPreviewExited(mPreviewProc);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotPreviewExited(KProcess *)
{
    // Ugly hack to prevent continual respawning of savers that crash
    if (mSelected == mPrevSelected)
        return;

    // Some xscreensaver hacks do something nasty to the window that
    // requires a new one to be created (or proper investigation of the
    // problem).
    if (mMonitor)
        delete mMonitor;

	mMonitor = new KSSMonitor(mMonitorLabel);
	mMonitor->setBackgroundColor(black);
	mMonitor->setGeometry((mMonitorLabel->width()-200)/2+20,
                          (mMonitorLabel->height()-160)/2+10, 157, 111);
    mMonitor->show();

    if (mEnabled)
    {
        mPreviewProc->clearArguments();
        
        QString saver = mSaverList.at(mSelected-1)->saver();
        QTextStream ts(&saver, IO_ReadOnly);
        
        QString word;
        ts >> word;
        QString path = KStandardDirs::findExe(word);

        if (!path.isEmpty())
        {
            (*mPreviewProc) << path;

            while (!ts.atEnd())
            {
                ts >> word;
                if (word == "%w")
                {
                    word = word.setNum(mMonitor->winId());
                }
                (*mPreviewProc) << word;
            }

            mPreviewProc->start();
        }
    }

    mPrevSelected = mSelected;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotScreenSaver(int indx)
{
	if ( indx == 0 ) {
		mSetupBt->setEnabled( false );
		mTestBt->setEnabled( false );
		mEnabled = false;
	}
	else {
		if (!mSetupProc->isRunning())
			mSetupBt->setEnabled(!mSaverList.at(indx - 1)->setup().isEmpty());
		mTestBt->setEnabled(true);
        mSaver = mSaverList.at(indx - 1)->file();
		mEnabled = true;
	}
    
    mSelected = indx;
    
	setMonitor();

	mChanged = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotSetup()
{
	if ( !mEnabled )
	    return;

	if (mSetupProc->isRunning())
	    return;
	
    mSetupProc->clearArguments();

    QString saver = mSaverList.at(mSelected-1)->setup();
    QTextStream ts(&saver, IO_ReadOnly);

    QString word;
    ts >> word;
    QString path = KStandardDirs::findExe(word);

    if (!path.isEmpty())
    {
        (*mSetupProc) << path;

        while (!ts.atEnd())
        {
            ts >> word;
            (*mSetupProc) << word;
        }

        mSetupBt->setEnabled( FALSE );
        kapp->flushX();

        mSetupProc->start();
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotTest()
{
    if (!mTestProc) {
	    mTestProc = new KProcess;
    }

	mTestProc->clearArguments();
    QString saver = mSaverList.at(mSelected-1)->saver();
    QTextStream ts(&saver, IO_ReadOnly);

    QString word;
    ts >> word;
    QString path = KStandardDirs::findExe(word);

    if (!path.isEmpty())
    {
        (*mTestProc) << path;

        if (!mTestWin)
        {
            mTestWin = new TestWin();
            mTestWin->setBackgroundMode(QWidget::NoBackground);
            mTestWin->setGeometry(0, 0, kapp->desktop()->width(),
                                    kapp->desktop()->height());
            connect(mTestWin, SIGNAL(stopTest()), SLOT(slotStopTest()));
        }

        mTestWin->show();
        mTestWin->raise();
        mTestWin->setFocus();
        mTestWin->grabKeyboard();

        mTestBt->setEnabled( FALSE );

        while (!ts.atEnd())
        {
            ts >> word;
            if (word == "%w")
            {
                word = word.setNum(mTestWin->winId());
            }
            (*mTestProc) << word;
        }

        mTestProc->start(KProcess::DontCare);
    }
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotStopTest()
{
    if (mTestProc->isRunning()) {
        mTestProc->kill();
    }
    mTestWin->releaseKeyboard();
    mTestWin->hide();
	mTestBt->setEnabled(true);
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotTimeoutChanged(int to )
{
	mTimeout = to * 60;
	mChanged = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotLock( bool l )
{
	mLock = l;
	mChanged = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotStars( bool s )
{
	mPasswordStars = s;
	mChanged = true;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotPriorityChanged( int val )
{
	if ( val != mPriority )
		mChanged = true;
	
	mPriority = val;

	if ( mPriority > 19 )
		mPriority = 19;
}

//---------------------------------------------------------------------------
//
void KScreenSaver::slotSetupDone(KProcess *)
{
    mPrevSelected = -1;  // see ugly hack in slotPreviewExited()
	setMonitor();
	mSetupBt->setEnabled( true );
}

//---------------------------------------------------------------------------
//
void KScreenSaver::applySettings()
{
    if (mChanged)
    {
        writeSettings();
        KWM::sendKWMCommand("kss_reconfigure");
        mChanged = false;
    }
}

