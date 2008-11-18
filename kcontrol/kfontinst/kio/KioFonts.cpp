/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config-workspace.h>
#include "KioFonts.h"
#include <stdlib.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <errno.h>
#include <utime.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <kio/ioslave_defaults.h>
#include <KDE/KMimeType>
#include <KDE/KMessageBox>
#include <QtCore/QDir>
#include <QtCore/QDataStream>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>
#include <QtGui/QFontDatabase>
#include <KDE/KComponentData>
#include <kde_file.h>
#include <KDE/KTemporaryFile>
#include <KDE/SuProcess>
#include <KDE/KDebug>
#include <KDE/KStandardDirs>
#include <KDE/KMD5>
#include <KDE/KZip>
#include <KDE/KConfigGroup>
#include <kxftconfig.h>
#include <fontconfig/fontconfig.h>
#include "KfiConstants.h"
#include "Fc.h"
#include "Misc.h"
#include "SuProc.h"
#include "Socket.h"
#include <ctype.h>

// Enable the following so that all URLs are actually <family>, <style>, e.g.
//   without #define: fonts:/arial.ttf
//   with #define:    fonts:/Arial, Regular.ttf
//
// Not enabled - as it messes things up a little with fonts/group files.
//#define KFI_KIO_ALL_URLS_HAVE_NAME

#define KFI_DBUG kDebug(7000) << '(' << time(NULL) << ')'

#define MAX_IPC_SIZE    (1024*32)
#define DEFAULT_TIMEOUT 2         // Time between last mod and writing files...
#define FC_CACHE_CMD    "fc-cache"

using namespace KDESu;

static const int constMaxFcCheckTime=10;

extern "C"
{
    KDE_EXPORT int kdemain(int argc, char **argv);
}

static KFI::CKioFonts *slaveInstance=NULL;

void kioFontsExitHandler()
{
    if(slaveInstance)
    {
        slaveInstance->cleanup();
        slaveInstance=NULL;
    }
}

int kdemain(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: kio_" KFI_KIO_FONTS_PROTOCOL
                        " protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    KLocale::setMainCatalog(KFI_CATALOGUE);

    KComponentData componentData("kio_" KFI_KIO_FONTS_PROTOCOL);
    KFI::CKioFonts slave(argv[2], argv[3]);

    atexit(kioFontsExitHandler);
    slave.dispatchLoop();

    return 0;
}   

namespace KFI
{

static QString urlString(const KUrl &u)
{
    KUrl url(u);

    if(url.hasUser() && KFI_KIO_FONTS_PROTOCOL==url.protocol() && KFI_SYS_USER==url.user())
        url.setUser(QString());
    return url.prettyUrl();
}

static bool addCreateFolderCmd(const QString &folder, QList<CKioFonts::TCommand> &cmd)
{
    if(!Misc::dExists(folder))
    {
        cmd.append(CKioFonts::TCommand(KFI::CMD_CREATE_DIR, folder));
        return true;
    }

    return false;
}

inline bool isSysFolder(const QString &sect)
{
    return i18n(KFI_KIO_FONTS_SYS)==sect || KFI_KIO_FONTS_SYS==sect;
}

inline bool isUserFolder(const QString &sect)
{
    return i18n(KFI_KIO_FONTS_USER)==sect || KFI_KIO_FONTS_USER==sect;
}

inline bool isAllFolder(const QString &sect)
{
    return i18n(KFI_KIO_FONTS_ALL)==sect || KFI_KIO_FONTS_ALL==sect;
}

#ifdef KFI_KIO_ALL_URLS_HAVE_NAME
static const char *constExtensions[]=
            {".ttf", KFI_FONTS_PACKAGE, ".otf", ".pfa", ".pfb", ".ttc",
             ".pcf", ".pcf.gz", ".bdf", ".bdf.gz", NULL };

static QString removeKnownExtension(const KUrl &url)
{
    QString fname(url.fileName());
    int     pos;

    for(int i=0; constExtensions[i]; ++i)
        if(-1!=(pos=fname.lastIndexOf(QString::fromLatin1(constExtensions[i]), -1, Qt::CaseInsensitive)))
            return fname.left(pos);
    return fname;
}

#define removeMultipleExtension(A) removeKnownExtension(A)
static void addKnownExtension(QString &url, const CDisabledFonts::TFileList &files, const QString &name,
                              bool hidden)
{
    if(files.count()>1)
    {
        if(hidden)
            url+='.';
        url+=name+QString::fromLatin1(KFI_FONTS_PACKAGE);
    }
    else
    {
        QString fileName(Misc::getFile(files.first()));
        int     pos(0);

        for(int i=0; constExtensions[i]; ++i)
            if(-1!=(pos=fileName.lastIndexOf(QString::fromLatin1(constExtensions[i]), -1,
                                             Qt::CaseInsensitive)))
            {
                if(hidden)
                    url+='.';
                url+=name+constExtensions[i];
                return;
            }
        url+=fileName;
    }
}
#else
static QString removeMultipleExtension(const KUrl &url)
{
    QString fname(url.fileName());
    int     pos;

    if(-1!=(pos=fname.lastIndexOf(QString::fromLatin1(KFI_FONTS_PACKAGE))))
        fname=fname.left(pos);

    return fname;
}
#endif

static QString modifyName(const QString &fname, bool toUpper=false)
{
    static const char constSymbols[]={ '-', ' ', ':', ';', '/', '~', 0 };

    QString rv(toUpper ? fname.toUpper() : fname.toLower());

    for(int s=0; constSymbols[s]; ++s)
        rv=rv.replace(constSymbols[s], '_');

    return rv;
}

static bool checkFiles(const CDisabledFonts::TFileList &files)
{
    CDisabledFonts::TFileList::ConstIterator it(files.begin()),
                                             end(files.end());

    for(; it!=end; ++it)
        if(!Misc::fExists(*it))
            return false;
    return true;
}

inline int getSize(KIO::UDSEntry &entry)
{
    return entry.numberValue(KIO::UDSEntry::UDS_SIZE, 0);
}

static int getSize(const QByteArray &file)
{
    KDE_struct_stat buff;

    if(-1!=KDE_lstat(file, &buff))
    {
        if (S_ISLNK(buff.st_mode))
        {
            char buffer2[1000];
            int n=readlink(file, buffer2, 1000);
            if(n!= -1)
                buffer2[n]='\0';

            if(-1==KDE_stat(file, &buff))
                return -1;
        }
        return buff.st_size;
    }

    return -1;
}

static int getSize(const CDisabledFonts::TFileList &files)
{
    CDisabledFonts::TFileList::ConstIterator it,
                                             end=files.end();
    int                                      size=0;

    for(it=files.begin(); it!=end; ++it)
        size+=getSize(QFile::encodeName(*it));

    return size;
}

//
// Return real filename
//    if normal file, return file
//    if link, return dest
static QString getReal(const QString &file)
{
    QByteArray      cPath(QFile::encodeName(file));
    KDE_struct_stat buff;

    if(-1!=KDE_lstat(cPath, &buff) && S_ISLNK(buff.st_mode))
    {
        char buffer2[1000];
        int  n=readlink(cPath, buffer2, 1000);

        if(n!= -1)
            buffer2[n]='\0';


        if('.'==buffer2[0]) // Relative link...
        {
            QString linkDest(QString::fromLocal8Bit(buffer2));
            QDir    d(Misc::getDir(file)+Misc::getDir(linkDest));

            return Misc::dirSyntax(d.canonicalPath())+Misc::getFile(linkDest);
        }
        else
            return QString::fromLocal8Bit(buffer2);
    }

    return file;
}

//
// Get the list of files associated with a list of entries...
//    1. get any associated afm or pfms
//    2. Resolve any symlinks
//    3. Remove duplicates (due to symlik and real being in list)
static void getFontFiles(const CDisabledFonts::TFileList &entries, CDisabledFonts::TFileList &files,
                         bool removeSymLinks=true)
{
    CDisabledFonts::TFileList::ConstIterator it,
                                             end=entries.end();

    for(it=entries.begin(); it!=end; ++it)
    {
        QStringList assoc;
        QString     file(removeSymLinks ? getReal(*it) : (*it).path);

        if(-1==files.indexOf(file) && Misc::fExists(file))
            files.append(CDisabledFonts::TFile(file, (*it).face, (*it).foundry));

        Misc::getAssociatedFiles(*it, assoc);

        if(assoc.count())
        {
            QStringList::Iterator fIt,
                                  fEnd=assoc.end();

            for(fIt=assoc.begin(); fIt!=fEnd; ++fIt)
            {
                file=removeSymLinks ? getReal(*fIt) : *fIt;

                if(!files.contains(file) && Misc::fExists(file))
                    files.append(CDisabledFonts::TFile(file));
            }
        }
    }
}

static bool isScalable(const QString &str)
{
    QByteArray cFile(QFile::encodeName(str));

    return Misc::checkExt(cFile, "ttf") || Misc::checkExt(cFile, "otf") || Misc::checkExt(cFile, "ttc") ||
           Misc::checkExt(cFile, "pfa") || Misc::checkExt(cFile, "pfb");
}

static bool isATtc(const QByteArray &file)
{
    //
    // To speed things up, check the files extension 1st...
    if(Misc::checkExt(file, "ttc"))
        return true;
    else
    {
        // No '.ttc' exension match, so try querying with FreeType...
        int       count=0;
        FcPattern *pat=FcFreeTypeQuery((const FcChar8 *)(QFile::encodeName(file).constData()), 0, NULL, &count);

        if(pat)
        {
            FcPatternDestroy(pat);
            return count>1; // Only care if TTC has more than 1 face...
        }
    }
    return false;
}

// Check if
// ...src and dest are the *same* ttc file -> TTC copy
// ...or that src does not exist, and a new dest does -> TTC move
static bool isSameTtc(const QString &src, const QString &dest)
{
    static const int constMaxTimeDiff=20;

    QByteArray cDest(QFile::encodeName(dest));

    if(isATtc(cDest))
    {
        QByteArray cSrc(QFile::encodeName(src));

        KDE_struct_stat srcStat,
                        destStat;
        bool            srcExists=-1!=KDE_lstat(cSrc, &srcStat),
                        destExists=-1!=KDE_lstat(cDest, &destStat);

        // Check that file sizes are the same, and that the dest file is recent...
        if(srcExists && destExists)
        {
            if(srcStat.st_size==destStat.st_size && abs(destStat.st_atime-time(NULL))<constMaxTimeDiff)
            {
                // Sizes match, so check md5 sums...
                QFile srcFile(src),
                      destFile(dest);

                if(srcFile.open(QIODevice::ReadOnly) && destFile.open(QIODevice::ReadOnly))
                {
                    KMD5 srcMd5,
                         destMd5;

                    return srcMd5.update(srcFile) && destMd5.update(destFile) &&
                           srcMd5.verify(destMd5.rawDigest());
                }
            }
        }
        else // In case of move, after the 1st, src won't exist, but dest should (and should be recent!)
            return destExists && abs(destStat.st_atime-time(NULL))<constMaxTimeDiff;
    }

    return false;
}

enum EUrlStatus
{
    BAD_Url,
    Url_OK,
    REDIRECT_Url
};

static KUrl getRedirect(const KUrl &u)
{
    // Go from fonts:/System to fonts:/

    KUrl    redirect(u);
    QString path(u.path()),
            sect(CKioFonts::getSect(path));

    path.remove(sect);
    path.replace("//", "/");
    redirect.setPath(path);

    KFI_DBUG << "Redirect from " << u.path() << " to " << redirect.path();
    return redirect;
}

static bool nonRootSys(const KUrl &u)
{
    return !Misc::root() && isSysFolder(CKioFonts::getSect(u.path()));
}

static bool writeAll(int fd, const char *buf, size_t len)
{
   while(len>0)
   {
      ssize_t written=write(fd, buf, len);
      if (written<0 && EINTR!=errno)
          return false;
      buf+=written;
      len-=written;
   }
   return true;
}

static bool isAAfm(const QString &fname)
{
    if(Misc::checkExt(QFile::encodeName(fname), "afm"))   // CPD? Is this a necessary check?
    {
        QFile file(fname);

        if(file.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&file);
            QString     line;

            for(int lc=0; lc<30 && !stream.atEnd(); ++lc)
            {
                line=stream.readLine();

                if(line.contains("StartFontMetrics"))
                {
                    file.close();
                    return true;
                }
            }

            file.close();
        }
    }

    return false;
}

