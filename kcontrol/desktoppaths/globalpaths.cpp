/**
 *  Copyright (c) Martin R. Jones 1996
 *  Copyright (c) Bernd Wuebben 1998
 *  Copyright (c) Christian Tibirna 1998
 *  Copyright 1998-2007 David Faure <faure@kde.org>
 *  Copyright (c) 2010 Matthias Fuchs <mat69@gmx.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

//
//
// "Desktop Options" Tab for KDesktop configuration
//
// (c) Martin R. Jones 1996
// (c) Bernd Wuebben 1998
//
// Layouts
// (c) Christian Tibirna 1998
// Port to KControl, split from Misc Tab, Port to KControl2
// (c) David Faure 1998
// Desktop menus, paths
// (c) David Faure 2000


// Own
#include "globalpaths.h"

// Qt
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QFormLayout>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QtDBus/QtDBus>

// KDE
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kfileitem.h>
#include <kglobalsettings.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "kcmdesktoppaths.h"

//-----------------------------------------------------------------------------

DesktopPathConfig::DesktopPathConfig(QWidget *parent, const QVariantList &)
    : KCModule( KcmDesktopPaths::componentData(), parent )
{
  QFormLayout *lay = new QFormLayout(this);
  lay->setMargin(0);

  setQuickHelp( i18n("<h1>Paths</h1>\n"
    "This module allows you to choose where in the filesystem the "
    "files on your desktop should be stored.\n"
    "Use the \"Whats This?\" (Shift+F1) to get help on specific options."));

  urDesktop = addRow(lay, i18n("Desktop path:"),
                     i18n("This folder contains all the files"
                          " which you see on your desktop. You can change the location of this"
                          " folder if you want to, and the contents will move automatically"
                          " to the new location as well."));

  urAutostart = addRow(lay, i18n("Autostart path:"),
                       i18n("This folder contains applications or"
                            " links to applications (shortcuts) that you want to have started"
                            " automatically whenever KDE starts. You can change the location of this"
                            " folder if you want to, and the contents will move automatically"
                            " to the new location as well."));

  urDocument = addRow(lay, i18n("Documents path:"),
                      i18n("This folder will be used by default to "
                           "load or save documents from or to."));

  urDownload = addRow(lay, i18n("Downloads path:"),
                      i18n("This folder will be used by default to "
                           "save your downloaded items."));

  urMovie = addRow(lay, i18n("Movies path:"),
                   i18n("This folder will be used by default to "
                        "load or save movies from or to."));

  urPicture = addRow(lay, i18n("Pictures path:"),
                     i18n("This folder will be used by default to "
                          "load or save pictures from or to."));

  urMusic = addRow(lay, i18n("Music path:"),
                   i18n("This folder will be used by default to "
                        "load or save music from or to."));
}

KUrlRequester* DesktopPathConfig::addRow(QFormLayout *lay, const QString& label, const QString& whatsThis)
{
    KUrlRequester* ur = new KUrlRequester(this);
    ur->setMode(KFile::Directory | KFile::LocalOnly);
    lay->addRow(label, ur);
    connect(ur, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    lay->labelForField(ur)->setWhatsThis(whatsThis);
    ur->setWhatsThis(whatsThis);
    return ur;
}

void DesktopPathConfig::load()
{
    // Desktop Paths
    urDesktop->setUrl( KGlobalSettings::desktopPath() );
    urAutostart->setUrl( KGlobalSettings::autostartPath() );
    urDocument->setUrl( KGlobalSettings::documentPath() );
    urDownload->setUrl( KGlobalSettings::downloadPath() );
    urMovie->setUrl( KGlobalSettings::videosPath() );
    urPicture->setUrl( KGlobalSettings::picturesPath() );
    urMusic->setUrl( KGlobalSettings::musicPath() );
    emit changed(false);
}

void DesktopPathConfig::defaults()
{
    // Desktop Paths - keep defaults in sync with kglobalsettings.cpp
    urDesktop->setUrl( QString(QDir::homePath() + "/Desktop") );
    urAutostart->setUrl( QString(KGlobal::dirs()->localkdedir() + "Autostart/") );
    urDocument->setUrl( QString(QDir::homePath() + "/Documents") );
    urDownload->setUrl( QString(QDir::homePath() + "/Downloads") );
    urMovie->setUrl( QString(QDir::homePath() + "/Movies") );
    urPicture->setUrl( QString(QDir::homePath() + "/Pictures") );
    urMusic->setUrl( QString(QDir::homePath() + "/Music") );
}

// the following method is copied from kdelibs/kdecore/config/kconfiggroup.cpp
static bool cleanHomeDirPath( QString &path, const QString &homeDir )
{
#ifdef Q_WS_WIN //safer
    if (!QDir::convertSeparators(path).startsWith(QDir::convertSeparators(homeDir)))
        return false;
#else
    if (!path.startsWith(homeDir))
        return false;
#endif

    int len = homeDir.length();
    // replace by "$HOME" if possible
    if (len && (path.length() == len || path[len] == '/')) {
        path.replace(0, len, QString::fromLatin1("$HOME"));
        return true;
    } else
        return false;
}

// TODO this functionality is duplicated in libkonq - keep it only there and export

static QString translatePath( QString path ) // krazy:exclude=passbyvalue
{
    // keep only one single '/' at the beginning - needed for cleanHomeDirPath()
    while (path[0] == '/' && path[1] == '/')
        path.remove(0,1);

    // we probably should escape any $ ` and \ characters that may occur in the path, but the Qt code that reads back
    // the file doesn't unescape them so not much point in doing so

    // All of the 3 following functions to return the user's home directory
    // can return different paths. We have to test all them.
    const QString homeDir0 = QFile::decodeName(qgetenv("HOME"));
    const QString homeDir1 = QDir::homePath();
    const QString homeDir2 = QDir(homeDir1).canonicalPath();
    if (cleanHomeDirPath(path, homeDir0) ||
        cleanHomeDirPath(path, homeDir1) ||
        cleanHomeDirPath(path, homeDir2) ) {
        // kDebug() << "Path was replaced\n";
    }

    return path;
}

void DesktopPathConfig::save()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup configGroup( config, "Paths" );

    bool pathChanged = false;
    bool autostartMoved = false;

    KUrl desktopURL( KGlobalSettings::desktopPath() );

    KUrl autostartURL( KGlobalSettings::autostartPath() );
    KUrl newAutostartURL = urAutostart->url();

    if ( !urDesktop->url().equals( desktopURL, KUrl::CompareWithoutTrailingSlash ) )
    {
        // Test which other paths were inside this one (as it is by default)
        // and for each, test where it should go.
        // * Inside destination -> let them be moved with the desktop (but adjust name if necessary)
        // * Not inside destination -> move first
        // !!!
        kDebug() << "desktopURL=" << desktopURL;
        QString urlDesktop = urDesktop->url().toLocalFile();
        if ( !urlDesktop.endsWith('/'))
            urlDesktop+='/';

        if ( desktopURL.isParentOf( autostartURL ) )
        {
            kDebug() << "Autostart is on the desktop";

            // Either the Autostart field wasn't changed (-> need to update it)
            if ( newAutostartURL.equals( autostartURL, KUrl::CompareWithoutTrailingSlash ) )
            {
                // Hack. It could be in a subdir inside desktop. Hmmm... Argl.
                urAutostart->setUrl( QString(urlDesktop + "Autostart/") );
                kDebug() << "Autostart is moved with the desktop";
                autostartMoved = true;
            }
            // or it has been changed (->need to move it from here)
            else
            {
                KUrl futureAutostartURL;
                futureAutostartURL.setUrl( urlDesktop + "Autostart/" );
                if ( newAutostartURL.equals( futureAutostartURL, KUrl::CompareWithoutTrailingSlash ) )
                    autostartMoved = true;
                else
                    autostartMoved = moveDir( KUrl( KGlobalSettings::autostartPath() ), KUrl( urAutostart->url() ), i18n("Autostart") );
            }
        }

        if ( moveDir( KUrl( KGlobalSettings::desktopPath() ), KUrl( urlDesktop ), i18n("Desktop") ) )
        {
            //save in XDG path
            const QString userDirsFile(KGlobal::dirs()->localxdgconfdir() + QLatin1String("user-dirs.dirs"));
            KConfig xdgUserConf( userDirsFile, KConfig::SimpleConfig );
            KConfigGroup g( &xdgUserConf, "" );
            g.writeEntry( "XDG_DESKTOP_DIR", QString("\"" + translatePath( urlDesktop ) + "\"") );
            pathChanged = true;
        }
    }

    if ( !newAutostartURL.equals( autostartURL, KUrl::CompareWithoutTrailingSlash ) )
    {
        if (!autostartMoved)
            autostartMoved = moveDir( KUrl( KGlobalSettings::autostartPath() ), KUrl( urAutostart->url() ), i18n("Autostart") );
        if (autostartMoved)
        {
            configGroup.writePathEntry( "Autostart", urAutostart->url().toLocalFile(), KConfigBase::Normal | KConfigBase::Global );
            pathChanged = true;
        }
    }

    config->sync();

    if (xdgSavePath(urDocument, KGlobalSettings::documentPath(), "XDG_DOCUMENTS_DIR", i18n("Documents")))
        pathChanged = true;

    if (xdgSavePath(urDownload, KGlobalSettings::downloadPath(), "XDG_DOWNLOAD_DIR", i18n("Downloads")))
        pathChanged = true;

    if (xdgSavePath(urMovie, KGlobalSettings::videosPath(), "XDG_VIDEOS_DIR", i18n("Movies")))
        pathChanged = true;

    if (xdgSavePath(urPicture, KGlobalSettings::picturesPath(), "XDG_PICTURES_DIR", i18n("Pictures")))
        pathChanged = true;

    if (xdgSavePath(urMusic, KGlobalSettings::musicPath(), "XDG_MUSIC_DIR", i18n("Music")))
        pathChanged = true;

    if (pathChanged) {
        kDebug() << "sending message SettingsChanged";
        KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_PATHS);
    }
}

bool DesktopPathConfig::xdgSavePath(KUrlRequester* ur, const KUrl& currentUrl, const char* xdgKey, const QString& type)
{
    KUrl newUrl = ur->url();
    //url might be empty, use QDir::homePath (the default for xdg) then
    if (!newUrl.isValid()) {
        newUrl = KUrl(QDir::homePath());
    }
    if (!newUrl.equals(currentUrl, KUrl::CompareWithoutTrailingSlash)) {
        const QString path = newUrl.toLocalFile();
        if (!QDir(path).exists()) {
            // Check permissions
            if (KStandardDirs::makeDir(path)) {
                QDir().rmdir(path); // rmdir again, so that we get a fast rename
            } else {
                KMessageBox::sorry(this, KIO::buildErrorString(KIO::ERR_COULD_NOT_MKDIR, path));
                ur->setUrl(currentUrl); // revert
                return false;
            }
        }
        if (moveDir(currentUrl, newUrl, type)) {
            //save in XDG user-dirs.dirs config file, this is where KGlobalSettings/QDesktopServices reads from.
            const QString userDirsFile(KGlobal::dirs()->localxdgconfdir() + QLatin1String("user-dirs.dirs"));
            KConfig xdgUserConf(userDirsFile, KConfig::SimpleConfig);
            KConfigGroup g(&xdgUserConf, "");
            g.writeEntry(xdgKey, QString("\"" + translatePath(path) + "\""));
            return true;
        }
    }
    return false;
}

bool DesktopPathConfig::moveDir( const KUrl & src, const KUrl & dest, const QString & type )
{
    if (!src.isLocalFile() || !dest.isLocalFile())
        return true;
    if (!QFile::exists(src.toLocalFile()))
        return true;
    // Do not move $HOME! #193057
    const QString translatedPath = translatePath(src.toLocalFile());
    if (translatedPath == "$HOME" || translatedPath == "$HOME/") {
        return true;
    }

    m_ok = true;

    QString question;
    KGuiItem yesItem;
    KGuiItem noItem;
    if (QFile::exists(dest.toLocalFile())) {
        // TODO: check if the src dir is empty? Nothing to move, then...
        question = i18n("The path for '%1' has been changed.\nDo you want the files to be moved from '%2' to '%3'?",
                        type, src.toLocalFile(),
                        dest.toLocalFile());
        yesItem = KGuiItem(i18nc("Move files from old to new place", "Move"));
        noItem = KGuiItem(i18nc("Use the new directory but do not move files", "Do not Move"));
    } else {
        question = i18n("The path for '%1' has been changed.\nDo you want to move the directory '%2' to '%3'?",
                        type, src.toLocalFile(),
                        dest.toLocalFile());
        yesItem = KGuiItem(i18nc("Move the directory", "Move"));
        noItem = KGuiItem(i18nc("Use the new directory but do not move anything", "Do not Move"));
    }

    // Ask for confirmation before moving the files
    if (KMessageBox::questionYesNo(this, question, i18n("Confirmation Required"),
                                   yesItem, noItem)
            == KMessageBox::Yes )
    {
        if (QFile::exists(dest.toLocalFile())) {
            // Destination already exists -- should always be the case, for most types,
            // but maybe not for the complex autostart case (to be checked...)
            m_copyToDest = dest;
            m_copyFromSrc = src;
            KIO::ListJob* job = KIO::listDir( src );
            job->setAutoDelete(false); // see <noautodelete> below
            job->ui()->setWindow(this);
            job->ui()->setAutoErrorHandlingEnabled(true);
            connect(job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)),
                    this, SLOT(slotEntries(KIO::Job*,KIO::UDSEntryList)));
            // slotEntries will move every file/subdir individually into the dest
            job->exec();
            if (m_ok) {
                QDir().rmdir(src.toLocalFile()); // hopefully it's empty by now
            }
            delete job;
        }
        else
        {
            kDebug() << "Direct move from" << src << "to" << dest;
            KIO::Job * job = KIO::move( src, dest );
            job->ui()->setWindow(this);
            connect(job, SIGNAL(result(KJob*)), this, SLOT(slotResult(KJob*)));
            job->exec();
        }
    }
    kDebug() << "DesktopPathConfig::slotResult returning " << m_ok;
    return m_ok;
}

void DesktopPathConfig::slotEntries(KIO::Job*, const KIO::UDSEntryList& list)
{
    QListIterator<KIO::UDSEntry> it(list);
    while (it.hasNext()) {
        KFileItem file(it.next(), m_copyFromSrc, true, true);
        kDebug() << file.url();
        if (file.url() == m_copyFromSrc || file.url().fileName() == "..") {
            continue;
        }

        KIO::Job * moveJob = KIO::move(file.url(), m_copyToDest);
        moveJob->ui()->setWindow(this);
        connect(moveJob, SIGNAL(result(KJob*)), this, SLOT(slotResult(KJob*)));
        moveJob->exec(); // sub-event loop here. <noautodelete>: the main job is not autodeleted because it would be deleted here
    }
}

void DesktopPathConfig::slotResult( KJob * job )
{
    if (job->error()) {
        if ( job->error() != KIO::ERR_DOES_NOT_EXIST )
            m_ok = false;

        // If the source doesn't exist, no wonder we couldn't move the dir.
        // In that case, trust the user and set the new setting in any case.

        static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
    }
}

#include "globalpaths.moc"
