/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), Trolltech ASA 
    (or its successors, if any) and the KDE Free Qt Foundation, which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public 
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <kstandarddirs.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QSettings settings(QLatin1String("Trolltech"));
    QString qversion = qVersion();
    if (qversion.count('.') > 1) {
        qversion.truncate(qversion.lastIndexOf('.'));
    }
    if (qversion.contains('-')) {
        qversion.truncate(qversion.lastIndexOf('-'));
    }
    const QString &libPathKey = QString("/qt/%1/libraryPath").arg(qversion);

    QStringList kdeAdded;
    KComponentData kcd("krdb libraryPath fix");
    const QStringList &plugins = KGlobal::dirs()->resourceDirs("qtplugins");
    foreach (const QString &_path, plugins) {
        QString path = QDir(_path).canonicalPath();
        if (path.isEmpty() || kdeAdded.contains(path)) {
            continue;
        }
        kdeAdded.prepend(path);
        if (path.contains("/lib64/")) {
            path.replace("/lib64/", "/lib/");
            if (!kdeAdded.contains(path)) {
                kdeAdded.prepend(path);
            }
        }
    }
    QStringList libraryPath;
    foreach (const QString &path, const_cast<const QStringList &>(kdeAdded)) {
        libraryPath.append(path);
    }

    // Write the list out..
    settings.setValue("/qt/KDE/kdeAddedLibraryPaths", kdeAdded);
    settings.setValue(libPathKey, libraryPath.join(QLatin1String(":")));

    return 0;
}