static bool isAPfm(const QString &fname)
{
    bool ok=false;

    // I know extension checking is bad, but Ghostscript's pf2afm requires the pfm file to
    // have the .pfm extension...
    QByteArray name(QFile::encodeName(fname));

    if(Misc::checkExt(name, "pfm"))
    {
        //
        // OK, the extension matches, so perform a little contents checking...
        FILE *f=fopen(name.constData(), "r");

        if(f)
        {
            static const unsigned long constCopyrightLen =  60;
            static const unsigned long constTypeToExt    =  49;
            static const unsigned long constExtToFname   =  20;
            static const unsigned long constExtLen       =  30;
            static const unsigned long constFontnameMin  =  75;
            static const unsigned long constFontnameMax  = 512;

            unsigned short version=0,
                           type=0,
                           extlen=0;
            unsigned long  length=0,
                           fontname=0,
                           fLength=0;

            fseek(f, 0, SEEK_END);
            fLength=ftell(f);
            fseek(f, 0, SEEK_SET);

            if(2==fread(&version, 1, 2, f) && // Read version
               4==fread(&length, 1, 4, f) &&  // length...
               length==fLength &&
               0==fseek(f, constCopyrightLen, SEEK_CUR) &&   // Skip copyright notice...
               2==fread(&type, 1, 2, f) &&
               0==fseek(f, constTypeToExt, SEEK_CUR) &&
               2==fread(&extlen, 1, 2, f) &&
               extlen==constExtLen &&
               0==fseek(f, constExtToFname, SEEK_CUR) &&
               4==fread(&fontname, 1, 4, f) &&
               fontname>constFontnameMin && fontname<constFontnameMax)
                ok=true;
            fclose(f);
        }
    }

    return ok;
}

// This function is *only* used for the generation of AFMs from PFMs.
static bool isAType1(const QString &fname)
{
    static const char *       constStr="%!PS-AdobeFont-";
    static const unsigned int constStrLen=15;
    static const unsigned int constPfbOffset=6;
    static const unsigned int constPfbLen=constStrLen+constPfbOffset;

    QByteArray name(QFile::encodeName(fname));
    char       buffer[constPfbLen];
    bool       match=false;

    if(Misc::checkExt(name, "pfa"))
    {
        FILE *f=fopen(name.constData(), "r");

        if(f)
        {
            if(constStrLen==fread(buffer, 1, constStrLen, f))
                match=0==memcmp(buffer, constStr, constStrLen);
            fclose(f);
        }
    }
    else if(Misc::checkExt(name, "pfb"))
    {
        static const char constPfbMarker=0x80;

        FILE *f=fopen(name.constData(), "r");

        if(f)
        {
            if(constPfbLen==fread(buffer, 1, constPfbLen, f))
                match=buffer[0]==constPfbMarker && 0==memcmp(&buffer[constPfbOffset], constStr,
                                                             constStrLen);
            fclose(f);
        }
    }

    return match;
}

static QString getMatch(const QString &file, const char *extension)
{
    QString f(Misc::changeExt(file, extension));

    return Misc::fExists(f) ? f : QString();
}

struct KfiFont
{
    struct Path
    {
        Path(const QString &p=QString()) : orig(p) { }

        QString orig,
                modified;

        bool operator==(const Path &p) const { return p.orig==orig; }
    };

    KfiFont(const QString &n=QString(), const QString &p=QString()) : name(n)
        { if(!p.isEmpty()) paths.append(Path(p)); }

    QString     name;
    QList<Path> paths;

    bool operator==(const KfiFont &f) const { return f.name==name; }
};

struct KfiFontList : public QList<KfiFont>
{
    Iterator locate(const KfiFont &t) { int i = indexOf(t); return (-1==i ? end() : (begin()+i)); }
};

static void setName(const QString &orig, QString &modified, int pos, bool hidden)
{
    modified=orig.mid(pos);
    modified=modified.replace('/', '_');
    if(hidden && '.'!=modified[0])
        modified='.'+modified;
}

//
// This function returns a set of maping of from -> to for copy/move operations
static bool getFontList(const CDisabledFonts::TFileList &files, QMap<QString, QString> &map)
{
    // First of all create a list of font files, and their paths
    CDisabledFonts::TFileList::ConstIterator it=files.begin(),
                                             end=files.end();
    KfiFontList                              list;

    for(;it!=end; ++it)
    {
        QString name(Misc::getFile(*it)),
                path(Misc::getDir(*it));
        KfiFontList::Iterator entry=list.locate(KfiFont(name));

        if(entry!=list.end())
        {
            if(!(*entry).paths.contains(path))
                (*entry).paths.append(path);
        }
        else
            list.append(KfiFont(name, path));
    }

    KfiFontList::Iterator fIt(list.begin()),
                              fEnd(list.end());

    for(; fIt!=fEnd; ++fIt)
    {
        bool                            hidden='.'==(*fIt).name[0];
        QList<KfiFont::Path>::Iterator pBegin((*fIt).paths.begin()),
                                        pIt(++pBegin),
                                        pEnd((*fIt).paths.end());
        --pBegin;

        if((*fIt).paths.count()>1)
        {
            // There's more than 1 file with the same name, but in a different location
            // therefore, take the unique part of the path, and replace / with _
            // e.g.
            //     /usr/X11R6/lib/X11/fonts/75dpi/times.pcf.gz
            //     /usr/X11R6/lib/X11/fonts/100dpi/times.pcf.gz
            //
            //   Will produce:
            //     75dpi_times.pcf.gz
            //     100dpi_times.pcf.gz
            int beginLen((*pBegin).orig.length());

            for(; pIt!=pEnd; ++pIt)
            {
                unsigned int len=qMin((*pIt).orig.length(), beginLen);
                bool         modified=false;

                for(unsigned int i=0; i<len; ++i)
                    if((*pIt).orig[i]!=(*pBegin).orig[i])
                    {
                        setName((*pIt).orig, (*pIt).modified, i, hidden);
                        if((*pBegin).modified.isEmpty())
                            setName((*pBegin).orig, (*pBegin).modified, i, hidden);
                        modified=true;
                        break;
                    }
                if(!modified)
                    if(beginLen>(*pIt).orig.length())
                    {
                        if((*pBegin).modified.isEmpty())
                            setName((*pBegin).orig, (*pBegin).modified, (*pIt).orig.length(), hidden);
                    }
                    else
                        setName((*pIt).orig, (*pIt).modified, beginLen, hidden);
            }
        }
        for(pIt=(*fIt).paths.begin(); pIt!=pEnd; ++pIt)
            if(hidden && '.'==(*pIt).modified[0] && '.'==(*fIt).name[0])
                map[(*pIt).orig+(*fIt).name]=(*pIt).modified+(*fIt).name.mid(1);
            else
                map[(*pIt).orig+(*fIt).name]=(*pIt).modified+(*fIt).name;
    }

    return list.count() ? true : false;
}

inline QString getDestFolder(const QString &folder, const QString &file)
{
    return folder+file[0].toLower()+'/';
}

// Extract just the family from a font name
static QString getFamily(const QString &font)
{
    int     commaPos=font.lastIndexOf(',');
    return -1==commaPos ? font : font.left(commaPos);
}

inline qulonglong toBit(QFontDatabase::WritingSystem ws)
{
    return ((qulonglong)1) << (int)ws;
}

//.........the following section is inspired by qfontdatabase_x11.cpp / loadFontConfig


// Unfortunately FontConfig doesn't know about some languages. We have to test these through the
// charset. The lists below contain the systems where we need to do this.
static const struct
{
    QFontDatabase::WritingSystem ws;
    ushort                       ch;
} sampleCharForWritingSystem[] =
{
    { QFontDatabase::Telugu, 0xc15 },
    { QFontDatabase::Kannada, 0xc95 },
    { QFontDatabase::Malayalam, 0xd15 },
    { QFontDatabase::Sinhala, 0xd9a },
    { QFontDatabase::Myanmar, 0x1000 },
    { QFontDatabase::Ogham, 0x1681 },
    { QFontDatabase::Runic, 0x16a0 },
    { QFontDatabase::Any, 0x0 }
};

static qulonglong getWritingSystems(FcPattern *pat)
{
    qulonglong ws(0);

    FcLangSet                                  *langset(0);
    const CDisabledFonts::LangWritingSystemMap *langForWritingSystem(CDisabledFonts::languageForWritingSystemMap());

    if (FcResultMatch==FcPatternGetLangSet(pat, FC_LANG, 0, &langset))
    {
        for (int i = 0; langForWritingSystem[i].lang; ++i)
            if (FcLangDifferentLang!=FcLangSetHasLang(langset, langForWritingSystem[i].lang))
                ws|=toBit(langForWritingSystem[i].ws);
    }
    else
        ws|=toBit(QFontDatabase::Other);

    FcCharSet *cs(0);

    if (FcResultMatch == FcPatternGetCharSet(pat, FC_CHARSET, 0, &cs))
    {
        // some languages are not supported by FontConfig, we rather check the
        // charset to detect these
        for (int i = 0; QFontDatabase::Any!=sampleCharForWritingSystem[i].ws; ++i)
            if (FcCharSetHasChar(cs, sampleCharForWritingSystem[i].ch))
                ws|=toBit(sampleCharForWritingSystem[i].ws);
    }

    return ws;
}

static QString obtainMimeType(const QString &file)
{
    if(Misc::checkExt(file, "ttf") || Misc::checkExt(file, "ttc"))
        return "application/x-font-ttf";
    if(Misc::checkExt(file, "otf"))
        return "application/x-font-otf";
    if(Misc::checkExt(file, "pfa") || Misc::checkExt(file, "pfb"))
        return "application/x-font-type1";
    if(Misc::checkExt(file, "pcf.gz") || Misc::checkExt(file, "pcf"))
        return "application/x-font-pcf";
    if(Misc::checkExt(file, "bdf.gz") || Misc::checkExt(file, "bdf"))
        return "application/x-font-bdf";

    // File extension check failed, use kmimetype to read contents...
    return KMimeType::findByPath(file)->name();
}

//.........

CKioFonts::CKioFonts(const QByteArray &pool, const QByteArray &app)
         : KIO::SlaveBase(KFI_KIO_FONTS_PROTOCOL, pool, app),
           itsRoot(Misc::root()),
           itsAddToSysFc(false),
           itsLastFcCheckTime(0),
           itsFontList(NULL),
           itsSocket(NULL),
           itsSuProc(NULL)
{
    KFI_DBUG;

    slaveInstance=this;

    if(!itsRoot)
    {
        // Set core dump size to 0 because we will have
        // root's password in memory.
        struct rlimit rlim;
        rlim.rlim_cur=rlim.rlim_max=0;
        setrlimit(RLIMIT_CORE, &rlim);
    }
    //
    // Check with fontconfig for folder locations...
    //
    // 1. Get list of fontconfig dirs
    // 2. For user, look for any starting with $HOME - but prefer $HOME/.fonts
    // 3. For system, look for any starting with /usr/local/share - but prefer /usr/local/share/fonts
    // 4. If either are not found, then add to local.conf / .fonts.conf

    FcStrList   *list=FcConfigGetFontDirs(FcInitLoadConfigAndFonts());
    QStringList dirs;
    FcChar8     *dir;

    while((dir=FcStrListNext(list)))
        dirs.append(Misc::dirSyntax((const char *)dir));

    EFolder mainFolder=FOLDER_SYS;

    if(!itsRoot)
    {
        QString home(Misc::dirSyntax(QDir::homePath())),
                defaultDir(Misc::dirSyntax(QDir::homePath()+"/.fonts/")),
                dir(Misc::getFolder(defaultDir, home, dirs));

        if(dir.isEmpty())  // Then no $HOME/ was found in fontconfigs dirs!
        {
            KXftConfig xft(KXftConfig::Dirs, false);
            xft.addDir(defaultDir);
            xft.apply();
            dir=defaultDir;
        }
        mainFolder=FOLDER_USER;
        itsFolders[FOLDER_USER].setLocation(dir, false);
    }

    QString sysDefault("/usr/local/share/fonts/"),
            sysDir(Misc::getFolder(sysDefault, "/usr/local/share/", dirs));

    if(sysDir.isEmpty())
    {
        if(itsRoot)
        {
            KXftConfig xft(KXftConfig::Dirs, true);
            xft.addDir(sysDefault);
            xft.apply();
        }
        else
            itsAddToSysFc=true;

        sysDir=sysDefault;
    }

    itsFolders[FOLDER_SYS].setLocation(sysDir, true);

    //
    // Ensure exists
    if(!Misc::dExists(itsFolders[mainFolder].location))
        Misc::createDir(itsFolders[mainFolder].location);

    updateFontList();
}

void CKioFonts::cleanup()
{
    slaveInstance=NULL;

    KFI_DBUG;
    itsFolders[FOLDER_USER].disabled->save();
    doModified();
    quitHelper();

    delete itsSuProc;
    delete itsSocket;
}

void CKioFonts::listDir(const KUrl &url)
{
    KFI_DBUG << url.path() << " query:" << url.query();

    clearFontList(); // Always refresh font list when listing...
    if(updateFontList() && checkUrl(url, true))
    {
        KIO::UDSEntry entry;
        int           size=0,
                      sections=url.path().split('/', QString::SkipEmptyParts).count();
        if(itsRoot || sections!=0)
        {
            if(!itsRoot && isAllFolder(getSect(url.path())))
            {
                totalSize(itsFolders[FOLDER_SYS].fontMap.count()+itsFolders[FOLDER_SYS].disabled->items().count()+
                          itsFolders[FOLDER_USER].fontMap.count()+itsFolders[FOLDER_USER].disabled->items().count());
                listDir(FOLDER_SYS, entry);
                listDir(FOLDER_USER, entry);
            }
            else
            {
                EFolder folder=getFolder(url);

                totalSize(itsFolders[folder].fontMap.count()+itsFolders[folder].disabled->items().count());
                listDir(folder, entry);
            }
        }
        else
        {
            size=2;
            totalSize(size);
            createFolderUDSEntry(entry, i18n(KFI_KIO_FONTS_USER), itsFolders[FOLDER_USER].location,
                                 false);
            listEntry(entry, false);
            createFolderUDSEntry(entry, i18n(KFI_KIO_FONTS_SYS), itsFolders[FOLDER_SYS].location,
                                 true);
            listEntry(entry, false);
        }

        listEntry(size ? entry : KIO::UDSEntry(), true);
        finished();
    }

    KFI_DBUG << "finished!";
}

