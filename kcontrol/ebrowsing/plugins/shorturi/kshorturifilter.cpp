/*
    kshorturifilter.h

    This file is part of the KDE project
    Copyright (C) 2000 Dawit Alemayehu <adawit@kde.org>
    Copyright (C) 2000 Malte Starostik <starosti@zedat.fu-berlin.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

#include <qdir.h>
#include <qptrlist.h>
#include <qregexp.h>

#include <kurl.h>
#include <kdebug.h>
#include <kprotocolinfo.h>
#include <klocale.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kconfig.h>

//#include "kshorturiopts.h"
#include "kshorturifilter.h"


#define FQDN_PATTERN    "[a-zA-Z][a-zA-Z0-9-]*\\.[a-zA-Z]"
#define HOSTPORT_PATTERN "[a-zA-Z][a-zA-Z0-9-]*:[0-9]+"
#define IPv4_PATTERN    "[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?:?[[0-9][0-9]?[0-9]?]?/?"
#define ENV_VAR_PATTERN "\\$[a-zA-Z_][a-zA-Z0-9_]*"

#define QFL1(x) QString::fromLatin1(x)

typedef QMap<QString,QString> EntryMap;

KShortURIFilter::KShortURIFilter( QObject *parent, const char *name,
                                  const QStringList & /*args*/ )
                :KURIFilterPlugin( parent, name ? name : "kshorturifilter", 1.0),
                 DCOPObject("KShortURIFilterIface")
{
    configure();
    m_strDefaultProtocol = QFL1("http://");
}

bool KShortURIFilter::isValidShortURL( const QString& cmd ) const
{
  // Loose many of the QRegExp matches as they tend
  // to slow things down.  They are also unnecessary!! (DA)
    if ( cmd[cmd.length()-1] == '&' ||     // must not end with '&'
       (!cmd.contains('.') && !cmd.contains(':')) ||             // must contain either '.' or ':'
       cmd.contains(QFL1("||")) || cmd.contains(QFL1("&&")) ||   // must not look like shell
       cmd.contains(QRegExp(QFL1("[ ;<>]"))) )  // must not contain space, ;, < or >
       return false;

  return true;
}

QString KShortURIFilter::removeArgs( const QString& _cmd ) const
{
  QString cmd( _cmd );
  if( cmd[0] != '\'' && cmd[0] != '"' )
  {
    // Remove command-line options (look for first non-escaped space)
    int spacePos = 0;
    do {
      spacePos = cmd.find( ' ', spacePos+1 );
    } while ( spacePos > 1 && cmd[spacePos - 1] == '\\' );
    if( spacePos > 0 )
    {
      cmd = cmd.left( spacePos );
      //kdDebug() << k_funcinfo << "spacePos=" << spacePos << " returning " << cmd << endl;
    }
  }
  return cmd;
}

