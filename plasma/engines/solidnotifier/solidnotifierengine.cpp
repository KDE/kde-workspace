/*
 *   Copyright (C) 2007 Menard Alexis <darktears31@gmail.com>
 *
 * This program is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "solidnotifierengine.h"

#include <KDebug>
#include <KLocale>
#include <KStandardDirs>
#include "plasma/datasource.h"

//solid specific includes
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/predicate.h>



SolidNotifierEngine::SolidNotifierEngine(QObject* parent, const QStringList& args)
    : Plasma::DataEngine(parent)
{
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(const QString &)),
            this, SLOT(onDeviceAdded(const QString &)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString &)),
            this, SLOT(onDeviceRemoved(const QString &)));
    QStringList files = KGlobal::dirs()->findAllResources("data", "solid/actions/*.desktop");
    //kDebug() <<files.size()<<endl;
}

SolidNotifierEngine::~SolidNotifierEngine()
{

}

void SolidNotifierEngine::onDeviceAdded(const QString &udi)
{
    Solid::Device device(udi);
    //temporary predicate in order to filter
    Solid::Predicate predicate=Solid::Predicate::fromString("[[ StorageVolume.ignored == false AND StorageVolume.usage == 'FileSystem' ] OR [ IS StorageAccess AND StorageDrive.driveType == 'Floppy' ]]");
    if(predicate.matches(device))
    {
        setData(udi,i18n("Clef usb"), udi);
        kDebug() << "add hardware solid : " << udi<<endl;
        checkForUpdates();
    }
}

void SolidNotifierEngine::onDeviceRemoved(const QString &udi)
{
    removeSource(udi);
    kDebug() << "remove hardware solid : " << udi<<endl;
    checkForUpdates();
}

#include "solidnotifierengine.moc"