void CKioFonts::listDir(EFolder folder, KIO::UDSEntry &entry)
{
    if(itsFolders[folder].fontMap.count())
    {
        TFontMap::Iterator it=itsFolders[folder].fontMap.begin(),
                           end=itsFolders[folder].fontMap.end();

        for ( ; it != end; ++it)
        {
            entry.clear();
            if(createFontUDSEntry(entry, it.key(), it.value().files, it.value().styleVal,
                                  it.value().writingSystems, FOLDER_SYS==folder))
                listEntry(entry, false);
        }
    }

    CDisabledFonts::TFontList::Iterator dIt(itsFolders[folder].disabled->items().begin()),
                                        dEnd(itsFolders[folder].disabled->items().end());

    for(; dIt!=dEnd; ++dIt)
        if(createFontUDSEntry(entry, (*dIt).getName(), (*dIt).files,
                              (*dIt).styleInfo, (*dIt).writingSystems, FOLDER_SYS==folder, true))
            listEntry(entry, false);
}

void CKioFonts::stat(const KUrl &url)
{
    KFI_DBUG << url.prettyUrl() << " query:" << url.query();

    KIO::UDSEntry entry;
    bool          err=false;

    if(checkUrl(url, true, false))
    {
        QString path(url.path(KUrl::RemoveTrailingSlash));

        if(path.isEmpty())
        {
            error(KIO::ERR_COULD_NOT_STAT, urlString(url));
            return;
        }

        QStringList pathList(path.split('/', QString::SkipEmptyParts));

        switch(pathList.count())
        {
            case 0:
                err=!createFolderUDSEntry(entry, i18n("Fonts"),
                                          itsFolders[itsRoot ? FOLDER_SYS : FOLDER_USER].location,
                                          false);
                break;
            case 1:
                if(itsRoot)
                    err=!createStatEntry(entry, url, FOLDER_SYS);
                else
                    if(isUserFolder(pathList[0]))
                        err=!createFolderUDSEntry(entry, i18n(KFI_KIO_FONTS_USER),
                                                  itsFolders[FOLDER_USER].location, false);
                    else if(isSysFolder(pathList[0]))
                        err=!createFolderUDSEntry(entry, i18n(KFI_KIO_FONTS_SYS),
                                                  itsFolders[FOLDER_SYS].location, true);
                    else if(isAllFolder(pathList[0]))
                        err=!createFolderUDSEntry(entry, i18n(KFI_KIO_FONTS_ALL),
                                                  itsFolders[FOLDER_SYS].location, true);
                    else
                    {
                        error(KIO::ERR_SLAVE_DEFINED,
                              i18n("Please specify \"%1\" or \"%2\".",
                                   i18n(KFI_KIO_FONTS_USER), i18n(KFI_KIO_FONTS_SYS)));
                        return;
                    }
                break;
            default:
                err=!createStatEntry(entry, url, getFolder(url));
        }
    }
    else if(!itsRoot && 1==url.path(KUrl::RemoveTrailingSlash)
                              .split('/', QString::SkipEmptyParts).count())
    {
        //
        // If a user (non-root) copies a font to fonts:/, kio_fonts will redirect to fonts:/Personal
        // If the font already exists in fonts:/Personal, konqueror will then do a stat on
        // fonts:/<filename> to get its file size in the "overwrite" dialog.

        //
        // But, the font will not exist in fonts:/<filename> - so we need to see if it exists
        // in fonts:/Personal/<filename> in order to get the correct size. Otherwise konqueror, etc,
        // display 0 bytes!

        KUrl modUrl(url);

        modUrl.setPath(QChar('/')+i18n(KFI_KIO_FONTS_USER)+QChar('/')+url.fileName());
        err=!createStatEntry(entry, modUrl, FOLDER_USER);
    }
    else
    {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Please specify \"%1\" or \"%2\".",
              i18n(KFI_KIO_FONTS_USER), i18n(KFI_KIO_FONTS_SYS)));
        return;
    }

    if(err)
    {
        error(KIO::ERR_DOES_NOT_EXIST, urlString(url));
        return;
    }

    statEntry(entry);
    finished();
}

bool CKioFonts::createStatEntry(KIO::UDSEntry &entry, const KUrl &url, EFolder folder)
{
    KFI_DBUG << url.path();

    // First try to create stat entry without refreshing lists...
    bool ok=createStatEntryReal(entry, url, folder) && getSize(entry)>0;

    // Hmm... well that failed, so refresh lists and try again!
    if(!ok)
    {
        KFI_DBUG << "refresh font list";
        entry.clear();
        clearFontList();
        updateFontList();
        ok=createStatEntryReal(entry, url, folder) && getSize(entry)>0;
    }

    // Perhaps its not a valid font? Try to stat on location+name
    if(!ok)
    {
        KFI_DBUG << "could not find";

        QStringList folders;

        folders.append(itsFolders[folder].location);
        folders.append(getDestFolder(itsFolders[folder].location, url.fileName()));

        QStringList::Iterator it(folders.begin()),
                              end(folders.end());

        for(; it!=end; ++it)
            for(int t=0; t<3 && !ok; ++t)
            {
                CDisabledFonts::TFileList files;
                QString                   fileName=0==t
                                              ? url.fileName()
                                              : 1==t ? modifyName(url.fileName())  // lowercase
                                                     : modifyName(url.fileName(), true);  // uppercase

                files.append(CDisabledFonts::TFile((*it)+fileName));
                entry.clear();
                ok=createFontUDSEntry(entry, i18n("Invalid Font"), files,
                                      KFI_NO_STYLE_INFO, 0, FOLDER_SYS==folder,
                                      Misc::isHidden(url)) && getSize(entry)>0;
            }
    }

    if(!ok)
        entry.clear();

    return ok;
}

bool CKioFonts::createStatEntryReal(KIO::UDSEntry &entry, const KUrl &url, EFolder folder)
{
    KFI_DBUG << url.path();

    TFontMap::Iterator it=getMap(url);

    if(it!=itsFolders[folder].fontMap.end())
    {
        KFI_DBUG << "its a normal font";
        return createFontUDSEntry(entry, it.key(), it.value().files, it.value().styleVal,
                                  it.value().writingSystems, FOLDER_SYS==folder);
    }

    KFI_DBUG << "try looking in disabled fonts";

    QString                             name=Misc::getFile(removeMultipleExtension(url));
    CDisabledFonts::TFontList::Iterator dIt=itsFolders[folder].disabled->find(name,
                                            Misc::getIntQueryVal(url, KFI_KIO_FACE, 0));

    if(dIt!=itsFolders[folder].disabled->items().end())
    {
        KFI_DBUG << "its a disabled font";
        return createFontUDSEntry(entry, (*dIt).getName(), (*dIt).files, (*dIt).styleInfo,
                                  (*dIt).writingSystems, FOLDER_SYS==folder, true);
    }

    return false;
}

void CKioFonts::get(const KUrl &url)
{
    KFI_DBUG << url.path() << " query:" << url.query();

    bool                      thumb="1"==metaData("thumbnail");
    CDisabledFonts::TFileList srcFiles;

    // Any error will be logged in getSourceFiles
    if(updateFontList() && checkUrl(url) && getSourceFiles(url, srcFiles))
    {
        //
        // The thumbnail job always donwloads non-local files to /tmp/... and passes this file name to
        // the thumbnail creator. However, in the case of fonts which are split among many files, this
        // wont work. Therefore, when the thumbnail code asks for the font to donwload, just return
        // the family and style info for enabled fonts, and the filename for disabled fonts. This way
        // the font-thumbnail creator can read this and just ask Xft/fontconfig for the font data.
        if(thumb)
        {
            QByteArray         array;
            QTextStream        stream(&array, QIODevice::WriteOnly);
            EFolder            folder(getFolder(url));
            TFontMap::Iterator it(getMap(url)),
                               end(itsFolders[folder].fontMap.end());

            emit mimeType("text/plain");

            if(it==end)
            {
                //
                // OK, its a disabled font - if possible try to return the location of the font file
                // itself.
                QString                             name(Misc::getFile(removeMultipleExtension(url)));
                CDisabledFonts::TFontList::Iterator dIt(itsFolders[folder].disabled
                                                     ->find(name, Misc::getIntQueryVal(url,
                                                                  KFI_KIO_FACE, 0))),
                                                    dEnd(itsFolders[folder].disabled->items().end());
                bool                                found=false;

                if(dIt!=dEnd)
                {
                    CDisabledFonts::TFileList::ConstIterator fIt((*dIt).files.begin()),
                                                             fEnd((*dIt).files.end());

                    for(; fIt!=fEnd; ++fIt)
                        if(isScalable((*fIt).path))
                        {
                            KFI_DBUG << "hasMetaData(\"thumbnail\"), so return FILE: "
                                     << (*fIt).path << " / " << (*fIt).face;
                            stream << KFI_PATH_KEY << (*fIt).path << endl
                                   << KFI_FACE_KEY << (*fIt).face << endl;
                            found=true;
                            break;
                        }
                }

                if(!found)
                {
                    KFI_DBUG << "hasMetaData(\"thumbnail\"), so return Url: " << url;
                    stream << url.prettyUrl();
                }
            }
            else
            {
                KFI_DBUG << "hasMetaData(\"thumbnail\"), so return DETAILS: " << it.key() << " / "
                         << (*it).styleVal;

                stream << KFI_NAME_KEY << it.key() << endl
                       << KFI_STYLE_KEY << (*it).styleVal << endl;

                if(1==(*it).files.count())
                {
                    CDisabledFonts::TFileList::ConstIterator fIt((*it).files.begin());

                    stream << KFI_PATH_KEY << (*fIt).path << endl
                           << KFI_FACE_KEY << (*fIt).face << endl;
                }
            }

            totalSize(array.size());
            data(array);
            processedSize(array.size());
            data(QByteArray());
            processedSize(array.size());
            finished();
            KFI_DBUG << "Finished thumbnail...";
            return;
        }

        QString         realPath;
        KDE_struct_stat buff;
        bool            multiple=false;

        if(1==srcFiles.count())
            realPath=srcFiles.first().path;
        else   // Font is made up of multiple files - so create .zip of them all!
        {
            KTemporaryFile tmpFile;

            if(tmpFile.open())
            {
                KZip zip(tmpFile.fileName());

                tmpFile.setAutoRemove(false);
                realPath=tmpFile.fileName();

                if(zip.open(QIODevice::WriteOnly))
                {
                    QMap<QString, QString> map;

                    getFontList(srcFiles, map);

                    QMap<QString, QString>::Iterator fIt(map.begin()),
                                                    fEnd(map.end());

                    //
                    // Iterate through created list, and add to zip archive
                    // ...unhide any hidden files
                    for(; fIt!=fEnd; ++fIt)
                        zip.addLocalFile(fIt.key(), Misc::unhide(fIt.value()));

                    multiple=true;
                    zip.close();
                }
            }
        }

        QByteArray realPathC(QFile::encodeName(realPath));
        KFI_DBUG << "real: " << realPathC;

        if (-2==KDE_stat(realPathC.constData(), &buff))
            error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST, urlString(url));
        else if (S_ISDIR(buff.st_mode))
            error(KIO::ERR_IS_DIRECTORY, urlString(url));
        else if (!S_ISREG(buff.st_mode))
            error(KIO::ERR_CANNOT_OPEN_FOR_READING, urlString(url));
        else
        {
            int fd = KDE_open(realPathC.constData(), O_RDONLY);

            if (fd < 0)
                error(KIO::ERR_CANNOT_OPEN_FOR_READING, urlString(url));
            else
            {
                // Determine the mimetype of the file to be retrieved, and emit it.
                // This is mandatory in all slaves (for KRun/BrowserRun to work).
                emit mimeType(KMimeType::findByPath(realPathC, buff.st_mode)->name());

                totalSize(buff.st_size);

                KIO::filesize_t processed=0;
                char            buffer[MAX_IPC_SIZE];
                QByteArray      array;

                while(1)
                {
                    int n=::read(fd, buffer, MAX_IPC_SIZE);

                    if (-1==n)
                    {
                        if (EINTR==errno)
                            continue;

                        error(KIO::ERR_COULD_NOT_READ, urlString(url));
                        ::close(fd);
                        if(multiple)
                            ::unlink(realPathC);
                        return;
                    }
                    if (0==n)
                        break; // Finished

                    array=array.fromRawData(buffer, n);
                    data(array);
                    array.clear();

                    processed+=n;
                    processedSize(processed);
                }

                data(QByteArray());
                ::close(fd);
                processedSize(buff.st_size);
                finished();
            }
        }
        if(multiple)
            ::unlink(realPathC);
    }
}

