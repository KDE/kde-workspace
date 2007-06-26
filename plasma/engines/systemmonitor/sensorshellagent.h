/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KSG_SENSORSHELLAGENT_H
#define KSG_SENSORSHELLAGENT_H

#include <QObject>
#include <QPointer>

#include "sensoragent.h"

class QString;

class K3Process;

namespace KSGRD {

class SensorManager;

/**
  The SensorShellAgent starts a ksysguardd process and handles the
  asynchronous communication. It keeps a list of pending requests
  that have not been answered yet by ksysguard. The current
  implementation only allowes one pending requests. Incoming requests
  are queued in an input FIFO.
 */
class SensorShellAgent : public SensorAgent
{
  Q_OBJECT

  public:
    explicit SensorShellAgent( SensorManager *sm );
    ~SensorShellAgent();

    bool start( const QString &host, const QString &shell,
                const QString &command = "", int port = -1 );

    void hostInfo( QString &shell, QString &command, int &port) const;

  private Q_SLOTS:
    void msgSent( K3Process* );
    void msgRcvd( K3Process*, char *buffer, int buflen );
    void errMsgRcvd( K3Process*, char *buffer, int buflen );
    void daemonExited( K3Process* );

  private:
    bool writeMsg( const char *msg, int len );
    bool txReady();
    int mRetryCount;
    QPointer<K3Process> mDaemon;
    QString mShell;
    QString mCommand;
};

}
	
#endif
