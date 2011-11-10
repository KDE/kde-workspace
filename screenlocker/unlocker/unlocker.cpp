/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 1999 Martin R. Jones <mjones@kde.org>
Copyright (C) 2003 Oswald Buddenhagen <ossi@kde.org>
Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

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
#include "unlocker.h"

// Qt
#include <QtCore/QFile>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>
#include <QtGui/QGraphicsProxyWidget>
// KDE
#include <KDE/KAuthorized>
#include <KDE/KDebug>
#include <KDE/KLibrary>
#include <KDE/KLocale>
#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>
#include <KDE/KStandardDirs>
// workspace
#include <kcheckpass-enums.h>
#include <kdisplaymanager.h>
#include "kscreensaversettings.h"

// kscreenlocker stuff
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

namespace ScreenLocker
{

UnlockerItem::UnlockerItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_proxy(new QGraphicsProxyWidget(this))
{
}

UnlockerItem::~UnlockerItem()
{
}

QGraphicsProxyWidget *UnlockerItem::proxy()
{
    return m_proxy;
}

// KeyboardItem

KeyboardItem::KeyboardItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_widget(new QWidget())
    , m_proxy(new QGraphicsProxyWidget(this))
{
    m_widget->setAttribute(Qt::WA_TranslucentBackground);
    KPluginFactory *kxkbFactory = KPluginLoader(QLatin1String("keyboard_layout_widget")).factory();
    if (kxkbFactory) {
        kxkbFactory->create<QWidget>(m_widget);
    } else {
        kDebug() << "can't load keyboard layout widget library";
    }
    m_proxy->setWidget(m_widget);
}

KeyboardItem::~KeyboardItem()
{
}

UserSessionsModel::UserSessionsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    init();
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole] = "session";
    roles[Qt::UserRole + 1] = "location";
    roles[Qt::UserRole + 2] = "vt";
    setRoleNames(roles);
}

UserSessionsModel::~UserSessionsModel()
{
}

QVariant UserSessionsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.row() < 0 || index.row() >= m_model.size()) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
    case Qt::UserRole:
        return m_model[index.row()].m_session;
    case (Qt::UserRole + 1):
        return m_model[index.row()].m_location;
    case (Qt::UserRole + 2):
        return m_model[index.row()].m_vt;
    default:
        return QVariant();
    }
}

QVariant UserSessionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
    case Qt::UserRole:
        return i18n("Session");
    case Qt::UserRole + 1:
        return i18n("Location");
    default:
        return QAbstractItemModel::headerData(section, orientation, role);
    }
}

int UserSessionsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_model.count();
}

Qt::ItemFlags UserSessionsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    if (!m_model[index.row()].m_enabled) {
        return QAbstractItemModel::flags(index) & ~Qt::ItemIsEnabled;
    }
    return QAbstractItemModel::flags(index);
}

void UserSessionsModel::init()
{
    beginResetModel();
    m_model.clear();
    KDisplayManager dm;

    SessList sess;
    if (dm.localSessions(sess)) {

        QString user, loc;
        for (SessList::ConstIterator it = sess.constBegin(); it != sess.constEnd(); ++it) {
            KDisplayManager::sess2Str2(*it, user, loc);
            m_model << UserSessionItem(user, loc, (*it).vt, (*it).vt);
        }
    }
    endResetModel();
}

Unlocker::Unlocker(QObject *parent)
    : QObject(parent)
    , m_greeterWidget(new QWidget())
    , m_greet(0)
    , m_valid(false)
    , m_pid(0)
    , m_fd(0)
    , m_notifier(NULL)
    , m_sessionModel(new UserSessionsModel(this))
    , m_failedLock(false)
{
    m_pluginHandle.library = 0;
    initialize();
    m_valid = loadGreetPlugin();
    if (m_valid) {

        m_greet = m_pluginHandle.info->create(this, m_greeterWidget, QString(),
                                        KGreeterPlugin::Authenticate,
                                        KGreeterPlugin::ExUnlock);
    }
}

Unlocker::~Unlocker()
{
    if (m_pluginHandle.library) {
        if (m_pluginHandle.info->done) {
            m_pluginHandle.info->done();
        }
        m_pluginHandle.library->unload();
    }
}

void Unlocker::initialize()
{
    m_plugins = KScreenSaverSettings::pluginsUnlock();
    if (m_plugins.isEmpty()) {
        m_plugins << QLatin1String( "classic" ) << QLatin1String( "generic" );
    }
    m_pluginOptions = KScreenSaverSettings::pluginOptions();
}

// standard greeter stuff
// private static
QVariant Unlocker::getConf(void *ctx, const char *key, const QVariant &dflt)
{
    Unlocker *that = (Unlocker *)ctx;
    QString fkey = QLatin1String( key ) % QLatin1Char( '=' );
    for (QStringList::ConstIterator it = that->m_pluginOptions.constBegin();
         it != that->m_pluginOptions.constEnd(); ++it)
        if ((*it).startsWith( fkey ))
            return (*it).mid( fkey.length() );
    return dflt;
}