void CKioFonts::put(const KUrl &u, int mode, KIO::JobFlags flags)
{
    KFI_DBUG << u.path() << " query:" << u.query();

    if(Misc::isHidden(u))
    {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Cannot install %1\nHidden fonts cannot be "
                                           "installed.", urlString(u)));
        return;
    }

    // updateFontList(); // CPD: don't update font list upon a put - it's too slow. Just stat on
    //                   // filename!
    //checkUrl(u) // CPD: Don't need to check Url, as the call to "confirmUrl()" below will sort out
    //            // any probs!

    KUrl            url(u);

    correctUrl(url);

    bool            nrs(nonRootSys(url)),
                    clearList(!hasMetaData(KFI_KIO_NO_CLEAR));
    EFolder         destFolder(getFolder(url));
    QString         destFolderReal(getDestFolder(itsFolders[destFolder].location, url.fileName())),
                    dest(destFolderReal+modifyName(url.fileName()));
    QByteArray      destC(QFile::encodeName(dest));
    KDE_struct_stat buffDest;
    bool            destExists(KDE_lstat(destC.constData(), &buffDest)!= -1);

    if (destExists && !(flags & KIO::Overwrite) && !(flags & KIO::Resume))
    {
        error(KIO::ERR_FILE_ALREADY_EXIST, urlString(url));
        return;
    }

    if(nrs && !getRootPasswd(u)) // Need to check can get root passwd before start download...
    {
        error(KIO::ERR_ACCESS_DENIED, KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS);
        return;
    }

    //
    // As we don't get passed a mime-type the following needs to happen:
    //
    //    1. Download to a temporary file
    //    2. Check with FreeType that the file is a font, or that it is
    //       an AFM or PFM file
    KTemporaryFile tmpFile;

    tmpFile.setAutoRemove(true);

    if(putReal(tmpFile))
    {
        EFileType type(checkFile(tmpFile.fileName(), u));  // error logged in checkFile

        if(FILE_UNKNOWN==type)
            return;

        int timeout(reconfigTimeout());

        if(nrs)  // Ask root to move the tmp font...
        {
            QList<TCommand> cmd;
            TCommand        c(KFI::CMD_MOVE_FILE);

            addCreateFolderCmd(itsFolders[FOLDER_SYS].location, cmd);
            if(destFolderReal!=itsFolders[FOLDER_SYS].location)
                addCreateFolderCmd(destFolderReal, cmd);

            c.args.append(tmpFile.fileName());
            c.args.append(dest);
            c.args.append(0);
            c.args.append(0);

            cmd.append(c);

            // Get root to move this to fonts folder...
            if(doRootCmd(u, cmd, false)) // Already asked for passwd...
            {
                tmpFile.setAutoRemove(false);
                if(FILE_FONT==type)
                    modified(timeout, FOLDER_SYS, clearList, destFolderReal);
                createAfm(dest, true);
            }
            else
            {
                error(KIO::ERR_ACCESS_DENIED, KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS);
                return;
            }
        }
        else // Move it to our font folder...
        {
            tmpFile.setAutoRemove(false);
            if(!Misc::dExists(destFolderReal))
                Misc::createDir(destFolderReal);
            if(0==::rename(QFile::encodeName(tmpFile.fileName()).constData(), destC.constData()))
            {
                Misc::setFilePerms(destC);
                if(FILE_FONT==type)
                    modified(timeout, FOLDER_USER, clearList, destFolderReal);
                createAfm(dest);
            }
            else
            {
                error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.",
                                              i18n(KFI_KIO_FONTS_USER)));
                return;
            }
        }

        finished();
    }
}

QString CKioFonts::getUserName(uid_t uid)
{
    if (!itsUserCache.contains(uid))
    {
        struct passwd *user = getpwuid(uid);
        if(user)
            itsUserCache.insert(uid, QString::fromLatin1(user->pw_name));
        else
            return QString::number(uid);
    }
    return itsUserCache[uid];
}

QString CKioFonts::getGroupName(gid_t gid)
{
    if (!itsGroupCache.contains(gid))
    {
        struct group *grp = getgrgid(gid);
        if(grp)
            itsGroupCache.insert(gid, QString::fromLatin1(grp->gr_name));
        else
            return QString::number(gid);
    }
    return itsGroupCache[gid];
}

bool CKioFonts::createFontUDSEntry(KIO::UDSEntry &entry, const QString &name,
                                   const CDisabledFonts::TFileList &patterns,
                                   quint32 styleVal, qulonglong writingSystems,
                                   bool sys, bool hidden)
{
    //KFI_DBUG << "name:" << name << " style:" << styleVal << " #"
    //         << patterns.count();

    //
    // First of all get list of real files - i.e. remove any duplicates due to symlinks
    CDisabledFonts::TFileList files;

    getFontFiles(patterns, files);

    CDisabledFonts::TFileList::ConstIterator it(files.begin()),
                                             end(files.end());

    if(files.count()>1)
    {
        //
        // Sort list of files - placing scalable ones first. This is because, when determening the
        // mimetype, the 1st valid file will be chosen. In case of mixed bitmap/scalable - prefer
        // scalable
        CDisabledFonts::TFileList sorted;

        for(; it!=end; ++it)
            if(isScalable(*it))
                sorted.prepend(*it);
            else
                sorted.append(*it);
        files=sorted;
        it=files.begin();
        end=files.end();
    }

    entry.clear();
    entry.insert(KIO::UDSEntry::UDS_SIZE, getSize(files));

    for(; it!=end; ++it)
    {
        QByteArray      cPath(QFile::encodeName(*it));
        KDE_struct_stat buff;

        if(-1!=KDE_lstat(cPath, &buff))
        {
            if(0==writingSystems)
                writingSystems=toBit(QFontDatabase::Other);

            entry.insert(KIO::UDSEntry::UDS_NAME, name);
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, buff.st_mode&S_IFMT);
            entry.insert(KIO::UDSEntry::UDS_ACCESS, buff.st_mode&07777);
            entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, buff.st_mtime);
            entry.insert(KIO::UDSEntry::UDS_USER, getUserName(buff.st_uid));
            entry.insert(KIO::UDSEntry::UDS_GROUP, getGroupName(buff.st_gid));
            entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, buff.st_atime);
            entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, obtainMimeType(*it));
            entry.insert(UDS_EXTRA_FC_STYLE, styleVal);
            entry.insert(UDS_EXTRA_WRITING_SYSTEMS, writingSystems);

            if(hidden)
                entry.insert(KIO::UDSEntry::UDS_HIDDEN, 1);

            entry.insert(UDS_EXTRA_FILE_NAME, (*it).path);
            entry.insert(UDS_EXTRA_FOUNDRY, (*it).foundry);
            if(files.count()>1)
                entry.insert(UDS_EXTRA_FILE_LIST, files.toString(true));

            QString url(KFI_KIO_FONTS_PROTOCOL+QString::fromLatin1(":/"));

            if(!Misc::root())
            {
                url+=sys ? i18n(KFI_KIO_FONTS_SYS) : i18n(KFI_KIO_FONTS_USER);
                url+=QString::fromLatin1("/");
            }

#ifdef KFI_KIO_ALL_URLS_HAVE_NAME
            addKnownExtension(url, files, name, hidden);
#else
            if(files.count()>1)
            {
                if(hidden)
                    url+='.';
                url+=name+QString::fromLatin1(KFI_FONTS_PACKAGE);
            }
            else
                url+=Misc::getFile(*it);
#endif

            if(files.count()==1 && (*it).face>0)
            {
                KUrl kUrl(url);

                kUrl.setQuery("?"KFI_KIO_FACE"="+QString::number((*it).face));
                entry.insert(KIO::UDSEntry::UDS_URL, kUrl.url());
            }
            else
                entry.insert(KIO::UDSEntry::UDS_URL, url);

            return true;  // This file was OK, so use its values...
        }
    }
    return false;
}

bool CKioFonts::createFolderUDSEntry(KIO::UDSEntry &entry, const QString &name,
                                     const QString &path, bool sys)
{
    KFI_DBUG << "name:" << name << " path:" << path << " sys?:" << sys;

    KDE_struct_stat buff;
    QByteArray      cPath(QFile::encodeName(path));

    entry.clear();

    if(-1!=KDE_lstat(cPath, &buff))
    {
        entry.insert(KIO::UDSEntry::UDS_NAME, name);

        if (S_ISLNK(buff.st_mode))
        {
            KFI_DBUG << path << " is a link";

            char buffer2[1000];
            int n=readlink(cPath, buffer2, 1000);
            if(n!= -1)
                buffer2[n]='\0';

            if(-1==KDE_stat(cPath, &buff))
            {
                // It is a link pointing to nowhere
                entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFMT - 1);
                entry.insert(KIO::UDSEntry::UDS_ACCESS, S_IRWXU | S_IRWXG | S_IRWXO);
                entry.insert(KIO::UDSEntry::UDS_SIZE, 0);
                goto notype;
            }
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR); // CPD Treat links as regular folder...
        }
        else
            entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, buff.st_mode&S_IFMT);
        entry.insert(KIO::UDSEntry::UDS_ACCESS, buff.st_mode&07777);
        entry.insert(KIO::UDSEntry::UDS_SIZE, buff.st_size);

        notype:
        entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, buff.st_mtime);
        entry.insert(KIO::UDSEntry::UDS_USER, getUserName(buff.st_uid));
        entry.insert(KIO::UDSEntry::UDS_GROUP, getGroupName(buff.st_gid));
        entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, buff.st_atime);
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
        return true;
    }
    else if (sys && !Misc::root())   // Default system fonts folder does not actually exist yet!
    {
        KFI_DBUG << "Default system folder (" << path
                 << ") does not yet exist, so create dummy entry";
        entry.insert(KIO::UDSEntry::UDS_NAME, name);
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.insert(KIO::UDSEntry::UDS_ACCESS, 0744);
        entry.insert(KIO::UDSEntry::UDS_USER, QString::fromLatin1("root"));
        entry.insert(KIO::UDSEntry::UDS_GROUP, QString::fromLatin1("root"));
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
        return true;
    }

    return false;
}

bool CKioFonts::putReal(KTemporaryFile &dest)
{
    if (!dest.open())
    {
        error(EACCES==errno ? KIO::ERR_WRITE_ACCESS_DENIED : KIO::ERR_CANNOT_OPEN_FOR_WRITING, dest.fileName());
        return false;
    }

    int result;
    // Loop until we got 0 (end of data)
    do
    {
        QByteArray buffer;

        dataReq(); // Request for data
        result = readData(buffer);
        if(result > 0 && !writeAll(dest.handle(), buffer.constData(), buffer.size()))
        {
            if(ENOSPC==errno) // disk full
            {
                error(KIO::ERR_DISK_FULL, dest.fileName());
                result = -2; // means: remove dest file
            }
            else
            {
                error(KIO::ERR_COULD_NOT_WRITE, dest.fileName());
                result = -1;
            }
        }
    }
    while(result>0);

    if (result<0)
    {
        dest.close();
        ::exit(255);
    }

    return true;
}

