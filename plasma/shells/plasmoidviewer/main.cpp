/*
 * Copyright 2007 Frerich Raabe <raabe@kde.org>
 * Copyright 2007-2008 Aaron Seigo <aseigo@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fullview.h"

#include <QPixmapCache>

#include <KApplication>
#include <KAboutData>
#include <KAction>
#include <KCmdLineArgs>
#include <KLocale>
#include <KStandardAction>

using namespace Plasma;

static const char description[] = I18N_NOOP("Run Plasma widgets in their own window");

int main(int argc, char **argv)
{
    KAboutData aboutData("plasmoidviewer", 0, ki18n("Plasma Widget Viewer"),
                         "1.0", ki18n(description), KAboutData::License_BSD,
                         ki18n("2007-2008, Frerich Raabe"));
    aboutData.setProgramIconName("plasma");
    aboutData.addAuthor(ki18n("Frerich Raabe"),
                         ki18n("Original author"),
                        "raabe@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("f");
    options.add("formfactor <name>", ki18n("The formfactor to use (horizontal, vertical, mediacenter or planar)"), "planar");
    options.add("l");
    options.add("location <name>", ki18n("The location constraint to start the Containment with (floating, desktop, fullscreen, top, bottom, left, right)"), "floating");
    options.add("c");
    options.add("containment <name>", ki18n("Name of the containment plugin"), "null");
    options.add("w");
    options.add("wallpaper <name>", ki18n("Name of the wallpaper plugin"), QByteArray());
    options.add("p");
    options.add("pixmapcache <size>", ki18n("The size in KB to set the pixmap cache to"));
    options.add("+applet", ki18n("Name of applet to add (required)"));
    options.add("+[args]", ki18n("Optional arguments of the applet to add"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs() ;
    QString pluginName;
    if (args->count() == 0) {
        KCmdLineArgs::usageError(i18n("No applet name specified"));
    }

    //At this point arg(0) is always set
    pluginName = args->arg(0);

    QString formfactor;
    if (args->isSet("formfactor")) {
        kDebug() << "setting FormFactor to" << args->getOption("formfactor");
        formfactor = args->getOption("formfactor");
    }

    QString location;
    if (args->isSet("location")) {
        kDebug() << "setting Location to" << args->getOption("location");
        location = args->getOption("location");
    }

    QString containment;
    containment = args->getOption("containment");
    kDebug() << "setting containment to" << containment;

    QString wallpaper;
    if (args->isSet("wallpaper")) {
        wallpaper = args->getOption("wallpaper");
        kDebug() << "setting wallpaper to" << wallpaper;
    }

    QVariantList appletArgs;
    for (int i = 1; i < args->count(); ++i) {
        appletArgs << args->arg(i);
    }

    FullView view(formfactor, location);
    view.addApplet(pluginName, containment, wallpaper, appletArgs);
    view.show();

    QAction *action = KStandardAction::quit(&app, SLOT(quit()), &view);
    view.addAction(action);

    if (args->isSet("pixmapcache")) {
        kDebug() << "setting pixmap cache to" << args->getOption("pixmapcache").toInt();
        QPixmapCache::setCacheLimit(args->getOption("pixmapcache").toInt());
    }
    args->clear();

    return app.exec();
}

