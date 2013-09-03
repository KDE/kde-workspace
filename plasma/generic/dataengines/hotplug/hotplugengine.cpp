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

#include "hotplugengine.h"
#include "hotplugservice.h"

#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>

#include <KDirWatch>
#include <KConfigGroup>
#include <QDebug>
#include <KLocale>
#include <KDesktopFile>
#include <kdesktopfileactions.h>
#include <Plasma/DataContainer>

//solid specific includes
#include <Solid/DeviceNotifier>
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>
#include <Solid/OpticalDisc>

//#define HOTPLUGENGINE_TIMING

HotplugEngine::HotplugEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args),
      m_dirWatch(new KDirWatch(this))
{
    const QStringList folders = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "solid/actions", QStandardPaths::LocateDirectory);

    foreach (const QString &folder, folders) {
        m_dirWatch->addDir(folder, KDirWatch::WatchFiles);
    }
    connect(m_dirWatch, SIGNAL(dirty(QString)), this, SLOT(updatePredicates(QString)));
    init();
}

HotplugEngine::~HotplugEngine()
{

}

void HotplugEngine::init()
{
    findPredicates();

    Solid::Predicate p(Solid::DeviceInterface::StorageAccess);
    p |= Solid::Predicate(Solid::DeviceInterface::StorageDrive);
    p |= Solid::Predicate(Solid::DeviceInterface::StorageVolume);
    p |= Solid::Predicate(Solid::DeviceInterface::OpticalDrive);
    p |= Solid::Predicate(Solid::DeviceInterface::PortableMediaPlayer);
    p |= Solid::Predicate(Solid::DeviceInterface::SmartCardReader);
    p |= Solid::Predicate(Solid::DeviceInterface::Camera);
    QList<Solid::Device> devices = Solid::Device::listFromQuery(p);
    foreach (const Solid::Device &dev, devices) {
        m_startList.insert(dev.udi(), dev);
    }

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
            this, SLOT(onDeviceAdded(QString)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
            this, SLOT(onDeviceRemoved(QString)));

    m_encryptedPredicate = Solid::Predicate("StorageVolume", "usage", "Encrypted");

    processNextStartupDevice();
}

Plasma::Service* HotplugEngine::serviceForSource(const QString& source)
{
    return new HotplugService (this, source);
}

void HotplugEngine::processNextStartupDevice()
{
    if (!m_startList.isEmpty()) {
        QHash<QString, Solid::Device>::iterator it = m_startList.begin();
        //Solid::Device dev = const_cast<Solid::Device &>(m_startList.takeFirst());
        onDeviceAdded(it.value(), false);
        m_startList.erase(it);
    }

    if (m_startList.isEmpty()) {
        m_predicates.clear();
    } else {
        QTimer::singleShot(0, this, SLOT(processNextStartupDevice()));
    }
}

void HotplugEngine::findPredicates()
{
    m_predicates.clear();
    QStringList files;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "solid/actions", QStandardPaths::LocateDirectory);
    Q_FOREACH (const QString& dir, dirs) {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.desktop"));
        while (it.hasNext()) {
            files.prepend(it.next());
        }
    }
    qDebug() << files;
    foreach (const QString &path, files) {
        KDesktopFile cfg(path);
        const QString string_predicate = cfg.desktopGroup().readEntry("X-KDE-Solid-Predicate");
        //qDebug() << path << string_predicate;
        m_predicates.insert(QUrl(path).fileName(), Solid::Predicate::fromString(string_predicate));
    }

    if (m_predicates.isEmpty()) {
        m_predicates.insert(QString(), Solid::Predicate::fromString(QString()));
    }
}

void HotplugEngine::updatePredicates(const QString &path)
{
    Q_UNUSED(path)

    findPredicates();

    QHashIterator<QString, Solid::Device> it(m_devices);
    while (it.hasNext()) {
        it.next();
        Solid::Device device(it.value());
        QString udi(it.key());

        const QStringList predicates = predicatesForDevice(device);
        if (!predicates.isEmpty()) {
            if (sources().contains(udi)) {
                Plasma::DataEngine::Data data;
                data.insert("predicateFiles", predicates);
                setData(udi, data);
            } else {
                onDeviceAdded(device, false);
            }
        } else if (!m_encryptedPredicate.matches(device) && sources().contains(udi)) {
            removeSource(udi);
        }
    }
}