void CKioFonts::copy(const KUrl &src, const KUrl &d, int mode, KIO::JobFlags flags)
{
    //
    // Support:
    //    Copying to fonts:/
    //    Copying from fonts:/ and file:/
    //
    KFI_DBUG << src.prettyUrl() << " query:" << src.query() << " - "
             << d.prettyUrl() << " query:" << d.query();

    if(Misc::isHidden(d))
    {
        error(KIO::ERR_SLAVE_DEFINED,
              i18n("Cannot copy %1 to %2\nHidden/disabled fonts cannot be installed.",
                   urlString(src), urlString(d)));
        return;
    }

    bool fromFonts=KFI_KIO_FONTS_PROTOCOL==src.protocol();

    // CPD: don't update font list upon a copy from file - it's too slow. Just stat on filename!
    if(!fromFonts || updateFontList() && checkUrl(src) && checkAllowed(src))
    {
        //checkUrl(u) // CPD as per comment in ::put()

        CDisabledFonts::TFileList srcFiles;
        int                       timeout(reconfigTimeout());

        if(getSourceFiles(src, srcFiles))  // Any error will be logged in getSourceFiles
        {
            KUrl dest(d);

            correctUrl(dest);

            bool                   metrics(fromFonts ? false : Misc::isMetrics(src.fileName())),
                                   clearList(!hasMetaData(KFI_KIO_NO_CLEAR));
            EFolder                destFolder(getFolder(dest));
            QMap<QString, QString> map;

            if(!fromFonts)
                map[src.path()]=src.fileName();

            // As above, if copying from file, then only stat on dest filename, but if from fonts to
            // fonts need to get the list of possible source files, etc.
            if(fromFonts ? confirmMultiple(src, srcFiles,
                                   FOLDER_SYS==destFolder ? FOLDER_USER : FOLDER_SYS, OP_COPY) &&
                           getFontList(srcFiles, map) &&
                           checkDestFiles(src, map, dest, destFolder, flags)
                         : checkDestFile(src, dest, destFolder, flags) )
            {
                if(nonRootSys(dest))
                {
                    QList<TCommand>                  cmd;
                    int                              size=0;
                    CDirList                         addedFolders;
                    QMap<QString, QString>::Iterator fIt(map.begin()),
                                                     fEnd(map.end());

                    for(; fIt!=fEnd; ++fIt)
                    {
                        TCommand c(KFI::CMD_COPY_FILE);
                        QString destFolderReal(getDestFolder(itsFolders[destFolder].location,
                                                             Misc::getFile(*fIt)));

                        if(!addedFolders.contains(itsFolders[FOLDER_SYS].location) &&
                           addCreateFolderCmd(itsFolders[FOLDER_SYS].location, cmd))
                            addedFolders.add(itsFolders[FOLDER_SYS].location);

                        if(!addedFolders.contains(destFolderReal) &&
                           addCreateFolderCmd(destFolderReal, cmd))
                            addedFolders.add(destFolderReal);

                        c.args.append(fIt.key());
                        c.args.append(destFolderReal+modifyName(fIt.value()));
                        cmd.append(c);

                        int s=getSize(QFile::encodeName(fIt.key()));
                        if(s>0)
                            size+=s;
                    }

                    totalSize(size);

                    if(doRootCmd(d, cmd, true))
                    {
                        if(!metrics)
                            modified(timeout, destFolder, clearList, addedFolders);
                        processedSize(size);
                        if(src.isLocalFile() && 1==srcFiles.count())
                            createAfm(itsFolders[destFolder].location+modifyName(map.begin().value()), true);
                    }
                    else
                    {
                        error(KIO::ERR_ACCESS_DENIED, KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS);
                        return;
                    }
                }
                else
                {
                    QMap<QString, QString>::Iterator fIt(map.begin()),
                                                     fEnd(map.end());
                    QString                          destFolderReal;

                    for(; fIt!=fEnd; ++fIt)
                    {
                        destFolderReal=getDestFolder(itsFolders[destFolder].location,
                                                     Misc::getFile(*fIt));

                        QByteArray      realSrc(QFile::encodeName(fIt.key())),
                                        realDest(QFile::encodeName(destFolderReal+
                                                 modifyName(fIt.value())));
                        KDE_struct_stat buffSrc;

                        if(-1==KDE_stat(realSrc.constData(), &buffSrc))
                        {
                            error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST,
                                  urlString(src));
                            return;
                        }

                        int srcFd=KDE_open(realSrc.constData(), O_RDONLY);

                        if (srcFd<0)
                        {
                            error(KIO::ERR_CANNOT_OPEN_FOR_READING, urlString(src));
                            return;
                        }

                        if(!Misc::dExists(destFolderReal))
                            Misc::createDir(destFolderReal);

                        // WABA: Make sure that we keep writing permissions ourselves,
                        // otherwise we can be in for a surprise on NFS.
                        int destFd=KDE_open(realDest.constData(), O_CREAT | O_TRUNC | O_WRONLY,
                                            -1==mode ? 0666 : mode | S_IWUSR);

                        if (destFd<0)
                        {
                            error(EACCES==errno
                                      ? KIO::ERR_WRITE_ACCESS_DENIED
                                      : KIO::ERR_CANNOT_OPEN_FOR_WRITING, urlString(dest));
                            ::close(srcFd);
                            return;
                        }

                        totalSize(buffSrc.st_size);

                        KIO::filesize_t processed = 0;
                        char            buffer[MAX_IPC_SIZE];
                        QByteArray      array;

                        while(1)
                        {
                            int n=::read(srcFd, buffer, MAX_IPC_SIZE);

                            if(-1==n)
                            {
                                error(KIO::ERR_COULD_NOT_READ, urlString(src));
                                ::close(srcFd);
                                ::close(destFd);
                                return;
                            }
                            if(0==n)
                                break; // Finished

                           if(!writeAll(destFd, buffer, n))
                           {
                                ::close(srcFd);
                                ::close(destFd);
                                if (ENOSPC==errno) // disk full
                                {
                                    error(KIO::ERR_DISK_FULL, urlString(dest));
                                    remove(realDest.constData());
                                }
                                else
                                    error(KIO::ERR_COULD_NOT_WRITE, urlString(dest));
                                return;
                            }

                            processed += n;
                            processedSize(processed);
                        }

                        ::close(srcFd);

                        if(::close(destFd))
                        {
                            error(KIO::ERR_COULD_NOT_WRITE, urlString(dest));
                            return;
                        }

                        Misc::setFilePerms(realDest);

                        // copy access and modification time
                        struct utimbuf ut;

                        ut.actime = buffSrc.st_atime;
                        ut.modtime = buffSrc.st_mtime;
                        ::utime(realDest.constData(), &ut);

                        processedSize(buffSrc.st_size);
                        if(!metrics)
                            modified(timeout, destFolder, clearList, destFolderReal);
                    }

                    if(src.isLocalFile() && 1==srcFiles.count())
                        createAfm(destFolderReal+modifyName(map.begin().value()));
                }

                finished();
            }
        }
    }
}

void CKioFonts::rename(const KUrl &src, const KUrl &d, KIO::JobFlags flags)
{
    KFI_DBUG << src.prettyUrl() << " query:" << src.query() << " - "
             << d.prettyUrl() << " query:" << d.query() << ", " << (flags & KIO::Overwrite);

    int timeout(reconfigTimeout());

    if(src.directory()==d.directory())
    {
        if(!itsRoot && "/"==src.directory())
        {
            error(KIO::ERR_SLAVE_DEFINED, i18n("You cannot rename font folders"));
            return;
        }

        CDisabledFonts::TFontList::Iterator disabledIt;
        TFontMap::Iterator                  enabledIt;
        const CDisabledFonts::TFileList     *entries(getEntries(src, enabledIt, disabledIt));

        if(!entries)
        {
            KFI_DBUG << "No entries found, updating font list antry again...";
            updateFontList();
            entries=getEntries(src, enabledIt, disabledIt);
        }

        if(entries)
        {
            QString destFile(Misc::getFile(d.path())),
                    srcFile(Misc::getFile(src.path())),
                    destEn(destFile.mid(1)),
                    srcEn(srcFile.mid(1));
            EFolder folder(getFolder(d));
            QString srcName(Misc::getFile(removeMultipleExtension(src)));
            bool    clearList(!hasMetaData(KFI_KIO_NO_CLEAR)),
                    nrs(nonRootSys(src)),
                    enable(Misc::isHidden(srcFile) && !Misc::isHidden(destFile) && srcEn==destFile),
                    disable(!Misc::isHidden(srcFile) && Misc::isHidden(destFile) &&
                            destEn==srcFile);

            if(enable && disabledIt!=itsFolders[folder].disabled->items().end())
            {
                if(confirmMultiple(src, (*disabledIt).files, folder, OP_ENABLE))
                {
                    CDirList                                 folders;
                    CDisabledFonts::TFileList::ConstIterator it((*disabledIt).files.begin()),
                                                             end((*disabledIt).files.end());
                    bool                                     ok(false);

                    for(; it!=end; ++it)
                        folders.add(Misc::getDir(*it));

                    if(nrs)
                    {
                        TCommand c(KFI::CMD_ENABLE_FONT);

                        c.args.append((*disabledIt).family);
                        c.args.append((int)(*disabledIt).styleInfo);
                        ok=doRootCmd(d, c);
                    }
                    else
                        ok=itsFolders[folder].disabled->enable(disabledIt);

                    if(ok)
                    {
                        modified(timeout, folder, clearList, folders);
                        finished();
                    }
                    else
                        if(nrs)
                            error(KIO::ERR_ACCESS_DENIED, KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS);
                        else
                            error(KIO::ERR_DOES_NOT_EXIST, urlString(src));
                }
                return;
            }
            else if (disable && disabledIt==itsFolders[folder].disabled->items().end())
            {
                if(confirmMultiple(src, (*enabledIt).files, folder, OP_DISABLE))
                {
                    CDirList                                 folders;
                    QMap<int, QString>                       names;
                    CDisabledFonts::TFileList::ConstIterator it((*enabledIt).files.begin()),
                                                             end((*enabledIt).files.end());
                    bool                                     ok=false;

                    for(; it!=end; ++it)
                        folders.add(Misc::getDir(*it));

                    //
                    // If there is only 1 file mapped to this fontname, see if this file maps
                    // to multiple font names - as would be the case in a TTC file...
                    if(1==(*enabledIt).files.count())
                        names=getFontIndexToNameEntries(folder, (*((*enabledIt).files.begin())).path);

                    if(0==names.count())
                        names[0]=enabledIt.key();  // Multiple files -> cant use index :-(

                    QMap<int, QString>::ConstIterator nameIt(names.begin()),
                                                      nameEnd(names.end());

                    for(; nameIt!=nameEnd; ++nameIt)
                        if(nrs)
                        {
                            TCommand c(KFI::CMD_DISABLE_FONT);

                            c.args.append(getFamily(enabledIt.key()));
                            c.args.append((int)(*enabledIt).styleVal);
                            c.args.append((*enabledIt).writingSystems);
                            c.args.append((int)(nameIt.key()));
                            c.args.append((*enabledIt).files.count());
                            for(it=(*enabledIt).files.begin(); it!=end; ++it)
                            {
                                c.args.append((*it).path);
                                c.args.append((*it).foundry);
                            }
                            ok=doRootCmd(d, c);
                        }
                        else
                        {
                            QString               fontStr(*nameIt);
                            int                   commaPos=fontStr.indexOf(',');
                            CDisabledFonts::TFont font(-1==commaPos
                                                        ? fontStr
                                                        : fontStr.left(commaPos),
                                                       (*enabledIt).styleVal,
                                                       (*enabledIt).writingSystems);

                            font.files=(*enabledIt).files;
                            if(1==font.files.count())
                                (*(font.files.begin())).face=nameIt.key();
                            ok=itsFolders[folder].disabled->disable(font);
                        }

                    if(ok)
                    {
                        modified(timeout, folder, clearList, folders);
                        finished();
                    }
                    else
                        if(nrs)
                            error(KIO::ERR_ACCESS_DENIED, KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS);
                        else
                            error(KIO::ERR_DOES_NOT_EXIST, urlString(src));
                    return;
                }
                error(KIO::ERR_SLAVE_DEFINED, i18n("Internal error - could not find font."));
                return;
            }
            else if(enable || disable)
                error(KIO::ERR_SLAVE_DEFINED, enable
                                                  ? i18n("Could not enable %1\n"
                                                         "An enabled font already exists, please delete the disabled one.",
                                                         urlString(src))
                                                  : i18n("Could not disable %1\n"
                                                         "A disabled font already exists, please delete the enabled one.",
                                                         urlString(src)));
            else
                error(KIO::ERR_SLAVE_DEFINED, i18n("Sorry, fonts cannot be renamed."));

            return;
        }
        error(KIO::ERR_DOES_NOT_EXIST, urlString(src));
        return;
    }
    else if(itsRoot) // Should never happen...
    {
        error(KIO::ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, KIO::CMD_RENAME));
        return;
    }
    else if(Misc::isHidden(src) || Misc::isHidden(d))
    {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Cannot move %1 to %2\nDisabled fonts cannot be moved.",
                                           urlString(src), urlString(d)));
        return;
    }
    else
    {
        //
        // Can't rename from/to file:/ -> therefore rename can only be from fonts:/System to
        // fonts:/Personal, or vice versa.
        CDisabledFonts::TFileList srcFiles;

        if(getSourceFiles(src, srcFiles))   // Any error will be logged in getSourceFiles
        {
            KUrl dest(d);

            correctUrl(dest);

            EFolder                destFolder(getFolder(dest));
            QMap<QString, QString> map;

            if(confirmMultiple(src, srcFiles, FOLDER_SYS==destFolder ? FOLDER_USER : FOLDER_SYS, OP_MOVE) &&
               getFontList(srcFiles, map) && checkDestFiles(src, map, dest, destFolder, flags))
            {
                QMap<QString, QString>::Iterator fIt(map.begin()),
                                                 fEnd(map.end());
                bool                             askPasswd=true,
                                                 toSys=FOLDER_SYS==destFolder;

                for(; fIt!=fEnd; ++fIt)
                {
                    QString         destFolderReal(getDestFolder(itsFolders[destFolder].location, fIt.value()));
                    QList<TCommand> cmd;
                    TCommand        c(KFI::CMD_MOVE_FILE);

                    if(toSys)
                    {
                        addCreateFolderCmd(itsFolders[FOLDER_SYS].location, cmd);
                        if(destFolderReal!=itsFolders[FOLDER_SYS].location)
                            addCreateFolderCmd(destFolderReal, cmd);
                    }

                    c.args.append(fIt.key());
                    c.args.append(destFolderReal+fIt.value());
                    c.args.append((int)(toSys ? 0 : getuid()));
                    c.args.append((int)(toSys ? 0 : getgid()));
                    cmd.append(c);

                    QString sysDir,
                            userDir;

                    if(FOLDER_SYS==destFolder)
                    {
                        sysDir=destFolderReal;
                        userDir=Misc::getDir(fIt.key());
                    }
                    else
                    {
                        userDir=destFolderReal;
                        sysDir=Misc::getDir(fIt.key());
                    }

                    if(doRootCmd(toSys ? d : src, cmd, askPasswd))
                    {
                        modified(timeout, FOLDER_SYS, true, sysDir);
                        modified(timeout, FOLDER_USER, true, userDir);
                        askPasswd=false;  // Don't keep on asking for password...
                    }
                    else
                    {
                        error(KIO::ERR_ACCESS_DENIED, KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS);
                        return;
                    }
                }
            }
        }
    }
    finished();
}

