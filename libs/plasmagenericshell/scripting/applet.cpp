/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "applet.h"

#include <QAction>
#include <QGraphicsLinearLayout>

#include <KService>
#include <KServiceTypeTrader>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>

namespace WorkspaceScripting
{

class Applet::Private
{
public:
    Private()
        : configDirty(false),
          inWallpaperConfig(false),
          wallpaperConfigDirty(false)
    {
    }

    KConfigGroup configGroup;
    QStringList configGroupPath;
    KConfigGroup globalConfigGroup;
    QStringList globalConfigGroupPath;
    bool configDirty : 1;
    bool inWallpaperConfig : 1;
    bool wallpaperConfigDirty : 1;
};

Applet::Applet(QObject *parent)
    : QObject(parent),
      d(new Applet::Private)
{
}

Applet::~Applet()
{
    delete d;
}

void Applet::setCurrentConfigGroup(const QStringList &groupNames)
{
    Plasma::Applet *app = applet();
    if (!app) {
        d->configGroup = KConfigGroup();
        d->configGroupPath.clear();
        return;
    }

    d->configGroup = app->config();
    d->configGroupPath = groupNames;

    foreach (const QString &groupName, groupNames) {
        d->configGroup = KConfigGroup(&d->configGroup, groupName);
    }

    d->inWallpaperConfig = !groupNames.isEmpty() && groupNames.first() == "Wallpaper";
}

QStringList Applet::currentConfigGroup() const
{
    return d->configGroupPath;
}

QStringList Applet::configKeys() const
{
    if (d->configGroup.isValid()) {
        return d->configGroup.keyList();
    }

    return QStringList();
}

QStringList Applet::configGroups() const
{
    if (d->configGroup.isValid()) {
        return d->configGroup.groupList();
    }

    return QStringList();
}

QVariant Applet::readConfig(const QString &key, const QVariant &def) const
{
    if (d->configGroup.isValid()) {
        return d->configGroup.readEntry(key, def);
    } else {
        return QVariant();
    }
}

void Applet::writeConfig(const QString &key, const QVariant &value)
{
    if (d->configGroup.isValid()) {
        if (d->inWallpaperConfig) {
            d->wallpaperConfigDirty = true;
        }

        d->configGroup.writeEntry(key, value);
        d->configDirty = true;
    }
}

void Applet::setCurrentGlobalConfigGroup(const QStringList &groupNames)
{
    Plasma::Applet *app = applet();
    if (!app) {
        d->globalConfigGroup = KConfigGroup();
        d->globalConfigGroupPath.clear();
        return;
    }

    d->globalConfigGroup = app->globalConfig();
    d->globalConfigGroupPath = groupNames;

    foreach (const QString &groupName, groupNames) {
        d->globalConfigGroup = KConfigGroup(&d->globalConfigGroup, groupName);
    }
}

QStringList Applet::currentGlobalConfigGroup() const
{
    return d->globalConfigGroupPath;
}

QStringList Applet::globalConfigKeys() const
{
    if (d->globalConfigGroup.isValid()) {
        return d->globalConfigGroup.keyList();
    }

    return QStringList();
}

QStringList Applet::globalConfigGroups() const
{
    if (d->globalConfigGroup.isValid()) {
        return d->globalConfigGroup.groupList();
    }

    return QStringList();
}

QVariant Applet::readGlobalConfig(const QString &key, const QVariant &def) const
{
    if (d->globalConfigGroup.isValid()) {
        return d->globalConfigGroup.readEntry(key, def);
    } else {
        return QVariant();
    }
}

void Applet::writeGlobalConfig(const QString &key, const QVariant &value)
{
    if (d->globalConfigGroup.isValid()) {
        d->globalConfigGroup.writeEntry(key, value);
        d->configDirty = true;
    }
}

void Applet::reloadConfigIfNeeded()
{
    if (d->configDirty) {
        reloadConfig();
    }
}

void Applet::reloadConfig()
{
    Plasma::Applet *app = applet();
    if (app) {
        KConfigGroup cg = app->config();

        if (!app->isContainment()) {
            app->restore(cg);
        }

        app->configChanged();

        if (app->containment() && app->containment()->corona()) {
            app->containment()->corona()->requestConfigSync();
        }

        d->configDirty = false;
    }
}

QString Applet::version() const
{
    Plasma::Applet *app = applet();
    if (!app) {
        return QString();
    }

    QString type = app->pluginName();
    KService::List services = KServiceTypeTrader::self()->query("Plasma/Applet", "[X-KDE-PluginInfo-Name] == '" + type + "'");
    if (services.isEmpty()) {
        return QString();
    }

    KPluginInfo info(services.first());
    return info.version();
}

void Applet::setLocked(bool locked)
{
    Plasma::Applet *app = applet();
    if (!app) {
        return;
    }

    app->setImmutability(locked ? Plasma::UserImmutable : Plasma::Mutable);
    KConfigGroup cg = app->config();
    if (!app->isContainment()) {
        cg = cg.parent();
    }

    if (cg.isValid()) {
        cg.writeEntry("immutability", (int)app->immutability());
    }
}

bool Applet::locked() const
{
    Plasma::Applet *app = applet();
    if (!app) {
        return Plasma::Mutable;
    }

    return app->immutability() != Plasma::Mutable;
}

bool Applet::wallpaperConfigDirty() const
{
    return d->wallpaperConfigDirty;
}

Plasma::Applet *Applet::applet() const
{
    return 0;
}

}

#include "applet.moc"