QStringList HotplugEngine::predicatesForDevice(Solid::Device &device) const
{
    QStringList interestingDesktopFiles;
    //search in all desktop configuration file if the device inserted is a correct device
    QHashIterator<QString, Solid::Predicate> it(m_predicates);
    //qDebug() << "=================" << udi;
    while (it.hasNext()) {
        it.next();
        if (it.value().matches(device)) {
            //qDebug() << "     hit" << it.key();
            interestingDesktopFiles << it.key();
        }
    }

    return interestingDesktopFiles;
}

void HotplugEngine::onDeviceAdded(const QString &udi)
{
    Solid::Device device(udi);
    onDeviceAdded(device);
}

void HotplugEngine::onDeviceAdded(Solid::Device &device, bool added)
{
    //qDebug() << "adding" << device.udi();
#ifdef HOTPLUGENGINE_TIMING
    QTime t;
    t.start();
#endif
    // Skip things we know we don't care about
    if (device.is<Solid::StorageDrive>()) {
        Solid::StorageDrive *drive = device.as<Solid::StorageDrive>();
        if (!drive->isHotpluggable()) {
#ifdef HOTPLUGENGINE_TIMING
            qDebug() << "storage, but not pluggable, returning" << t.restart();
#endif
            return;
        }
    } else if (device.is<Solid::StorageVolume>()) {
        Solid::StorageVolume *volume = device.as<Solid::StorageVolume>();
        Solid::StorageVolume::UsageType type = volume->usage();
        if ((type == Solid::StorageVolume::Unused ||
             type == Solid::StorageVolume::PartitionTable) && !device.is<Solid::OpticalDisc>()) {
#ifdef HOTPLUGENGINE_TIMING
            qDebug() << "storage volume, but not of interest" << t.restart();
#endif
            return;
        }
    }

    m_devices.insert(device.udi(), device);

    if (m_predicates.isEmpty()) {
        findPredicates();
    }

    const QStringList interestingDesktopFiles = predicatesForDevice(device);
    const bool isEncryptedContainer = m_encryptedPredicate.matches(device);

    if (!interestingDesktopFiles.isEmpty() || isEncryptedContainer) {
        //qDebug() << device.product();
        //qDebug() << device.vendor();
        //qDebug() << "number of interesting desktop file : " << interestingDesktopFiles.size();
        Plasma::DataEngine::Data data;
        data.insert("added", added);
        data.insert("udi", device.udi());

        if (!device.description().isEmpty()) {
            data.insert("text", device.description());
        } else {
            data.insert("text", QString(device.vendor() + QLatin1Char(' ') + device.product()));
        }
        data.insert("icon", device.icon());
        data.insert("emblems", device.emblems());
        data.insert("predicateFiles", interestingDesktopFiles);

        QVariantList actions;
        foreach(const QString& desktop, interestingDesktopFiles) {
            const QString actionUrl = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "solid/actions/" + desktop);
	    qDebug() << actionUrl;
            QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(actionUrl, true);
            if (!services.isEmpty()) {
                Plasma::DataEngine::Data action;
                action.insert("predicate", desktop);
                action.insert("text", services[0].text());
                action.insert("icon", services[0].icon());
                actions << action;
            }
        }
        data.insert("actions", actions);

        data.insert("isEncryptedContainer", isEncryptedContainer);

        setData(device.udi(), data);
        //qDebug() << "add hardware solid : " << udi;
    }

#ifdef HOTPLUGENGINE_TIMING
    qDebug() << "total time" << t.restart();
#endif
}

void HotplugEngine::onDeviceRemoved(const QString &udi)
{
    //qDebug() << "remove hardware:" << udi;

    if (m_startList.contains(udi)) {
        m_startList.remove(udi);
        return;
    }

    m_devices.remove(udi);
    removeSource(udi);
}

K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(hotplug, HotplugEngine, "plasma-dataengine-hotplug.json")

#include "hotplugengine.moc"
