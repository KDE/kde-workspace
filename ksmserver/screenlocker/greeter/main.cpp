/********************************************************************
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
#include <QDateTime>

#include <iostream>

#include <signal.h>

#include "greeterapp.h"

static const char description[] = I18N_NOOP( "Greeter for the KDE Plasma Workspaces Screen locker" );
static const char version[] = "0.1";

static void signalHandler(int signum)
{
    ScreenLocker::UnlockApp *instance = static_cast<ScreenLocker::UnlockApp *>(qApp);

    if (!instance)
        return;

    switch(signum)
    {
      case SIGTERM:
        // exit gracefully to not leave behind screensaver processes (bug#224200)
        // return exit code 1 to indicate that a valid password was not entered,
        // to prevent circumventing the password input by sending a SIGTERM
        instance->exit(1);
        break;
      case SIGUSR1:
        instance->lockImmediately();
        break;
    }
}

int main(int argc, char* argv[])
{
    KAboutData aboutData( "kscreenlocker_greet", 0, ki18n( "KScreenLocker Greeter" ),
                          version, ki18n(description), KAboutData::License_GPL,
                          ki18n("(c) 2011, Martin Gräßlin") );
    aboutData.addAuthor( ki18n("Martin Gräßlin"),
                         ki18n( "Author and maintainer" ),
                         "mgraesslin@kde.org" );
    aboutData.addAuthor( ki18n("Chani Armitage"),
                         ki18n("Author"),
                         "chanika@gmail.com");
    aboutData.addAuthor( ki18n("Oswald Buddenhagen"),
                         ki18n("Author"),
                         "ossi@kde.org");
    aboutData.addAuthor( ki18n("Chris Howells"),
                         ki18n("Author"),
                         "howells@kde.org");
    aboutData.addAuthor( ki18n("Luboš Luňák"),
                         ki18n("Author"),
                         "l.lunak@kde.org");
    aboutData.addAuthor( ki18n("Martin R. Jones"),
                         ki18n("Author"),
                         "mjones@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("testing", ki18n("Starts the greeter in testing mode"));
    options.add("immediateLock", ki18n("Lock immediately, ignoring any grace time etc."));
    KCmdLineArgs::addCmdLineOptions(options);

    ScreenLocker::UnlockApp app;
    KGlobal::locale()->insertCatalog(QLatin1String( "libkworkspace" ));
    app.disableSessionManagement(); // manually-started
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("testing")) {
        app.setTesting(true);
        app.setImmediateLock(true);
    } else {
        app.setImmediateLock(args->isSet("immediateLock"));
    }
    args->clear();
    app.desktopResized();

    // This allow ksmserver to know when the applicaion has actually finished setting itself up.
    // Crucial for blocking until it is ready, ensuring locking happens before sleep, e.g.
    std::cout << "Locked at " << QDateTime::currentDateTime().toTime_t() << std::endl;

    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    return app.exec();
}
