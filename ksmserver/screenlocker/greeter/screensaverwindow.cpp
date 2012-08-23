/*
 *   Copyright 2012 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "screensaverwindow.h"
#include "kscreensaversettings.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>

#include <KDebug>
#include <kmacroexpander.h>
#include <kshell.h>
#include <KService>
#include <KServiceTypeTrader>
#include <KAuthorized>
#include <KDesktopFile>
#include <KStandardDirs>

namespace ScreenLocker
{

ScreenSaverWindow::ScreenSaverWindow(QWidget *parent)
    : QWidget(parent),
      m_startMousePos(-1, -1),
      m_forbidden(false),
      m_openGLVisual(false)
{
    m_reactivateTimer = new QTimer(this);
    m_reactivateTimer->setSingleShot(true);
    connect(m_reactivateTimer, SIGNAL(timeout()), this, SLOT(show()));

    setMouseTracking(true);
    m_saver = KScreenSaverSettings::saver();
    readSaver();
}

ScreenSaverWindow::~ScreenSaverWindow()
{
}

//---------------------------------------------------------------------------
//
// Read the command line needed to run the screensaver given a .desktop file.
//
void ScreenSaverWindow::readSaver()
{
    if (!m_saver.isEmpty())
    {
        QString entryName = m_saver;
        if( entryName.endsWith( QLatin1String( ".desktop" ) ))
            entryName = entryName.left( entryName.length() - 8 ); // strip it
        const KService::List offers = KServiceTypeTrader::self()->query( QLatin1String( "ScreenSaver" ),
            QLatin1String( "DesktopEntryName == '" ) + entryName.toLower() + QLatin1Char( '\'' ) );
        if( offers.isEmpty() )
        {
            kDebug(1204) << "Cannot find screesaver: " << m_saver;
            return;
        }
        const QString file = KStandardDirs::locate("services", offers.first()->entryPath());

        const bool opengl = KAuthorized::authorizeKAction(QLatin1String( "opengl_screensavers" ));
        const bool manipulatescreen = KAuthorized::authorizeKAction(QLatin1String( "manipulatescreen_screensavers" ));
        KDesktopFile config( file );
        KConfigGroup desktopGroup = config.desktopGroup();
        foreach (const QString &type, desktopGroup.readEntry("X-KDE-Type").split(QLatin1Char(';'))) {
            if (type == QLatin1String("ManipulateScreen")) {
                if (!manipulatescreen) {
                    kDebug(1204) << "Screensaver is type ManipulateScreen and ManipulateScreen is forbidden";
                    m_forbidden = true;
                }
            } else if (type == QLatin1String("OpenGL")) {
                m_openGLVisual = true;
                if (!opengl) {
                    kDebug(1204) << "Screensaver is type OpenGL and OpenGL is forbidden";
                    m_forbidden = true;
                }
            }
        }

        kDebug(1204) << "m_forbidden: " << (m_forbidden ? "true" : "false");

        if (config.hasActionGroup(QLatin1String( "InWindow" )))
        {
            m_saverExec = config.actionGroup(QLatin1String( "InWindow" )).readPathEntry("Exec", QString());
        }
    }
}

void ScreenSaverWindow::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    m_startMousePos = QPoint(-1, -1);
    //reappear in one minute
    m_reactivateTimer->start(1000 * 60);
    hide();
}


void ScreenSaverWindow::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event)

    hide();
}

void ScreenSaverWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_startMousePos == QPoint(-1, -1)) {
        m_startMousePos = event->globalPos();
    }
    if ((event->globalPos() - m_startMousePos).manhattanLength() > QApplication::startDragDistance()) {
        m_startMousePos = QPoint(-1, -1);
        hide();
        //reappear in one minute
        m_reactivateTimer->start(1000 * 60);
    }
}

void ScreenSaverWindow::showEvent(QShowEvent *event)
{
    m_reactivateTimer->stop();
    startXScreenSaver();
}

void ScreenSaverWindow::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.fillRect(event->rect(), Qt::black);
    p.end();
}

//---------------------------------------------------------------------------
//


bool ScreenSaverWindow::startXScreenSaver()
{
    //QString m_saverExec("kannasaver.kss --window-id=%w");
    kDebug(1204) << "Starting hack:" << m_saverExec;

    if (m_saverExec.isEmpty() || m_forbidden)
    {
        return false;
    }

    QHash<QChar, QString> keyMap;
    keyMap.insert(QLatin1Char( 'w' ), QString::number(winId()));
    m_ScreenSaverProcess << KShell::splitArgs(KMacroExpander::expandMacrosShellQuote(m_saverExec, keyMap));

    m_ScreenSaverProcess.start();
    if (m_ScreenSaverProcess.waitForStarted())
    {
#ifdef HAVE_SETPRIORITY
        setpriority(PRIO_PROCESS, m_ScreenSaverProcess.pid(), mPriority);
#endif
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------
//
void ScreenSaverWindow::stopXScreenSaver()
{
    if (m_ScreenSaverProcess.state() != QProcess::NotRunning)
    {
        m_ScreenSaverProcess.terminate();
        if (!m_ScreenSaverProcess.waitForFinished(10000))
        {
            m_ScreenSaverProcess.kill();
        }
    }
}

} // end namespace
#include "screensaverwindow.moc"
