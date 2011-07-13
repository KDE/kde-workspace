#ifndef __MISC_H__
#define __MISC_H__

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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <KDE/KUrl>
#include "KfiConstants.h"

class QTextStream;
class QByteArray;

namespace KFI
{

namespace Misc
{
    enum EConstants
    {
        FILE_PERMS = 0644,
        DIR_PERMS  = 0755
    };

    struct TFont
    {
        TFont(const QString &f=QString(), quint32 s=KFI_NO_STYLE_INFO) : family(f), styleInfo(s) { }

        bool operator==(const TFont &o) const { return o.styleInfo==styleInfo && o.family==family; }

        QString family;
        quint32 styleInfo;
    };

    extern KDE_EXPORT QString prettyUrl(const KUrl &url);
    inline KDE_EXPORT bool    isHidden(const QString &f)    { return QChar('.')==f[0]; }
    inline KDE_EXPORT bool    isHidden(const KUrl &url)     { return isHidden(url.fileName()); }
    extern KDE_EXPORT bool    check(const QString &path, bool file, bool checkW=false);
    inline KDE_EXPORT bool    fExists(const QString &p)     { return check(p, true, false); }
    inline KDE_EXPORT bool    dExists(const QString &p)     { return check(p, false, false); }
    inline KDE_EXPORT bool    fWritable(const QString &p)   { return check(p, true, true); }
    inline KDE_EXPORT bool    dWritable(const QString &p)   { return check(p, false, true); }
    extern KDE_EXPORT QString linkedTo(const QString &i);
    extern KDE_EXPORT QString dirSyntax(const QString &d);  // Has trailing slash:  /file/path/
    extern KDE_EXPORT QString fileSyntax(const QString &f);
    extern KDE_EXPORT QString getDir(const QString &f);
    extern KDE_EXPORT QString getFile(const QString &f);
    extern KDE_EXPORT bool    createDir(const QString &dir);
    extern KDE_EXPORT void    setFilePerms(const QByteArray &f);
    inline KDE_EXPORT void    setFilePerms(const QString &f) { setFilePerms(QFile::encodeName(f)); }
    extern KDE_EXPORT QString changeExt(const QString &f, const QString &newExt);
    extern KDE_EXPORT bool    doCmd(const QString &cmd, const QString &p1=QString(),
                                    const QString &p2=QString(), const QString &p3=QString());
    inline KDE_EXPORT bool    root() { return 0==getuid(); }
    extern KDE_EXPORT void    getAssociatedFiles(const QString &file, QStringList &list,
                                                 bool afmAndPfm=true);
    extern KDE_EXPORT time_t  getTimeStamp(const QString &item);
    extern KDE_EXPORT QString getFolder(const QString &defaultDir, const QString &root,
                                        QStringList &dirs);
    extern KDE_EXPORT bool    checkExt(const QString &fname, const QString &ext);
    extern KDE_EXPORT bool    isBitmap(const QString &str);
    extern KDE_EXPORT bool    isMetrics(const QString &str);
    inline KDE_EXPORT bool    isMetrics(const KUrl &url) { return isMetrics(url.fileName()); }
    inline KDE_EXPORT bool    isPackage(const QString &file)
                      { return file.indexOf(KFI_FONTS_PACKAGE)==(file.length()-KFI_FONTS_PACKAGE_LEN); }
    extern KDE_EXPORT int     getIntQueryVal(const KUrl &url, const char *key, int defVal);
    extern KDE_EXPORT bool    printable(const QString &mime);
    inline KDE_EXPORT QString hide(const QString &f) { return '.'!=f[0] ? QChar('.')+f : f; }
    inline KDE_EXPORT QString unhide(const QString &f) { return '.'==f[0] ? f.mid(1) : f; }
    extern KDE_EXPORT uint    qHash(const TFont &key);
    extern KDE_EXPORT QString encodeText(const QString &str, QTextStream &s);
    extern KDE_EXPORT QString contractHome(QString path);
    extern KDE_EXPORT QString expandHome(QString path);
    extern KDE_EXPORT QMap<QString, QString> getFontFileMap(const QSet<QString> &files);
    extern KDE_EXPORT QString modifyName(const QString &fname);
    inline QString getDestFolder(const QString &folder, const QString &file)
    {
        return folder+file[0].toLower()+'/';
    }
    extern KDE_EXPORT QString app(const QString &name, const char *path=0L);
}

}

inline KDE_EXPORT QDataStream & operator<<(QDataStream &ds, const KFI::Misc::TFont &font)
{
    ds << font.family << font.styleInfo;
    return ds;
}

inline KDE_EXPORT QDataStream & operator>>(QDataStream &ds, KFI::Misc::TFont &font)
{
    ds >> font.family >> font.styleInfo;
    return ds;
}

#endif
