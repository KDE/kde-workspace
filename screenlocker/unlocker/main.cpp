/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

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
#include <KDE/KAboutData>
#include <KDE/KCmdLineArgs>
#include <KDE/KLocale>
#include <KDE/KGlobal>

#include "unlockapp.h"

static const char description[] = I18N_NOOP( "KDE Plasma Workspaces Screen Un-locker" );
static const char version[] = "0.1";

int main(int argc, char* argv[])
{
    KAboutData aboutData( "kscreenunlocker", 0, ki18n( "Screen Un-Locker" ),
                          version, ki18n(description), KAboutData::License_GPL,
                          ki18n("(c) 2011, Martin Gräßlin") );
    aboutData.addAuthor( ki18n("Martin Gräßlin"),
                         ki18n( "Author and maintainer" ),
                         "mgraesslin@kde.org" );

    KCmdLineArgs::init(argc, argv, &aboutData);

    ScreenLocker::UnlockApp app;
    KGlobal::locale()->insertCatalog(QLatin1String( "libplasma" ));
    app.disableSessionManagement(); // manually-started
    return app.exec();
}
