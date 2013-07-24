/*
 *  KWin - the KDE window manager
 *  This file is part of the KDE project.
 *
 * Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "genericscriptedconfig.h"
#include "config-kwin.h"
#include <KDE/KAboutData>
#include <KDE/KStandardDirs>
#include <KDE/KLocalizedString>
#include <Plasma/ConfigLoader>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QFile>
#include <QLabel>
#include <QUiLoader>
#include <QVBoxLayout>

namespace KWin {

GenericScriptedConfigFactory::GenericScriptedConfigFactory()
    : KPluginFactory("kcm_kwin4_genericscripted")
{
}

QObject *GenericScriptedConfigFactory::create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
{
    Q_UNUSED(iface)
    Q_UNUSED(parent)
    if (keyword.startsWith(QStringLiteral("kwin4_effect_"))) {
        return new ScriptedEffectConfig(componentName(), keyword, parentWidget, args);
    } else {
        return new ScriptingConfig(componentName(), keyword, parentWidget, args);
    }
}

GenericScriptedConfig::GenericScriptedConfig(const QString &componentName, const QString &keyword, QWidget *parent, const QVariantList &args)
    : KCModule(KAboutData::pluginData(componentName), parent, args)
    , m_packageName(keyword)
{
}

GenericScriptedConfig::~GenericScriptedConfig()
{
}

void GenericScriptedConfig::createUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    const QString kconfigXTFile = KStandardDirs::locate("data",
                                                        QStringLiteral(KWIN_NAME) +
                                                        QStringLiteral("/") +
                                                        typeName() +
                                                        QStringLiteral("/") +
                                                        m_packageName +
                                                        QStringLiteral("/contents/config/main.xml"));
    const QString uiPath = KStandardDirs::locate("data",
                                                 QStringLiteral(KWIN_NAME) +
                                                 QStringLiteral("/") +
                                                 typeName() +
                                                 QStringLiteral("/") +
                                                 m_packageName +
                                                 QStringLiteral("/contents/ui/config.ui"));
    if (kconfigXTFile.isEmpty() || uiPath.isEmpty()) {
        layout->addWidget(new QLabel(i18nc("Error message", "Plugin does not provide configuration file in expected location")));
        return;
    }
    QFile xmlFile(kconfigXTFile);
    KConfigGroup cg = configGroup();
    Plasma::ConfigLoader *configLoader = new Plasma::ConfigLoader(&cg, &xmlFile, this);
    // load the ui file
    QUiLoader *loader = new QUiLoader(this);
    QFile uiFile(uiPath);
    uiFile.open(QFile::ReadOnly);
    QWidget *customConfigForm = loader->load(&uiFile, this);
    uiFile.close();
    layout->addWidget(customConfigForm);
    addConfig(configLoader, customConfigForm);
}

void GenericScriptedConfig::save()
{
    KCModule::save();
    reload();
}

void GenericScriptedConfig::reload()
{
}

ScriptedEffectConfig::ScriptedEffectConfig(const QString &componentName, const QString &keyword, QWidget *parent, const QVariantList &args)
    : GenericScriptedConfig(componentName, keyword, parent, args)
{
    createUi();
}

ScriptedEffectConfig::~ScriptedEffectConfig()
{
}

QString ScriptedEffectConfig::typeName() const
{
    return QStringLiteral("effects");
}

KConfigGroup ScriptedEffectConfig::configGroup()
{
    return KSharedConfig::openConfig(QStringLiteral(KWIN_CONFIG))->group(QStringLiteral("Effect-") + packageName());
}

void ScriptedEffectConfig::reload()
{
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kwin"),
                                                          QStringLiteral("/KWin"),
                                                          QStringLiteral("org.kde.KWin"),
                                                          QStringLiteral("reconfigureEffect"));
    message << QString(packageName());
    QDBusConnection::sessionBus().send(message);
}

ScriptingConfig::ScriptingConfig(const QString &componentName, const QString &keyword, QWidget *parent, const QVariantList &args)
    : GenericScriptedConfig(componentName, keyword, parent, args)
{
    createUi();
}

ScriptingConfig::~ScriptingConfig()
{
}

KConfigGroup ScriptingConfig::configGroup()
{
    return KSharedConfig::openConfig(QStringLiteral(KWIN_CONFIG))->group(QStringLiteral("Script-") + packageName());
}

QString ScriptingConfig::typeName() const
{
    return QStringLiteral("scripts");
}

void ScriptingConfig::reload()
{
    // TODO: what to call
}

K_EXPORT_PLUGIN(GenericScriptedConfigFactory())

} // namespace