void CKioFonts::del(const KUrl &url, bool)
{
    KFI_DBUG << url.path() << " query:" << url.query();

    const CDisabledFonts::TFileList     *entries;
    CDisabledFonts::TFontList::Iterator disabledIt;
    TFontMap::Iterator                  enabledIt;

    if(checkUrl(url) && checkAllowed(url) &&
       (( (entries=getEntries(url, enabledIt, disabledIt)) && entries->count() && checkFiles(*entries))  ||
        ( updateFontList() && (entries=getEntries(url, enabledIt, disabledIt)) && entries->count() &&
       checkFiles(*entries))) && confirmMultiple(url, entries, getFolder(url), OP_DELETE))
    {
        CDisabledFonts::TFileList::ConstIterator it,
                                                 end(entries->end());
        CDirList                                 modifiedDirs;
        bool                                     clearList(!hasMetaData(KFI_KIO_NO_CLEAR));
        int                                      timeout(reconfigTimeout());

        if(nonRootSys(url))
        {
            if(disabledIt!=itsFolders[FOLDER_SYS].disabled->items().end())
            {
                TCommand c(KFI::CMD_DELETE_DISABLED_FONT);

                c.args.append((*disabledIt).family);
                c.args.append((int)(*disabledIt).styleInfo);

                if(doRootCmd(url, c))
                    itsFolders[FOLDER_SYS].disabled->refresh();
                else
                {
                    error(KIO::ERR_ACCESS_DENIED, KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS);
                    return;
                }
            }
            else
            {
                QList<TCommand> cmd;

                for(it=entries->begin(); it!=end; ++it)
                {
                    modifiedDirs.add(Misc::getDir(*it));
                    cmd.append(TCommand(KFI::CMD_DELETE_FILE, (*it).path));

                    QStringList files;

                    Misc::getAssociatedFiles(*it, files);

                    if(files.count())
                    {
                        QStringList::Iterator fIt,
                                              fEnd=files.end();

                        for(fIt=files.begin(); fIt!=fEnd; ++fIt)
                            cmd.append(TCommand(KFI::CMD_DELETE_FILE, *fIt));
                    }
                }

                if(doRootCmd(url, cmd))
                    modified(timeout, FOLDER_SYS, clearList, modifiedDirs);
                else
                {
                    error(KIO::ERR_ACCESS_DENIED, KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS);
                    return;
                }
            }
        }
        else
        {
            for(it=entries->begin(); it!=end; ++it)
            {
                if (0!=unlink(QFile::encodeName(*it).constData()))
                    error(EACCES==errno || EPERM==errno
                            ? KIO::ERR_ACCESS_DENIED
                            : EISDIR==errno
                                    ? KIO::ERR_IS_DIRECTORY
                                    : KIO::ERR_CANNOT_DELETE,
                          *it);
                else
                {
                    modifiedDirs.add(Misc::getDir(*it));

                    QStringList files;

                    Misc::getAssociatedFiles(*it, files);

                    if(files.count())
                    {
                        QStringList::Iterator fIt,
                                              fEnd=files.end();

                        for(fIt=files.begin(); fIt!=fEnd; ++fIt)
                            unlink(QFile::encodeName(*fIt).constData());
                    }
                }
            }

            if(disabledIt!=itsFolders[itsRoot ? FOLDER_SYS : FOLDER_USER].disabled->items().end())
            {
                itsFolders[itsRoot ? FOLDER_SYS : FOLDER_USER].disabled->remove(disabledIt);
                itsFolders[itsRoot ? FOLDER_SYS : FOLDER_USER].disabled->refresh();
            }
            else
                modified(timeout, itsRoot ? FOLDER_SYS : FOLDER_USER, clearList, modifiedDirs);
        }
        finished();
    }
    else if(isATtc(QFile::encodeName(url.fileName())))
        finished();
    else
        error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\".", urlString(url)));
}

void CKioFonts::modified(int timeout, EFolder folder, bool clearList, const CDirList &dirs)
{
    KFI_DBUG << "timout:" << timeout << " folder:" << (int)folder << " clearList:" << clearList;

    if(dirs.count())
    {
        CDirList::ConstIterator it(dirs.begin()),
                                end(dirs.end());

        for(; it!=end; ++it)
            itsFolders[folder].modified.add(*it);
    }
    else
        itsFolders[folder].modified.add(itsFolders[folder].location);
    setTimeoutSpecialCommand(timeout ? timeout : -1);

    if(clearList)
        clearFontList();  // List of fonts has changed.../
}

void CKioFonts::special(const QByteArray &a)
{
    KFI_DBUG;

    if(a.size())
    {
        QDataStream stream(a);
        int         cmd;

        stream >> cmd;

        switch (cmd)
        {
            case SPECIAL_RESCAN:
                clearFontList();
                updateFontList();
                finished();
                break;
            case SPECIAL_CONFIGURE:
            {
                if(itsRoot)
                {
                    KUrl url;

                    stream >> url;

                    if(url.isValid())
                        itsFolders[FOLDER_SYS].disabled->reload();
                    if(0==itsFolders[FOLDER_SYS].modified.count())
                        configure(FOLDER_SYS);
                }
                else
                    for(;;)
                    {
                        KUrl url;

                        stream >> url;

                        if(url.isEmpty() || !url.isValid())
                            break;

                        QString sect(getSect(url.path()));

                        if(isSysFolder(sect))
                        {
                            itsFolders[FOLDER_SYS].disabled->reload();
                            if(0==itsFolders[FOLDER_SYS].modified.count())
                                configure(FOLDER_SYS);
                        }
                        else if(isUserFolder(sect))
                        {
                            itsFolders[FOLDER_USER].disabled->reload();
                            if(0==itsFolders[FOLDER_USER].modified.count())
                                configure(FOLDER_USER);
                        }
                    }

                if(itsFolders[FOLDER_USER].disabled->modified())
                    itsFolders[FOLDER_USER].disabled->reload();
                doModified();
                clearFontList();
                updateFontList();
                finished();
                break;
            }
            default:
                error(KIO::ERR_UNSUPPORTED_ACTION, QString::number(cmd));
        }
    }
    else
        doModified();
}

bool CKioFonts::configure(EFolder folder)
{
    bool refreshX(false);

    if(!itsRoot && FOLDER_SYS==folder)
    {
        QList<TCommand> cmd;

        if(itsAddToSysFc)
        {
            itsAddToSysFc=false;
                cmd.append(TCommand(KFI::CMD_ADD_DIR_TO_FONTCONFIG, itsFolders[FOLDER_SYS].location));
        }

        if(itsFolders[FOLDER_SYS].modified.count())
        {
            CDirList::ConstIterator it(itsFolders[FOLDER_SYS].modified.begin()),
                                    end(itsFolders[FOLDER_SYS].modified.end());

            for(; it!=end; ++it)
                if(Misc::fExists((*it)+"fonts.dir"))
                {
                        cmd.append(TCommand(KFI::CMD_CFG_DIR_FOR_X, *it));
                        refreshX=true;
                }
        }

        cmd.append(TCommand(KFI::CMD_FC_CACHE));
        doRootCmd(KUrl(), cmd, false);
    }
    else
    {
        Misc::doCmd(FC_CACHE_CMD);
        KFI_DBUG << "RUN: " << FC_CACHE_CMD;

        itsFolders[folder].disabled->save();

        CDirList::ConstIterator it(itsFolders[folder].modified.begin()),
                                end(itsFolders[folder].modified.end());

        for(; it!=end; ++it)
            if(Misc::fExists((*it)+"fonts.dir"))
            {
                Misc::configureForX11(*it);
                refreshX=true;
            }
    }
    return refreshX;
}

void CKioFonts::doModified()
{
    KFI_DBUG;
    bool refreshX(false),
         clear(false);

    infoMessage(i18n("Configuring installed fonts..."));
    setTimeoutSpecialCommand(-1); // Cancel timer

    if(itsFolders[FOLDER_SYS].modified.count())
    {
        refreshX=configure(FOLDER_SYS);
        itsFolders[FOLDER_SYS].modified.clear();
        clear=true;
    }

    if(!itsRoot && itsFolders[FOLDER_USER].modified.count())
    {
        refreshX=configure(FOLDER_USER);
        itsFolders[FOLDER_USER].modified.clear();
        clear=true;
    }

    if(clear)
        clearFontList();
    if(refreshX)
        Misc::doCmd("xset", "fp", "rehash");
    infoMessage(QString());
    KFI_DBUG << "finished";
}

bool CKioFonts::getRootPasswd(const KUrl &url, bool askPasswd)
{
    KFI_DBUG;

    if(url.hasUser() && !url.pass().isEmpty() && KFI_SYS_USER==url.user())
    {
        itsPasswd=url.pass();
        return !itsPasswd.isEmpty();
    }

    if(!askPasswd)
        return !itsPasswd.isEmpty();

    KIO::AuthInfo authInfo;
    SuProcess     proc(KFI_SYS_USER);
    bool          error=false;
    int           attempts=0;
    QString       errorMsg;

    authInfo.url=KUrl(KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS"/");
    authInfo.keepPassword=false;
    authInfo.caption=i18n("Authorization Required");
    authInfo.username=i18n(KFI_AUTHINF_USER);

    if(proc.useUsersOwnPassword())
        authInfo.prompt=i18n("The requested action requires administrator privileges.\n"
                             "If you have these privileges then please enter your password.");
    else
        authInfo.prompt=i18n("The requested action requires administrator privileges.\n"
                             "Please enter the system administrator's password.");

    if(!checkCachedAuthentication(authInfo) && !askPasswd)
        authInfo.password=itsPasswd;

    if(askPasswd)
    {
        while(!error && 0!=proc.checkInstall(authInfo.password.toLocal8Bit()))
        {
            KFI_DBUG << "ATTEMPT : " << attempts;
            if(1==attempts++)
                errorMsg=i18n("Incorrect password.\n");
            if(attempts>2 || !openPasswordDialog(authInfo, errorMsg))
                error=true;
        }
        if(!error && authInfo.keepPassword)
            cacheAuthentication(authInfo);
    }
    else
        error=proc.checkInstall(authInfo.password.toLocal8Bit()) ? true : false;

    itsPasswd= error ? QString() : authInfo.password;
    return !itsPasswd.isEmpty();
}

void CKioFonts::quitHelper()
{
    if(itsServer.isOpen() && itsSuProc && itsSocket && itsSuProc->isRunning())
    {
        KFI_DBUG;
        if(itsSocket->write(QVariant((int)KFI::CMD_QUIT)))
        {
            bool res;
            if(itsSocket->read(res, 10) && res)
            {
                itsSuProc->terminate();
                itsSuProc->wait(100);
            }
        }
    }
}

bool CKioFonts::doRootCmd(const KUrl &url, QList<TCommand> &cmd, bool askPasswd)
{
    KFI_DBUG;

    if(cmd.count() && getRootPasswd(url, askPasswd))
    {
        if(!itsServer.isOpen())
        {
            KFI_DBUG << "Open server socket";
            // Open socket for communication with helper app...
            itsServer.open();
        }

        if(itsServer.isOpen())
        {
            if(itsSuProc && !itsSuProc->isRunning())
            {
                KFI_DBUG << "Delete client socket";
                delete itsSocket;
                itsSocket=NULL;
                delete itsSuProc;
                itsSuProc=NULL;
            }

            if(!itsSuProc)
            {
                // Start helper app...
                KFI_DBUG << "Start helper...";
                itsSuProc=new CSuProc(itsServer.name(), itsPasswd);
                itsSuProc->start();
            }

            if(!itsSocket)
            {
                // Wait for helper app to connect...
                KFI_DBUG << "Wait for client...";
                itsSocket=itsServer.waitForClient();
            }

            if(itsSocket)
            {
                // Write commands to helper, and wait for replies...
                QList<TCommand>::ConstIterator it(cmd.begin()),
                                               end(cmd.end());
                bool                           commsError(false);

                for(; it!=end && !commsError; ++it)
                {
                    KFI_DBUG << "Send command #" << (*it).cmd;

                    if(itsSocket->write(QVariant((int)(*it).cmd)))
                    {
                        QList<QVariant>::ConstIterator argIt((*it).args.begin()),
                                                       argEnd((*it).args.end());

                        for(; argIt!=argEnd && !commsError; ++argIt)
                            if(!itsSocket->write(*argIt))
                            {
                                KFI_DBUG << "Failed to write arg";
                                commsError=true;
                            }

                        if(!commsError) // Wait for response!
                        {
                            bool res;

                            if(itsSocket->read(res, CMD_FC_CACHE==(*it).cmd ||
                                                    CMD_CFG_DIR_FOR_X==(*it).cmd
                                                        ? 600  // fc-cache can take a *long* time...
                                                        : 10))
                            {
                                if(!res &&
                                   CMD_ADD_DIR_TO_FONTCONFIG!=(*it).cmd &&
                                   CMD_CFG_DIR_FOR_X!=(*it).cmd)
                                {
                                    KFI_DBUG << "Command failed :-(";
                                    return false;
                                }
                            }
                            else
                            {
                                KFI_DBUG << "Failed to read response";
                                commsError=true;
                            }
                        }
                    }
                    else
                    {
                        KFI_DBUG << "Failed to write command id";
                        commsError=true;
                    }
                }

                if(!commsError)
                    return true;
                delete itsSocket;
                itsSocket=NULL;
            }
            else
                KFI_DBUG << "No socket connection :-(";
        }
    }

    return false;
}