bool Unlocker::loadGreetPlugin()
{
    if (m_pluginHandle.library) {
        //we were locked once before, so all the plugin loading's done already
        //FIXME should I be unloading the plugin on unlock instead?
        return true;
    }
    for (QStringList::ConstIterator it = m_plugins.constBegin(); it != m_plugins.constEnd(); ++it) {
        GreeterPluginHandle plugin;
        KLibrary *lib = new KLibrary( (*it)[0] == QLatin1Char( '/' ) ? *it : QLatin1String( "kgreet_" ) + *it );
        if (lib->fileName().isEmpty()) {
            kWarning(1212) << "GreeterPlugin " << *it << " does not exist" ;
            delete lib;
            continue;
        }
        if (!lib->load()) {
            kWarning(1212) << "Cannot load GreeterPlugin " << *it << " (" << lib->fileName() << ")" ;
            delete lib;
            continue;
        }
        plugin.library = lib;
        plugin.info = (KGreeterPluginInfo *)lib->resolveSymbol( "kgreeterplugin_info" );
        if (!plugin.info ) {
            kWarning(1212) << "GreeterPlugin " << *it << " (" << lib->fileName() << ") is no valid greet widget plugin" ;
            lib->unload();
            delete lib;
            continue;
        }
        if (plugin.info->method && !m_method.isEmpty() && m_method != QLatin1String(  plugin.info->method )) {
            kDebug(1212) << "GreeterPlugin " << *it << " (" << lib->fileName() << ") serves " << plugin.info->method << ", not " << m_method;
            lib->unload();
            delete lib;
            continue;
        }
        if (!plugin.info->init( m_method, getConf, this )) {
            kDebug(1212) << "GreeterPlugin " << *it << " (" << lib->fileName() << ") refuses to serve " << m_method;
            lib->unload();
            delete lib;
            continue;
        }
        kDebug(1212) << "GreeterPlugin " << *it << " (" << plugin.info->method << ", " << plugin.info->name << ") loaded";
        m_pluginHandle = plugin;
        return true;
    }
    return false;
}

void Unlocker::verify()
{
    if (m_failedLock) {
        // greeter blocked due to failed unlock attempt
        return;
    }
    m_greet->next();
}

void Unlocker::failedTimer()
{
    emit greeterReady();
    m_greet->revive();
    m_greet->start();
    m_failedLock = false;
}

bool Unlocker::isSwitchUserSupported() const
{
    return KDisplayManager().isSwitchable() && KAuthorized::authorizeKAction(QLatin1String("switch_user"));
}

bool Unlocker::isStartNewSessionSupported() const
{
    KDisplayManager dm;
    return dm.isSwitchable() && dm.numReserve() > 0 && KAuthorized::authorizeKAction(QLatin1String("start_new_session"));
}

void Unlocker::startNewSession()
{
    // verify that starting a new session is allowed
    if (!isStartNewSessionSupported()) {
        return;
    }

    KDisplayManager().startReserve();
}

void Unlocker::activateSession(int index)
{
    // verify that starting a new session is allowed
    if (!isSwitchUserSupported()) {
        return;
    }
    QModelIndex modelIndex(m_sessionModel->index(index));
    if (!modelIndex.isValid()) {
        return;
    }
    KDisplayManager().switchVT(m_sessionModel->data(modelIndex, Qt::UserRole + 2).toInt());
}

////// kckeckpass interface code

int Unlocker::Reader(void *buf, int count)
{
    int ret, rlen;

    for (rlen = 0; rlen < count; ) {
      dord:
        ret = ::read(m_fd, (void *)((char *)buf + rlen), count - rlen);
        if (ret < 0) {
            if (errno == EINTR)
                goto dord;
            if (errno == EAGAIN)
                break;
            return -1;
        }
        if (!ret)
            break;
        rlen += ret;
    }
    return rlen;
}

bool Unlocker::GRead(void *buf, int count)
{
    return Reader(buf, count) == count;
}

bool Unlocker::GWrite(const void *buf, int count)
{
    return ::write(m_fd, buf, count) == count;
}

bool Unlocker::GSendInt(int val)
{
    return GWrite(&val, sizeof(val));
}

bool Unlocker::GSendStr(const char *buf)
{
    int len = buf ? ::strlen (buf) + 1 : 0;
    return GWrite(&len, sizeof(len)) && GWrite (buf, len);
}

bool Unlocker::GSendArr(int len, const char *buf)
{
    return GWrite(&len, sizeof(len)) && GWrite (buf, len);
}

