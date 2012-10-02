/*****************************************************************
ksmserver - the KDE session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>

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

#ifndef SERVER_H
#define SERVER_H

// needed to avoid clash with INT8 defined in X11/Xmd.h on solaris
#define QT_CLEAN_NAMESPACE 1
#include <QStringList>
#include <QObject>

#include <kapplication.h>
#include <kworkspace/kworkspace.h>
#include <kmessagebox.h>
#include <QTimer>
#include <QTime>
#include <QMap>

#define INT32 QINT32
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/ICE/ICElib.h>
extern "C" {
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>
#include <X11/SM/SM.h>
#include <X11/SM/SMlib.h>
}

#include <fixx11h.h>

#define SESSION_PREVIOUS_LOGOUT "saved at previous logout"
#define SESSION_BY_USER  "saved by user"

class KProcess;

class KSMListener;
class KSMConnection;
class KSMClient;

class OrgKdeKLauncherInterface;
class QDBusInterface;

enum SMType { SM_ERROR, SM_WMCOMMAND, SM_WMSAVEYOURSELF };
struct SMData
    {
    SMType type;
    QStringList wmCommand;
    QString wmClientMachine;
    QString wmclass1, wmclass2;
    };
typedef QMap<WId,SMData> WindowMap;

class KSMServer : public QObject
{
Q_OBJECT
public:
    KSMServer( const QString& windowManager, bool only_local, bool lockscreen = false );
    ~KSMServer();

    static KSMServer* self();

    void* watchConnection( IceConn iceConn );
    void removeConnection( KSMConnection* conn );

    KSMClient* newClient( SmsConn );
    void  deleteClient( KSMClient* client );

    // callbacks
    void saveYourselfDone( KSMClient* client, bool success );
    void interactRequest( KSMClient* client, int dialogType );
    void interactDone( KSMClient* client, bool cancelShutdown );
    void phase2Request( KSMClient* client );

    // error handling
    void ioError( IceConn iceConn );

    // notification
    void clientSetProgram( KSMClient* client );
    void clientRegistered( const char* previousId );

    // public API
    void restoreSession( const QString &sessionName );
    void startDefaultSession();
    void shutdown( KWorkSpace::ShutdownConfirm confirm,
                   KWorkSpace::ShutdownType sdtype,
                   KWorkSpace::ShutdownMode sdmode );

    virtual void suspendStartup( const QString &app );
    virtual void resumeStartup( const QString &app );

    void launchWM( const QList< QStringList >& wmStartCommands );

public Q_SLOTS:
    void cleanUp();

private Q_SLOTS:
    void newConnection( int socket );
    void processData( int socket );

    void protectionTimeout();
    void timeoutQuit();
    void timeoutWMQuit();
    void kcmPhase1Timeout();
    void kcmPhase2Timeout();
    void pendingShutdownTimeout();
    void logoutSoundTimeout();

    void autoStart0();
    void autoStart1();
    void autoStart2();
    void tryRestoreNext();
    void startupSuspendTimeout();
    void wmProcessChange();
    void logoutSoundFinished();
    void autoStart0Done();
    void autoStart1Done();
    void autoStart2Done();
    void kcmPhase1Done();
    void kcmPhase2Done();

    void defaultLogout();
    void logoutWithoutConfirmation();
    void haltWithoutConfirmation();
    void rebootWithoutConfirmation();

private:
    void handlePendingInteractions();
    void completeShutdownOrCheckpoint();
    void startKilling();
    void startKillingSubSession();
    void performStandardKilling();
    void completeKilling();
    void completeKillingSubSession();
    void killWM();
    void signalSubSessionClosed();
    void completeKillingWM();
    void cancelShutdown( KSMClient* c );
    void killingCompleted();
    void createLogoutEffectWidget();

    void runUserAutostart();

    void discardSession();
    void storeSession();

    void startProtection();
    void endProtection();

    KProcess* startApplication( const QStringList& command,
        const QString& clientMachine = QString(),
        const QString& userId = QString(),
        bool wm = false );
    void executeCommand( const QStringList& command );

    bool isWM( const KSMClient* client ) const;
    bool isWM( const QString& command ) const;
    void selectWm( const QString& kdewm );
    bool defaultSession() const; // empty session
    void setupXIOErrorHandler();

    void performLegacySessionSave();
    void storeLegacySession( KConfig* config );
    void restoreLegacySession( KConfig* config );
    void restoreLegacySessionInternal( KConfigGroup* config, char sep = ',' );
    QStringList windowWmCommand(WId w);
    QString windowWmClientMachine(WId w);
    WId windowWmClientLeader(WId w);
    QByteArray windowSessionId(WId w, WId leader);

    bool checkStartupSuspend();
    void finishStartup();
    void resumeStartupInternal();
    void setupShortcuts();

    // public dcop interface

 public Q_SLOTS: //public dcop interface
    void logout( int, int, int );
    bool canShutdown();
    QString currentSession();
    void saveCurrentSession();
    void saveCurrentSessionAs( const QString & );
    QStringList sessionList();
    void wmChanged();
    void saveSubSession( const QString &name, QStringList saveAndClose,
                       QStringList saveOnly = QStringList() );
    void restoreSubSession( const QString &name );

 Q_SIGNALS:
    void subSessionClosed();
    void subSessionCloseCanceled();
    void subSessionOpened();

 private:
    QList<KSMListener*> listener;
    QList<KSMClient*> clients;

    enum State
        {
        Idle,
        LaunchingWM, AutoStart0, KcmInitPhase1, AutoStart1, Restoring, FinishingStartup, // startup
        Shutdown, Checkpoint, Killing, KillingWM, WaitingForKNotify, // shutdown
        ClosingSubSession, KillingSubSession, RestoringSubSession
        };
    State state;
    bool dialogActive;
    bool saveSession;
    int wmPhase1WaitingCount;
    int saveType;
    QMap< QString, int > startupSuspendCount;

    KWorkSpace::ShutdownType shutdownType;
    KWorkSpace::ShutdownMode shutdownMode;
    QString bootOption;

    bool clean;
    KSMClient* clientInteracting;
    QString wm;
    QStringList wmCommands;
    KProcess* wmProcess;
    QString sessionGroup;
    QString sessionName;
    QTimer protectionTimer;
    QTimer restoreTimer;
    QString xonCommand;
    QTimer startupSuspendTimeoutTimer;
    bool waitAutoStart2;
    bool waitKcmInit2;
    QTimer pendingShutdown;
    QWidget* logoutEffectWidget;
    KWorkSpace::ShutdownConfirm pendingShutdown_confirm;
    KWorkSpace::ShutdownType pendingShutdown_sdtype;
    KWorkSpace::ShutdownMode pendingShutdown_sdmode;

    // ksplash interface
    void upAndRunning( const QString& msg );

    // sequential startup
    int appsToStart;
    int lastAppStarted;
    QString lastIdStarted;

    QStringList excludeApps;

    WindowMap legacyWindows;

    OrgKdeKLauncherInterface* klauncherSignals;
    QDBusInterface* kcminitSignals;

    int inhibitCookie;

    //subSession stuff
    QList<KSMClient*> clientsToKill;
    QList<KSMClient*> clientsToSave;
};

#endif