bool CKioFonts::doRootCmd(const KUrl &url, const TCommand &cmd, bool askPasswd)
{
    QList<TCommand> cmds;

    cmds.append(cmd);
    return doRootCmd(url, cmds, askPasswd);
}

void CKioFonts::correctUrl(KUrl &url)
{
    KFI_DBUG << url.path();
    if(!itsRoot)
    {
        QString sect(getSect(url.path()));

        if(!isSysFolder(sect) && !isUserFolder(sect))
        {
            url.setPath(QChar('/')+i18n(KFI_KIO_FONTS_USER)+QChar('/')+url.fileName());
            KFI_DBUG << "Changed URL to:" << url.path();
        }
    }
}

void CKioFonts::clearFontList()
{
    KFI_DBUG;

    if(itsFontList)
        FcFontSetDestroy(itsFontList);

    itsFontList=NULL;
    itsFolders[FOLDER_SYS].fontMap.clear();
    if(!itsRoot)
        itsFolders[FOLDER_USER].fontMap.clear();
}

bool CKioFonts::updateFontList()
{
    KFI_DBUG;

    // For some reason just the "!FcConfigUptoDate(0)" check does not always work :-(
    if(0!=itsLastFcCheckTime &&
       (!itsFontList || !FcConfigUptoDate(0) ||
        (abs(time(NULL)-itsLastFcCheckTime)>constMaxFcCheckTime)))
    {
        KFI_DBUG << "itsFontList:" << (intptr_t)itsFontList
                 << " FcConfigUptoDate:" << (int)FcConfigUptoDate(0)
                 << " time diff:" << abs(time(NULL)-itsLastFcCheckTime)
                 << " max:" << constMaxFcCheckTime;
        FcInitReinitialize();
        clearFontList();
    }

    if(!itsRoot)
    {
        if(itsServer.isOpen() && itsSuProc && itsSocket)
            doRootCmd(KUrl(), TCommand(KFI::CMD_RELOAD_DISABLED_LIST), false);
        itsFolders[FOLDER_USER].disabled->refresh();
    }
    itsFolders[FOLDER_SYS].disabled->refresh();

    if(!itsFontList)
    {
        KFI_DBUG << "update list of fonts";

        itsLastFcCheckTime=time(NULL);

        FcPattern   *pat = FcPatternCreate();
        FcObjectSet *os  = FcObjectSetBuild(FC_FILE, FC_FAMILY,
#ifdef KFI_USE_TRANSLATED_FAMILY_NAME
                                            FC_FAMILYLANG,
#endif
                                            FC_WEIGHT, FC_LANG, FC_CHARSET,
                                            //FC_SCALABLE,
#ifndef KFI_FC_NO_WIDTHS
                                            FC_WIDTH,
#endif
                                            FC_SLANT, FC_INDEX, FC_FOUNDRY, (void*)0);

        itsFontList=FcFontList(0, pat, os);

        FcPatternDestroy(pat);
        FcObjectSetDestroy(os);

        if (itsFontList)
        {
            //
            // defoma (DEbian FOnt MAnanager) installs sym links into /var/lib/defoma/fontconfig.d, but also
            // places this folder into fontconfigs search path. Leading to duplicate font files. Therefore, just
            // ignore defoma's sym links...
            // -> Can't just ignore these, as if a font is disabled fontconfig will still list it, as it sees the symlink
            //static const char * constDefomaLocation="/var/lib/defoma/fontconfig.d";

            QString home(Misc::dirSyntax(QDir::homePath()));

            for (int i = 0; i < itsFontList->nfont; i++)
            {
                EFolder folder=FOLDER_SYS;
                QString fileName(Misc::fileSyntax(FC::getFcString(itsFontList->fonts[i], FC_FILE)));

                if(!fileName.isEmpty()) // && 0!=fileName.indexOf(constDefomaLocation))
                {
                    QString name,
                            foundry(FC::getFcString(itsFontList->fonts[i], FC_FOUNDRY));
                    quint32 styleVal;
                    int     index;

                    if(!itsRoot && 0==fileName.indexOf(home))
                        folder=FOLDER_USER;

                    FC::getDetails(itsFontList->fonts[i], name, styleVal, index);

                    TFontDetails &details=itsFolders[folder].fontMap[name];
                    bool         use=true;

                    details.styleVal=styleVal;
                    details.writingSystems|=getWritingSystems(itsFontList->fonts[i]);

                    if(details.files.count()) // Check for duplicates...
                    {
                        CDisabledFonts::TFileList::Iterator it,
                                                            end=details.files.end();

                        for(it=details.files.begin(); use && it!=end; ++it)
                            if(fileName==*it)
                                use=false;
                    }
                    if(use)
                        details.files.append(CDisabledFonts::TFile(fileName, index, foundry));
                }
            }
        }

        KFI_DBUG << "updated list of fonts";
    }

    if(NULL==itsFontList)
    {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Internal fontconfig error."));
        return false;
    }

    return true;
}

CKioFonts::EFolder CKioFonts::getFolder(const KUrl &url)
{
    return itsRoot || isSysFolder(getSect(url.path())) ? FOLDER_SYS : FOLDER_USER;
}

CKioFonts::TFontMap::Iterator CKioFonts::getMap(const KUrl &url)
{
    KFI_DBUG << url.prettyUrl();

    int                face(Misc::getIntQueryVal(url, KFI_KIO_FACE, 0));
    EFolder            folder(getFolder(url));
    TFontMap::Iterator it=itsFolders[folder].fontMap.find(removeMultipleExtension(url)),
                       end(itsFolders[folder].fontMap.end());

    if(it==end) // Perhaps it was fonts:/System/times.ttf ???
    {
        QString fName(Misc::getFile(url.path()));

        for(int t=0; t<3; ++t)
        {
            QString fileName=0==t
                                 ? fName
                                 : 1==t ? modifyName(fName)  // lowercase
                                        : modifyName(fName, true);  // uppercase

            KFI_DBUG << "look for " << fileName;

            for(it=itsFolders[folder].fontMap.begin(); it!=end; ++it)
            {
                CDisabledFonts::TFileList::Iterator sIt((*it).files.begin()),
                                                    sEnd((*it).files.end());

                for(;sIt!=sEnd; ++sIt)
                    if(Misc::getFile(*sIt)==fileName && (*sIt).face==face)
                        return it;
            }
        }
    }

    return it;
}

const CDisabledFonts::TFileList * CKioFonts::getEntries(const KUrl &url,
                                                        TFontMap::Iterator &enabledIt,
                                                        CDisabledFonts::TFontList::Iterator &disabledIt)
{
    KFI_DBUG << url.prettyUrl();

    EFolder                             folder=getFolder(url);
    TFontMap::Iterator                  it(getMap(url)),
                                        end(itsFolders[folder].fontMap.end());
    QString                             name=Misc::getFile(removeMultipleExtension(url));
    CDisabledFonts::TFontList::Iterator dIt(itsFolders[folder].disabled->find(name,
                                              Misc::getIntQueryVal(url, KFI_KIO_FACE, 0))),
                                        dEnd(itsFolders[folder].disabled->items().end());

    enabledIt=end;
    disabledIt=dEnd;

    if(it!=end && dIt==dEnd)
    {
        KFI_DBUG << "found enabled";
        enabledIt=it;
        return &(it.value().files);
    }
    else if (it==end && dIt!=dEnd)
    {
        disabledIt=dIt;
        KFI_DBUG << "found disabled";
        return &((*dIt).files);
    }
    else if(it!=end && dIt!=dEnd)
    {
        KFI_DBUG << "found both!";

        // Oops... we have a match for both a hidden, and non-hidden font! Have to ask which one...
        // This should never really happen, as hidden fonts will start with a period.
        if(KMessageBox::Yes==messageBox(QuestionYesNo,
                                  i18n("The selected URL (%1) matches both an enabled, and disabled "
                                       "font. Which one do you wish to access?", url.prettyUrl()),
                                  i18n("Duplicate Font"), i18n("Enabled Font"),
                                  i18n("Disabled Font")))
        {
            enabledIt=it;
            return &(it.value().files);
        }
        else
        {
            disabledIt=dIt;
            return &((*dIt).files);
        }
    }

    KFI_DBUG << "found none";
    return NULL;
}

QStringList CKioFonts::getFontNameEntries(EFolder folder, const QString &file, bool disabledFonts)
{
    QStringList rv;

    if(disabledFonts)
    {
       CDisabledFonts::TFontList::Iterator it(itsFolders[folder].disabled->items().begin()),
                                           end(itsFolders[folder].disabled->items().end());

        for(; it!=end; ++it)
        {
            CDisabledFonts::TFileList::ConstIterator patIt,
                                                     patEnd=(*it).files.end();

            for(patIt=(*it).files.begin(); patIt!=patEnd; ++patIt)
                if((*patIt).path==file)
                {
                    rv.append((*it).name);
                    break;
                }
        }
    }
    else
    {
        TFontMap::Iterator it,
                           end=itsFolders[folder].fontMap.end();

        for(it=itsFolders[folder].fontMap.begin(); it!=end; ++it)
        {
            CDisabledFonts::TFileList::ConstIterator patIt,
                                                     patEnd=it.value().files.end();

            for(patIt=it.value().files.begin(); patIt!=patEnd; ++patIt)
                if((*patIt).path==file)
                {
                    rv.append(it.key());
                    break;
                }
        }
    }
    return rv;
}

QMap<int, QString> CKioFonts::getFontIndexToNameEntries(EFolder folder, const QString &file)
{
    QMap<int, QString> rv;
    TFontMap::Iterator it,
                       end=itsFolders[folder].fontMap.end();

    for(it=itsFolders[folder].fontMap.begin(); it!=end; ++it)
    {
        CDisabledFonts::TFileList::Iterator patIt,
                                            patEnd=it.value().files.end();

        for(patIt=it.value().files.begin(); patIt!=patEnd; ++patIt)
            if((*patIt).path==file)
            {
                rv[(*patIt).face]=it.key();
                break;
            }
    }

    return rv;
}

QString * CKioFonts::getEntry(EFolder folder, const QString &file, bool full)
{
    TFontMap::Iterator it,
                       end=itsFolders[folder].fontMap.end();

    for(it=itsFolders[folder].fontMap.begin(); it!=end; ++it)
    {
        CDisabledFonts::TFileList::Iterator patIt,
                                            patEnd=it.value().files.end();

        for(patIt=it.value().files.begin(); patIt!=patEnd; ++patIt)
            if( (full && (*patIt).path==file) ||
                (!full && Misc::getFile(*patIt)==file))
                return &((*patIt).path);
    }

    return NULL;
}

CKioFonts::EFileType CKioFonts::checkFile(const QString &file, const KUrl &url)
{
    //
    // To speed things up, check the files extension 1st...
    if(Misc::checkExt(file, "bdf") || Misc::checkExt(file, "bdf.gz") ||
       Misc::checkExt(file, "pcf") || Misc::checkExt(file, "pcf.gz"))
    {
        // Need to check whether bitmaps have been hidden from from fontconfig - as happens on KUbuntu...
        if(FC::bitmapsEnabled())
            return FILE_FONT;
        else
            error(KIO::ERR_SLAVE_DEFINED, i18n("You cannot install bitmap fonts, as these have been "
                                               "disabled on your system."));
    }
    else if(isAAfm(file) || isAPfm(file))
        return FILE_METRICS;
    else if(Misc::isPackage(file))
        error(KIO::ERR_SLAVE_DEFINED, i18n("You cannot install a fonts package directly.\n"
                                           "Please extract %1, and install the components individually.",
                                           urlString(url)));
    else
    {
        //
        // Check that file is a font via FreeType...
        int       count=0;
        FcPattern *pat=FcFreeTypeQuery((const FcChar8 *)(QFile::encodeName(file).constData()), 0, NULL,
                                       &count);

        if(pat)
        {
            FcBool scalable;

            if(FcResultMatch==FcPatternGetBool(pat, FC_SCALABLE, 0, &scalable) && scalable)
            {
                // check too see whether font is already installed!
                int weight(KFI_NULL_SETTING), slant(KFI_NULL_SETTING), width(KFI_NULL_SETTING);

                FcPatternGetInteger(pat, FC_WEIGHT, 0, &weight);
                FcPatternGetInteger(pat, FC_SLANT, 0, &slant);
#ifndef KFI_FC_NO_WIDTHS
                FcPatternGetInteger(pat, FC_WIDTH, 0, &width);
#endif

                QString name(FC::createName(pat, weight, width, slant));

                KFI_DBUG << "Check for name:" << name;

                // TODO: CDisabledFonts need to find on family & style info? Also need a find() that does
                // not take into account face! Perhaps use -1?
                if(itsFolders[FOLDER_SYS].fontMap.contains(name) ||
                   itsFolders[FOLDER_SYS].disabled->items().end()!=
                            itsFolders[FOLDER_SYS].disabled->find(name, 1) ||
                   (!itsRoot && (itsFolders[FOLDER_USER].fontMap.contains(name) ||
                                 itsFolders[FOLDER_USER].disabled->items().end()!=
                                    itsFolders[FOLDER_USER].disabled->find(name, 1))))
                {
                    FcPatternDestroy(pat);
                    error(KIO::ERR_SLAVE_DEFINED, i18n("File %1 contains the font:\n%2\n"
                                                       "A font with this name is already installed.\n",
                                                       urlString(url), name));
                    return FILE_UNKNOWN;
                }
            }
            FcPatternDestroy(pat);
            return FILE_FONT;
        }

        error(KIO::ERR_SLAVE_DEFINED, i18n("Could not determine file type for: %1\n"
                                           "Only fonts may be installed.", urlString(url)));
    }
    return FILE_UNKNOWN;
}