bool Unlocker::GRecvInt(int *val)
{
    return GRead(val, sizeof(*val));
}

bool Unlocker::GRecvArr(char **ret)
{
    int len;
    char *buf;

    if (!GRecvInt(&len))
        return false;
    if (!len) {
        *ret = 0;
        return true;
    }
    if (!(buf = (char *)::malloc (len)))
        return false;
    *ret = buf;
    if (GRead (buf, len)) {
        return true;
    } else {
        ::free(buf);
        *ret = 0;
        return false;
    }
}

void Unlocker::reapVerify()
{
    m_notifier->setEnabled( false );
    m_notifier->deleteLater();
    m_notifier = 0;
    ::close( m_fd );
    int status;
    while (::waitpid( m_pid, &status, 0 ) < 0)
        if (errno != EINTR) { // This should not happen ...
            cantCheck();
            return;
        }
    if (WIFEXITED(status))
        switch (WEXITSTATUS(status)) {
        case AuthOk:
            m_greet->succeeded();
            emit greeterAccepted();
            return;
        case AuthBad:
            m_greet->failed();
            emit greeterFailed();
            m_failedLock = true;
            QTimer::singleShot(1500, this, SLOT(failedTimer()));
            //KNotification::event( QLatin1String( "unlockfailed" ) );*/
            return;
        case AuthAbort:
            return;
        }
    cantCheck();
}

void Unlocker::handleVerify()
{
    int ret;
    char *arr;

    if (GRecvInt( &ret )) {
        switch (ret) {
        case ConvGetBinary:
            if (!GRecvArr( &arr ))
                break;
            m_greet->binaryPrompt( arr, false );
            if (arr)
                ::free( arr );
            return;
        case ConvGetNormal:
            if (!GRecvArr( &arr ))
                break;
            m_greet->textPrompt( arr, true, false );
            if (arr)
                ::free( arr );
            return;
        case ConvGetHidden:
            if (!GRecvArr( &arr ))
                break;
            m_greet->textPrompt( arr, false, false );
            if (arr)
                ::free( arr );
            return;
        case ConvPutInfo:
            if (!GRecvArr( &arr ))
                break;
            if (!m_greet->textMessage( arr, false ))
                emit greeterMessage(QString::fromLocal8Bit(arr));
            ::free( arr );
            return;
        case ConvPutError:
            if (!GRecvArr( &arr ))
                break;
            if (!m_greet->textMessage( arr, true ))
                emit greeterMessage(QString::fromLocal8Bit(arr));
            ::free( arr );
            return;
        }
    }
    reapVerify();
}

////// greeter plugin callbacks

void Unlocker::gplugReturnText( const char *text, int tag )
{
    GSendStr( text );
    if (text)
        GSendInt( tag );
}

void Unlocker::gplugReturnBinary( const char *data )
{
    if (data) {
        unsigned const char *up = (unsigned const char *)data;
        int len = up[3] | (up[2] << 8) | (up[1] << 16) | (up[0] << 24);
        if (!len)
            GSendArr( 4, data );
        else
            GSendArr( len, data );
    } else
        GSendArr( 0, 0 );
}

void Unlocker::gplugSetUser( const QString & )
{
    // ignore ...
}

void Unlocker::cantCheck()
{
    m_greet->failed();
    emit greeterMessage(i18n("Cannot unlock the session because the authentication system failed to work!"));
    m_greet->revive();
}

//---------------------------------------------------------------------------
//
// Starts the kcheckpass process to check the user's password.
//
void Unlocker::gplugStart()
{
    int sfd[2];
    char fdbuf[16];

    if (m_notifier)
        return;
    if (::socketpair(AF_LOCAL, SOCK_STREAM, 0, sfd)) {
        cantCheck();
        return;
    }
    if ((m_pid = ::fork()) < 0) {
        ::close(sfd[0]);
        ::close(sfd[1]);
        cantCheck();
        return;
    }
    if (!m_pid) {
        ::close(sfd[0]);
        sprintf(fdbuf, "%d", sfd[1]);
        execlp(QFile::encodeName(KStandardDirs::findExe(QLatin1String( "kcheckpass" ))).data(),
               "kcheckpass",
               "-m", m_pluginHandle.info->method,
               "-S", fdbuf,
               (char *)0);
        _exit(20);
    }
    ::close(sfd[1]);
    m_fd = sfd[0];
    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, SIGNAL(activated(int)), SLOT(handleVerify()));
}

void Unlocker::gplugChanged()
{
}

void Unlocker::gplugActivity()
{
    // ignore
}

void Unlocker::gplugMsgBox(QMessageBox::Icon type, const QString &text)
{
    Q_UNUSED(type)
    emit greeterMessage(text);
}

bool Unlocker::gplugHasNode(const QString &)
{
    return false;
}

} // end namespace
#include "unlocker.moc"
