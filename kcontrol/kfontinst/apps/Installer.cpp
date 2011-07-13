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

#include "Installer.h"
#include "Misc.h"
#include "FontsPackage.h"
#include <QtCore/QFile>
#include <KDE/KCmdLineArgs>
#include <KDE/KAboutData>
#include <KDE/KApplication>
#include <KDE/KZip>
#include <KDE/KTempDir>
#include <KDE/KMessageBox>
#include <KDE/KStandardDirs>
#include <KDE/KIO/NetAccess>
#include "JobRunner.h"
#include "CreateParent.h"

namespace KFI
{

int CInstaller::install(const QSet<KUrl> &urls)
{
    QSet<KUrl>::ConstIterator it(urls.begin()),
                              end(urls.end());
    bool                      sysInstall(false);
    CJobRunner *jobRunner=new CJobRunner(itsParent);

    CJobRunner::startDbusService();

    if(!Misc::root())
    {
        switch(KMessageBox::questionYesNoCancel(itsParent,
                                       i18n("Do you wish to install the font(s) for personal use "
                                            "(only available to you), or "
                                            "system-wide (available to all users)?"),
                                       i18n("Where to Install"), KGuiItem(i18n(KFI_KIO_FONTS_USER)),
                                       KGuiItem(i18n(KFI_KIO_FONTS_SYS))))
        {
            case KMessageBox::No:
                sysInstall=true;
                break;
            case KMessageBox::Cancel:
                return -1;
            default:
                break;
        }
    }

    QSet<KUrl> instUrls;

    for(; it!=end; ++it)
    {
        KUrl local(KIO::NetAccess::mostLocalUrl(*it, NULL));
        bool package(false);

        if(local.isLocalFile())
        {
            QString localFile(local.toLocalFile());

            if(Misc::isPackage(localFile))
            {
                instUrls+=FontsPackage::extract(localFile, &itsTempDir);
                package=true;
            }
        }
        if(!package)
        {
            KUrl::List associatedUrls;

            CJobRunner::getAssociatedUrls(*it, associatedUrls, false, itsParent);
            instUrls.insert(*it);

            KUrl::List::Iterator aIt(associatedUrls.begin()),
                                 aEnd(associatedUrls.end());

            for(; aIt!=aEnd; ++aIt)
                instUrls.insert(*aIt);
        }
    }

    if(instUrls.count())
    {
        CJobRunner::ItemList      list;
        QSet<KUrl>::ConstIterator it(instUrls.begin()),
                                  end(instUrls.end());

        for(; it!=end; ++it)
            list.append(*it);

        return jobRunner->exec(CJobRunner::CMD_INSTALL, list, Misc::root() || sysInstall);
    }
    else
        return -1;
}

CInstaller::~CInstaller()
{
    delete itsTempDir;
}

}

static KAboutData aboutData("kfontinst", KFI_CATALOGUE, ki18n("Font Installer"), "1.0", ki18n("Simple font installer"),
                            KAboutData::License_GPL, ki18n("(C) Craig Drummond, 2007"));

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("embed <winid>", ki18n("Makes the dialog transient for an X app specified by winid"));
    options.add("+[URL]", ki18n("URL to install"));
    KCmdLineArgs::addCmdLineOptions(options);

    QSet<KUrl>   urls;
    KCmdLineArgs *args(KCmdLineArgs::parsedArgs());

    for(int i=0; i < args->count(); i++)
        urls.insert(args->url(i));

    if(urls.count())
    {
        KLocale::setMainCatalog(KFI_CATALOGUE);

        KApplication    app;
        QString         opt(args->getOption("embed"));
        KFI::CInstaller inst(createParent(opt.size() ? opt.toInt(0, 16) : 0));

        return inst.install(urls);
    }

    return -1;
}
