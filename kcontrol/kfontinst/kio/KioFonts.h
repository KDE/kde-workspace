#ifndef __KIO_FONTS_H__
#define __KIO_FONTS_H__

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

#include <fontconfig/fontconfig.h>
#include <time.h>
#include <KDE/KIO/SlaveBase>
#include <KDE/KUrl>
#include <KDE/KLocale>

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QVariant>
#include "Misc.h"
#include "KfiConstants.h"
#include "DisabledFonts.h"
#include "Server.h"
#include "Helper.h"

class KTemporaryFile;

namespace KFI
{

class CSuProc;
class CSocket;

class CKioFonts : public KIO::SlaveBase
{
    private:

    enum EFileType
    {
        FILE_UNKNOWN,
        FILE_FONT,
        FILE_METRICS
    };

    enum EFolder
    {
        FOLDER_SYS,
        FOLDER_USER,

        FOLDER_COUNT
    };

    enum EOp
    {
        OP_COPY,
        OP_MOVE,
        OP_DELETE,
        OP_ENABLE,
        OP_DISABLE
    };

    class CDirList : public QSet<QString>
    {
        public:

        CDirList()                   { }
        CDirList(const QString &str) { add(str); }
        void add(const QString &d)   { insert(d); }
    };

    struct TFontDetails
    {
        TFontDetails() : writingSystems(0) { }

        CDisabledFonts::TFileList files;
        quint32                   styleVal;
        qulonglong                writingSystems;
    };

    typedef QHash<QString, TFontDetails> TFontMap;

    struct TFolder
    {
        TFolder() : disabled(NULL) { }
        void  setLocation(const QString &l, bool sys);

        QString        location;
        CDirList       modified;
        TFontMap       fontMap;   // Maps from "Times New Roman" -> $HOME/.fonts/times.ttf
        CDisabledFonts *disabled;
    };

    public:

    struct TCommand
    {
        TCommand(ECommands c) : cmd(c)                    { }
        TCommand(ECommands c, const QVariant &v) : cmd(c) { args.append(v); }
        ECommands       cmd;
        QList<QVariant> args;
    };

    public:

    CKioFonts(const QByteArray &pool, const QByteArray &app);
    virtual ~CKioFonts() { cleanup(); }

    static QString     getSect(const QString &f) { return f.section('/', 1, 1); }

    void               listDir(const KUrl &url);
    void               listDir(EFolder folder, KIO::UDSEntry &entry);
    void               stat(const KUrl &url);
    bool               createStatEntry(KIO::UDSEntry &entry, const KUrl &url, EFolder folder);
    bool               createStatEntryReal(KIO::UDSEntry &entry, const KUrl &url, EFolder folder);
    void               get(const KUrl &url);
    void               put(const KUrl &url, int mode, KIO::JobFlags flags);
    void               copy(const KUrl &src, const KUrl &dest, int mode, KIO::JobFlags flags);
    void               rename(const KUrl &src, const KUrl &dest, KIO::JobFlags flags);
    void               del(const KUrl &url, bool isFile);
    void               cleanup();

    private:

    QString            getUserName(uid_t uid);
    QString            getGroupName(gid_t gid);
    bool               createFontUDSEntry(KIO::UDSEntry &entry, const QString &name,
                                          const CDisabledFonts::TFileList &patterns,
                                          quint32 styleVal, qulonglong writingSystems,
                                          bool sys, bool hidden=false);
    bool               createFolderUDSEntry(KIO::UDSEntry &entry, const QString &name, const QString &path,
                                            bool sys);
    bool               putReal(KTemporaryFile &dest);
    void               modified(int timeout, EFolder folder, bool clearList=true,
                                const CDirList &dirs=CDirList());
    void               special(const QByteArray &a);
    bool               configure(EFolder folder);
    void               doModified();
    bool               getRootPasswd(const KUrl &url, bool askPasswd=true);
    void               quitHelper();
    bool               doRootCmd(const KUrl &url, const TCommand &cmd, bool askPasswd=true);
    bool               doRootCmd(const KUrl &url, QList<TCommand> &cmd, bool askPasswd=true);
    void               correctUrl(KUrl &url);
    void               clearFontList();
    bool               updateFontList();
    EFolder            getFolder(const KUrl &url);
    TFontMap::Iterator getMap(const KUrl &url);
    const CDisabledFonts::TFileList * getEntries(const KUrl &url, TFontMap::Iterator &enabledIt,
                                                 CDisabledFonts::TFontList::Iterator &disabledIt);
    QStringList        getFontNameEntries(EFolder folder, const QString &file, bool disabledFonts);
    QMap<int, QString> getFontIndexToNameEntries(EFolder folder, const QString &file);
    QString *          getEntry(EFolder folder, const QString &file, bool full=false);
    EFileType          checkFile(const QString &file, const KUrl &url);
    bool               getSourceFiles(const KUrl &src, CDisabledFonts::TFileList &files,
                                      bool removeSymLinks=true);
    bool               checkDestFile(const KUrl &src, const KUrl &dest, EFolder destFolder,
                                     KIO::JobFlags flags);
    bool               checkDestFiles(const KUrl &src, QMap<QString, QString> &map, const KUrl &dest,
                                      EFolder destFolder, KIO::JobFlags flags);
    bool               confirmMultiple(const KUrl &url, const CDisabledFonts::TFileList &files,
                                       EFolder folder, EOp op);
    bool               confirmMultiple(const KUrl &url, const CDisabledFonts::TFileList *patterns,
                                       EFolder folder, EOp op);
    bool               checkUrl(const KUrl &u, bool rootOk=false, bool logError=true);
    bool               checkAllowed(const KUrl &u);
    void               createAfm(const QString &file, bool nrs=false);
    int                reconfigTimeout();

    private:

    bool                  itsRoot,
                          itsAddToSysFc;
    QString               itsPasswd;
    QByteArray            itsHelper;
    time_t                itsLastFcCheckTime;
    FcFontSet             *itsFontList;
    TFolder               itsFolders[FOLDER_COUNT];
    QHash<uid_t, QString> itsUserCache;
    QHash<gid_t, QString> itsGroupCache;
    CServer               itsServer;
    CSocket               *itsSocket;
    CSuProc               *itsSuProc;
};

}

#endif