bool KShortURIFilter::filterURI( KURIFilterData& data ) const
{
 /*
  * Here is a description of how the shortURI deals with the supplied
  * data.  First it expands any environment variable settings and then
  * deals with special shortURI cases. These special cases are the "smb:"
  * URL scheme which is very specific to KDE, "#" and "##" which are
  * shortcuts for man:/ and info:/ protocols respectively. It then handles
  * local files.  Then it checks to see if the URL is valid and one that is
  * supported by KDE's IO system.  If all the above checks fails, it simply
  * lookups the URL in the user-defined list and returns without filtering
  * if it is not found. TODO: the user-defined table is currently only manually
  * hackable and is missing a config dialog.
  */
  KURL url = data.uri();
  QString cmd = url.url();
  QString extra; // stuff that must appended to the cmd
  bool isMalformed = url.isMalformed();

  // Extract path if URL is local
  // (this removes "file:" but also decodes back %20 etc.)
  if( url.isLocalFile() && !isMalformed ) {
    cmd = url.path();
    if ( url.hasRef() )
        extra = QFL1("#") + url.ref();
  }

  // Note: do NOT use "url" below this point

  bool isLocalFullPath = cmd[0] == '/';
  //kdDebug() << "KShortURIFilter::filterURI cmd=" << cmd << endl;

  // TODO: Make this a bit more intelligent for Minicli! There
  // is no need to make comparisons if the supplied data is a local
  // executable and only the argument part, if any, changed! (Dawit)
  // You mean caching the last filtering, to try and reuse it, to save stat()s? (David)

  // Handle MAN & INFO pages shortcuts...
  QString man_proto = QFL1("man:");
  QString info_proto = QFL1("info:");
  if( cmd[0] == '#' ||
      cmd.find( man_proto, 0, true ) == 0 ||
      cmd.find( info_proto, 0, true ) == 0 )
  {
    if( cmd.left(2) == QFL1("##") )
      cmd = QFL1("info:/") + cmd.mid(2);
    else if ( cmd[0] == '#' )
      cmd = QFL1("man:/") + cmd.mid(1);

    else if ((cmd==info_proto) || (cmd==man_proto))
      cmd+='/';

    setFilteredURI( data, cmd );
    setURIType( data, KURIFilterData::HELP );
    return true;
  }

  // Expanding shortcut to HOME URL...
  if( cmd[0] == '~' )
  {
    int slashPos = cmd.find('/');
    if( slashPos == -1 )
      slashPos = cmd.length();
    if( slashPos == 1 )   // ~/
    {
      cmd.replace ( 0, 1, QDir::homeDirPath() );
      //url = cmd;
    }
    else // ~username/
    {
      QString user = cmd.mid( 1, slashPos-1 );
      struct passwd *dir = getpwnam(user.local8Bit().data());
      if( dir && strlen(dir->pw_dir) )
      {
        cmd.replace (0, slashPos, QString::fromLocal8Bit(dir->pw_dir));
        //url = cmd;          // update the URL...
      }
      else
      {
        QString msg = dir ? i18n("<qt><b>%1</b> doesn't have a home directory!</qt>").arg(user) :
                            i18n("<qt>There is no user called <b>%1</b>.</qt>").arg(user);
        setErrorMsg( data, msg );
        setURIType( data, KURIFilterData::ERROR );
        // Always return true for error conditions so
        // that other filters will not be invoked !!
        return true;
      }
    }
    isLocalFullPath = cmd[0] == '/';
  }
  else if ( cmd[0] == '$' ) {
      // Environment variable expansion.
      QRegExp r (QFL1(ENV_VAR_PATTERN));
      if ( r.search( cmd ) == 0 ) {
          const char* exp = getenv( cmd.mid( 1, r.matchedLength() - 1 ).local8Bit().data() );
          if(exp)
              cmd.replace( 0, r.matchedLength(), QString::fromLocal8Bit(exp) );
      }
      isLocalFullPath = cmd[0] == '/';
  }

  // Checking for local resource match...
  // Determine if "uri" is an absolute path to a local resource  OR
  // A local resource with a supplied absolute path in KURIFilterData
  QString abs_path = data.absolutePath();

  bool canBeAbsolute = (isMalformed && !abs_path.isEmpty());
  bool canBeLocalAbsolute = (canBeAbsolute && abs_path[0] =='/');
  //kdDebug() << "abs_path=" << abs_path << " malformed=" << isMalformed << " canBeLocalAbsolute=" << canBeLocalAbsolute << endl;
  bool exists = false;
  struct stat buff;
  if ( canBeLocalAbsolute )
  {
    QString abs = QDir::cleanDirPath( abs_path );
    // combine absolute path (abs_path) and relative path (cmd) into abs_path
    int len = abs.length();
    if( (len==1 && cmd[0]=='.') || (len==2 && cmd[0]=='.' && cmd[1]=='.') )
        cmd += '/';
    //kdDebug() << "adding " << abs << " and " << cmd << endl;
    abs = QDir::cleanDirPath(abs + '/' + cmd);
    //kdDebug() << "checking whether " << abs << " exists." << endl;
    // Check if it exists
    if( stat( abs.local8Bit().data(), &buff ) == 0 ) {
        cmd = abs; // yes -> store as the new cmd
        exists = true;
        isLocalFullPath = true;
    }
  }

  if( isLocalFullPath && !exists )
  {
    exists = ( stat( cmd.local8Bit().data() , &buff ) == 0 );
  }

  //kdDebug() << "cmd=" << cmd << " isLocalFullPath=" << isLocalFullPath << " exists=" << exists << endl;
  if( exists )
  {
    // Can be abs path to file or directory, or to executable with args
    bool isDir = S_ISDIR( buff.st_mode );
    if( !isDir && access (cmd.local8Bit().data(), X_OK) == 0 )
    {
      //kdDebug() << "Abs path to EXECUTABLE" << endl;
      setFilteredURI( data, cmd );
      setURIType( data, KURIFilterData::EXECUTABLE );
      return true;
    }
    // Open "uri" as file:/xxx if it is a non-executable local resource.
    if( isDir || S_ISREG( buff.st_mode ) )
    {
      cmd += extra;
      setFilteredURI( data, cmd );
      setURIType( data, ( isDir ) ? KURIFilterData::LOCAL_DIR : KURIFilterData::LOCAL_FILE );
      return true;
    }
    // Should we return LOCAL_FILE for non-regular files too?
    kdDebug() << "File found, but not a regular file nor dir... socket?" << endl;
  }

  // Let us deal with possible relative URLs to see
  // if it is executable under the user's $PATH variable.
  // We try hard to avoid parsing any possible command
  // line arguments or options that might have been supplied.
  QString exe = removeArgs( cmd );
  //kdDebug() << k_funcinfo << "findExe with " << exe << endl;
  if( !KStandardDirs::findExe( exe ).isNull() )
  {
    //kdDebug() << "EXECUTABLE  exe=" << exe << endl;
    setFilteredURI( data, exe );
    // check if we have command line arguments
    if( exe != cmd )
        setArguments(data, cmd.right(cmd.length() - exe.length()));
    setURIType( data, KURIFilterData::EXECUTABLE );
    return true;
  }

  // Process URLs of known and supported protocols so we don't have
  // to resort to the pattern matching scheme below which can possibly
  // be slow things down...
  if ( !isMalformed && !isLocalFullPath )
  {
    QStringList protocols = KProtocolInfo::protocols();
    for( QStringList::ConstIterator it = protocols.begin(); it != protocols.end(); it++ )
    {
      if( (cmd.left((*it).length()).lower() == *it) )
      {
        setFilteredURI( data, cmd );
        if ( *it == QFL1("man") || *it == QFL1("help") )
          setURIType( data, KURIFilterData::HELP );
        else
          setURIType( data, KURIFilterData::NET_PROTOCOL );
        return true;
      }
    }
  }

#if 0
  // Provided as a filter for remote URLs.  Example,
  // if the current path is ftp://ftp.kde.org/pub/
  // and user typed ../ in the location bar they would
  // correctly get ftp://ftp.kde.org/. Is that cool or what ??
  // David: yes but it's against the documentation for KURIFilterData::setAbsolutePath() :)
  // Plus, it breaks if you type something like "www.kde.org" after going to an FTP site.
  // It'll append www.kde.org, without checking that the resulting FTP url exists.
  // A stat() over FTP can be slow... too slow for a shorturifilter IMHO -> disabled.
  if( canBeAbsolute && !canBeLocalAbsolute )
  {
    KURL u( KURL( data.absolutePath() ), cmd );
    if( !u.isMalformed() )
    {
      setFilteredURI( data, u.url() );
      setURIType( data, KURIFilterData::NET_PROTOCOL );
      return true;
    }
  }
#endif

  // Okay this is the code that allows users to supply custom matches for
  // specific URLs using Qt's regexp class. This is hard-coded for now in
  // the constructor, but will soon be moved to the config dialog so that
  // people can configure this stuff.  This is perhaps one of those unecessary
  // but somewhat useful features that usually makes people go WHOO and WHAAA.
  if ( !cmd.contains( ' ' ) )
  {
    QRegExp match;
    QValueList<URLHint>::ConstIterator it;
    for( it = m_urlHints.begin(); it != m_urlHints.end(); ++it )
    {
      match = (*it).regexp;
      if ( match.search( cmd, 0 ) == 0 )
      {
        //kdDebug() << "match - prepending " << (*it).prepend << endl;
        cmd.prepend( (*it).prepend );
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        return true;
      }
    }

    // If cmd is NOT a local resource, check if it is a valid "shortURL"
    // candidate and append the default protocol the user supplied. (DA)
    if ( isMalformed && isValidShortURL(cmd) )
    {
      //kdDebug() << "valid short url, from malformed url -> using default proto=" << m_strDefaultProtocol << endl;
      cmd.insert( 0, m_strDefaultProtocol );
      setFilteredURI( data, cmd );
      setURIType( data, KURIFilterData::NET_PROTOCOL );
      return true;
    }
  }

  // If we previously determined that the URL might be a file,
  // and if it doesn't exist, then error
  if( isLocalFullPath && !exists )
  {
    //kdDebug() << "fileNotFound -> ERROR" << endl;
    setErrorMsg( data, QString::null );
    setURIType( data, KURIFilterData::ERROR );
    return true;
  }

  // If we reach this point, we cannot filter
  // this thing so simply return false so that
  // other filters, if present, can take a crack
  // at it.
  // kdDebug() << "KShortURIFilter::filterURI returning false!" << endl;
  return false;
}

KCModule* KShortURIFilter::configModule( QWidget*, const char* ) const
{
    return 0; //new KShortURIOptions( parent, name );
}

QString KShortURIFilter::configName() const
{
    return i18n("&ShortURLs");
}

void KShortURIFilter::configure()
{
    KConfig config( name() + QFL1("rc"), false, false );
    EntryMap map = config.entryMap( QFL1("Pattern Matching") );
    if( !map.isEmpty() )
    {
        EntryMap::Iterator it = map.begin();
        for( ; it != map.end(); ++it )
            m_urlHints.append( URLHint(it.key(), it.data()) );
    }

    // Include some basic defaults.  Note these will always be
    // overridden by a users entries. TODO: Make this configurable
    // from the control panel.
    m_urlHints.append( URLHint(QFL1(IPv4_PATTERN), QFL1("http://")) );
    m_urlHints.append( URLHint(QFL1(FQDN_PATTERN), QFL1("http://")) );
    m_urlHints.append( URLHint(QFL1(HOSTPORT_PATTERN), QFL1("http://")) );
}

K_EXPORT_COMPONENT_FACTORY( libkshorturifilter,
	                    KGenericFactory<KShortURIFilter>( "kshorturifilter" ) );

#include "kshorturifilter.moc"