bool CKioFonts::getSourceFiles(const KUrl &src, CDisabledFonts::TFileList &files, bool removeSymLinks)
{
    if(KFI_KIO_FONTS_PROTOCOL==src.protocol())
    {
        CDisabledFonts::TFontList::Iterator disabledIt;
        TFontMap::Iterator                  enabledIt;
        const CDisabledFonts::TFileList     *entries=getEntries(src, enabledIt, disabledIt);

        if(entries)
            getFontFiles(*entries, files, removeSymLinks);
    }
    else
        if(src.isLocalFile())
            if(FILE_UNKNOWN!=checkFile(src.path(), src))
                files.append(CDisabledFonts::TFile(src.path()));
            else
                return false;  // error logged in checkFile...

    if(files.count())
    {
        CDisabledFonts::TFileList::Iterator it,
                                            end=files.end();

        for(it=files.begin(); it!=end; ++it)
        {
            QByteArray      realSrc=QFile::encodeName(*it);
            KDE_struct_stat buffSrc;

            if (-1==KDE_stat(realSrc.constData(), &buffSrc))
            {
                error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST,
                      urlString(src));
                return false;
            }
            if(S_ISDIR(buffSrc.st_mode))
            {
                error(KIO::ERR_IS_DIRECTORY, urlString(src));
                return false;
            }
            if(S_ISFIFO(buffSrc.st_mode) || S_ISSOCK(buffSrc.st_mode))
            {
                error(KIO::ERR_CANNOT_OPEN_FOR_READING, urlString(src));
                return false;
            }
        }
    }
    else
    {
        error(KIO::ERR_DOES_NOT_EXIST, urlString(src));
        return false;
    }

    return true;
}

bool CKioFonts::checkDestFile(const KUrl &src, const KUrl &dest, EFolder destFolder, KIO::JobFlags flags)
{
    QStringList folders;

    folders.append(itsFolders[destFolder].location);
    folders.append(getDestFolder(itsFolders[destFolder].location, src.fileName()));

    QStringList::Iterator it(folders.begin()),
                          end(folders.end());
    QString               destFile;

    for(; it!=end; ++it)
    {
        if(!(flags & KIO::Overwrite) && (Misc::fExists(destFile=(*it)+src.fileName()) ||
                          Misc::fExists(destFile=(*it)+modifyName(src.fileName())) ||
                          Misc::fExists(destFile=(*it)+modifyName(src.fileName(), true)) ) )
        {
            // If copying / moving a TTC and it is the *same* file, then don't log an error, but
            // don't continue the transaction...
            //
            // Reason being that fonts:/ lists the font names (not filenames) so for a TTC there'll
            // be multiple entries...
            if(isSameTtc(src.path(), destFile))
                finished();
            else
                error(KIO::ERR_FILE_ALREADY_EXIST, urlString(dest));
            return false;
        }

        bool    isHidden=Misc::isHidden(src);
        QString other(isHidden ? src.fileName().mid(1)
                               : QChar('.')+src.fileName());

        if(Misc::fExists((*it)+other) ||
           Misc::fExists((*it)+modifyName(other)) ||
           Misc::fExists((*it)+modifyName(other, true)))
        {
            error(KIO::ERR_SLAVE_DEFINED,
                  isHidden
                      ? i18n("Could not install %1\nA matching enabled font already exists. "
                             "Please disable that.", urlString(src))
                      : i18n("Could not install %1\nA matching disabled font already exists. "
                             "Please enable that.", urlString(src)));
            return false;
        }
    }
    return true;
}

bool CKioFonts::checkDestFiles(const KUrl &src, QMap<QString, QString> &map, const KUrl &dest,
                               EFolder destFolder, KIO::JobFlags flags)
{
    //
    // Check whether files exist at destination...
    //
    if(dest.protocol()==src.protocol() &&
       dest.directory()==src.directory())  // Check whether confirmUrl changed a "cp fonts:/System
                                           // fonts:/" to "cp fonts:/System fonts:/System"
    {
        error(KIO::ERR_FILE_ALREADY_EXIST, urlString(dest));
        return false;
    }

    if(!(flags & KIO::Overwrite))
    {
        QMap<QString, QString>::Iterator fIt(map.begin()),
                                         fEnd(map.end());
        QString                          *destEntry;

        for(; fIt!=fEnd; ++fIt)
            if(NULL!=(destEntry=getEntry(destFolder, fIt.value())) ||
               NULL!=(destEntry=getEntry(destFolder, modifyName(fIt.value()))) ||  // lowercase
               NULL!=(destEntry=getEntry(destFolder, modifyName(fIt.value()), true)))  // uppercase
            {
                // If copying / moving a TTC and it is the *same* file, then don't log an error, but
                // don't continue the transaction...
                //
                // Reason being that fonts:/ lists the font names (not filenames) so for a TTC there'll
                // be multiple entries for a TTC...
                if(isSameTtc(src.path(), *destEntry))
                    finished();
                else
                    error(KIO::ERR_FILE_ALREADY_EXIST, urlString(dest));
                return false;
            }
    }

    return true;
}

//
// Gather the number and names of the font faces located in "files". If there is more than 1 face
// (such as there would be for a TTC font), then ask the user for confirmation of the action.
bool CKioFonts::confirmMultiple(const KUrl &url, const CDisabledFonts::TFileList &files, EFolder folder,
                                EOp op)
{
    if(KFI_KIO_FONTS_PROTOCOL!=url.protocol())
        return true;

    CDisabledFonts::TFileList::ConstIterator it,
                                             end=files.end();
    QStringList                              fonts;

    for(it=files.begin(); it!=files.end(); ++it)
    {
        QStringList           fn(getFontNameEntries(folder, *it, OP_ENABLE==op));
        QStringList::Iterator fnIt(fn.begin()),
                              fnEnd(fn.end());

        for(; fnIt!=fnEnd; ++fnIt)
            if(-1==fonts.indexOf(*fnIt))
                fonts.append(*fnIt);
    }

    if(fonts.count()>1)
    {
        QString               out,
                              question;
        QStringList::Iterator it,
                              end=fonts.end();

        for(it=fonts.begin(); it!=end; ++it)
            out+=QString("<li>")+*it+QString("</li>");

        switch(op)
        {
            case OP_MOVE:
                question=i18n("<p>You are attempting to move a font that is located in a file alongside "
                              "other fonts; in order to proceed with the move they will "
                              "all have to be moved. The affected fonts are:</p>"
                              "<ul>%1</ul><p>\n Do you wish to move all of these?</p>", out);
                break;
            case OP_COPY:
                question=i18n("<p>You are attempting to copy a font that is located in a file alongside "
                              "other fonts; in order to proceed with the copy they will "
                              "all have to be copied. The affected fonts are:</p>"
                              "<ul>%1</ul><p>\n Do you wish to copy all of these?</p>", out);
                break;
            case OP_DELETE:
                question=i18n("<p>You are attempting to delete a font that is located in a file alongside "
                              "other fonts; in order to proceed with the deletion they will "
                              "all have to be deleted. The affected fonts are:</p>"
                              "<ul>%1</ul><p>\n Do you wish to delete all of these?</p>", out);
                break;
            case OP_ENABLE:
                question=i18n("<p>You are attempting to enable a font that is located in a file alongside "
                              "other fonts; in order to proceed with the enabling they will "
                              "all have to be enabled. The affected fonts are:</p>"
                              "<ul>%1</ul><p>\n Do you wish to enable all of these?</p>", out);
                break;
            case OP_DISABLE:
                question=i18n("<p>You are attempting to disable a font that is located in a file alongside "
                              "other fonts; in order to proceed with the disabling they will "
                              "all have to be disabled. The affected fonts are:</p>"
                              "<ul>%1</ul><p>\n Do you wish to disable all of these?</p>", out);
                break;
        }

        if(KMessageBox::No==messageBox(question, QuestionYesNo))
        {
            error(KIO::ERR_USER_CANCELED, urlString(url));
            return false;
        }
    }

    return true;
}

bool CKioFonts::confirmMultiple(const KUrl &url, const CDisabledFonts::TFileList *patterns,
                                EFolder folder, EOp op)
{
    if(KFI_KIO_FONTS_PROTOCOL!=url.protocol())
        return true;

    return patterns ? confirmMultiple(url, *patterns, folder, op) : false;
}

bool CKioFonts::checkUrl(const KUrl &u, bool rootOk, bool logError)
{
    if(KFI_KIO_FONTS_PROTOCOL==u.protocol() && (!rootOk || (rootOk && "/"!=u.path())))
    {
        QString sect(getSect(u.path()));

        if(itsRoot)
        {
            if((isSysFolder(sect) || isUserFolder(sect)) &&
               (itsFolders[FOLDER_SYS].fontMap.end()==itsFolders[FOLDER_SYS].fontMap.find(sect)))
//CPD: TODO: || it has a font specified! e.g. fonts:/System/Times -> even in have a
// fonts:/System font, redirect should still happen
            {
                 redirection(getRedirect(u));
                 finished();
                 return false;
            }
        }
        else
            if(!isSysFolder(sect) && !isUserFolder(sect) && !isAllFolder(sect) )
            {
                if(logError)
                    error(KIO::ERR_SLAVE_DEFINED, i18n("Please specify \"%1\" or \"%2\".",
                          i18n(KFI_KIO_FONTS_USER), i18n(KFI_KIO_FONTS_SYS)));
                return false;
            }
    }

    return true;
}

bool CKioFonts::checkAllowed(const KUrl &u)
{
    if (KFI_KIO_FONTS_PROTOCOL==u.protocol())
    {
        QString ds(Misc::dirSyntax(u.path()));

        if(ds==QString(QChar('/')+i18n(KFI_KIO_FONTS_USER)+QChar('/')) ||
           ds==QString(QChar('/')+i18n(KFI_KIO_FONTS_SYS)+QChar('/')) ||
           ds==QString(QChar('/')+QString::fromLatin1(KFI_KIO_FONTS_USER)+QChar('/')) ||
           ds==QString(QChar('/')+QString::fromLatin1(KFI_KIO_FONTS_SYS)+QChar('/')))
        {
            error(KIO::ERR_SLAVE_DEFINED,
                  i18n("You cannot rename, move, copy, or delete either \"%1\" or \"%2\".",
                   i18n(KFI_KIO_FONTS_USER), i18n(KFI_KIO_FONTS_SYS))); \
            return false;
        }
    }

    return true;
}

//
// Create an AFM from a Type 1 (pfa/pfb) font and its PFM file...
void CKioFonts::createAfm(const QString &file, bool nrs)
{
    if(nrs && itsPasswd.isEmpty())
        return;

    bool type1=isAType1(file),
         pfm=!type1 && isAPfm(file);  // No point checking if is pfm if its a type1

    if(type1 || pfm)
    {
        // pf2afm wants files with lowercase extension, so just check for lowercase!
        // -- when a font is installed, the extension is converted to lowercase anyway...
        QString afm=getMatch(file, "afm");

        if(afm.isEmpty())  // No point creating if AFM already exists!
        {
            QString pfm,
                    t1;

            if(type1)      // Its a Type1, so look for existing PFM
            {
                pfm=getMatch(file, "pfm");
                t1=file;
            }
            else           // Its a PFM, so look for existing Type1
            {
                t1=getMatch(file, "pfa");
                if(t1.isEmpty())
                    t1=getMatch(file, "pfb");
                pfm=file;
            }

            if(!t1.isEmpty() && !pfm.isEmpty())         // Do we have both Type1 and PFM?
            {
                QString name(t1.left(t1.length()-4));   // pf2afm wants name without extension...

                if(nrs)
                    doRootCmd(KUrl(), TCommand(KFI::CMD_CREATE_AFM, name));
                else
                    Misc::doCmd("pf2afm", QFile::encodeName(name));
            }
        }
    }
}

int CKioFonts::reconfigTimeout()
{
    return hasMetaData(KFI_KIO_TIMEOUT)
            ? metaData(KFI_KIO_TIMEOUT).toInt()
            : DEFAULT_TIMEOUT;
}

void CKioFonts::TFolder::setLocation(const QString &l, bool sys)
{
    location=l;
    delete disabled;
    disabled=new CDisabledFonts(sys);
}

}
