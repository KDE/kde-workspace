/*****************************************************************************
 *  This file is part of the KDE libraries                                    *
 *  Copyright (C) 2012 by Shaun Reich <shaun.reich@kdemail.net>               *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU Lesser General Public License as published  *
 *  by the Free Software Foundation; either version 2 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; see the file COPYING.LIB.                        *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

#include "svgviewer.h"

#include <iostream>

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KDebug>


#include <Plasma/Theme>

static const char description[] = I18N_NOOP("View and browse SVGs used for Plasma. Useful for Plasma theme creating");
static const char version[] = "0.1";

void listPlugins(const KPluginInfo::List & plugins)
{
    int maxLen = 0;
    QMap<QString, QString> applets;
    foreach (const KPluginInfo &info, plugins) {
        if (info.property("NoDisplay").toBool()) {
            continue;
        }

        int len = info.pluginName().length();
        if (len > maxLen) {
            maxLen = len;
        }

        QString name = info.pluginName();
        QString comment = info.comment();

        if (comment.isEmpty()) {
            comment = i18n("No description available");
        }

        applets.insert(name, comment);
    }

    QMap<QString, QString>::const_iterator it;
    for (it = applets.constBegin(); it != applets.constEnd(); ++it) {
        QString applet("%1 - %2");

        applet = applet.arg(it.key().leftJustified(maxLen, ' ')).arg(it.value());
        std::cout << applet.toLocal8Bit().data() << std::endl;
    }
}

int main(int argc, char **argv)
{
    KAboutData aboutData("plasmasvgviewer", 0, ki18n("Plasma SVG Viewer"),
                         version, ki18n(description), KAboutData::License_GPL,
                         ki18n("(c) 2012, The KDE Team"));
    aboutData.addAuthor(ki18n("Shaun M. Reich"),
                        ki18n("Author and maintainer"),
                        "shaun.reich@kdemail.net");
    aboutData.setProgramIconName("plasma");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("list", ki18n("List all known Plasma SVG theme names"));

    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();


    KApplication app;

    if (args->isSet("list")) {
        listPlugins(Plasma::Theme::defaultTheme()->listThemeInfo());
        return 0;
    }


    SvgViewer* w = new SvgViewer;


    args->clear();

    w->show();
    return app.exec();
}
