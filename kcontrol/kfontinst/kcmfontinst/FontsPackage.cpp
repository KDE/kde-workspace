/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2009 Craig Drummond <craig@kde.org>
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

#include <KZip>
#include <KTempDir>
#include <KStandardDirs>
#include "FontsPackage.h"
#include "KfiConstants.h"
#include "Misc.h"

namespace KFI
{

namespace FontsPackage
{

QSet<KUrl> extract(const QString &fileName, KTempDir **tempDir)
{
    QSet<KUrl> urls;

    if(!tempDir)
        return urls;

    KZip zip(fileName);

    if(zip.open(QIODevice::ReadOnly))
    {
        const KArchiveDirectory *zipDir=zip.directory();

        if(zipDir)
        {
            QStringList fonts(zipDir->entries());

            if(fonts.count())
            {
                QStringList::ConstIterator it(fonts.begin()),
                                           end(fonts.end());

                for(; it!=end; ++it)
                {
                    const KArchiveEntry *entry=zipDir->entry(*it);

                    if(entry && entry->isFile())
                    {
                        if(!(*tempDir))
                        {
                            (*tempDir)=new KTempDir(KStandardDirs::locateLocal("tmp", KFI_TMP_DIR_PREFIX));
                            (*tempDir)->setAutoRemove(true);
                        }

                        ((KArchiveFile *)entry)->copyTo((*tempDir)->name());

                        QString name(entry->name());

                        //
                        // Cant install hidden fonts, therefore need to
                        // unhide 1st!
                        if(Misc::isHidden(name))
                        {
                            ::rename(QFile::encodeName((*tempDir)->name()+name).data(),
                                     QFile::encodeName((*tempDir)->name()+name.mid(1)).data());
                            name=name.mid(1);
                        }

                        urls.insert(KUrl((*tempDir)->name()+name));
                    }
                }
            }
        }
    }

    return urls;
}

}

}
