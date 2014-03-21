/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org)
 *   Copyright (C) 2008 Montel Laurent <montel@kde.org)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <KLocalizedString>
#include <KAboutData>
#include <kdbusservice.h>
#include <kdemacros.h>

#include <QApplication>
#include <QtCore/QCommandLineParser>
#include "kmenuedit.h"
#ifndef Q_WS_WIN
#include "khotkeys.h"
#endif

static const char description[] = I18N_NOOP("KDE menu editor");
static const char version[] = "0.9";

static KMenuEdit *menuEdit = 0;

class KMenuApplication : public QApplication
{
public:
   KMenuApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    {
        QCoreApplication::setApplicationName(QStringLiteral("kmenuedit"));
        QCoreApplication::setApplicationVersion(QString(version));
        QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
        QApplication::setApplicationDisplayName(i18n("KDE Menu Editor"));
    }
#ifdef WITH_HOTKEYS
   virtual ~KMenuApplication() { KHotKeys::cleanup(); }
#endif
};


extern "C" int Q_DECL_EXPORT kdemain( int argc, char **argv )
{
    KAboutData aboutData(QStringLiteral("kmenuedit"), QString(), i18n("KDE Menu Editor"),
                         QString(version), i18n(description), KAboutData::License_GPL,
                         i18n("(C) 2000-2003, Waldo Bastian, Raffaele Sandrini, Matthias Elter"));
    aboutData.addAuthor(i18n("Waldo Bastian"), i18n("Maintainer"), "bastian@kde.org");
    aboutData.addAuthor(i18n("Raffaele Sandrini"), i18n("Previous Maintainer"), QStringLiteral("sandrini@kde.org"));
    aboutData.addAuthor(i18n("Matthias Elter"), i18n("Original Author"), QStringLiteral("elter@kde.org"));
    aboutData.addAuthor(i18n("Montel Laurent"), QString(), QStringLiteral("montel@kde.org"));

    KMenuApplication app(argc, argv);

    KDBusService service(KDBusService::Unique);

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("KDE Menu Editor"));
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("menu"),
                                 i18n("Sub menu to pre-select"),
                                 QStringLiteral("[menu]"));
    parser.addPositionalArgument(QStringLiteral("menu-id"),
                                 i18n("Menu entry to pre-select"),
                                 QStringLiteral("[menu-id]"));

    parser.process(app);
    const auto args = parser.positionalArguments();

    menuEdit = new KMenuEdit();
    if (!args.isEmpty()) {
        menuEdit->selectMenu(args.at(0));
        if (args.count() > 1) {
            menuEdit->selectMenuEntry(args.at(1));
        }
    }
    menuEdit->show();

    return  app.exec();
}
