/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

 Copyright (C) 2008, 2009 Martin Gräßlin <mgraesslin@kde.org>

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
#include "flipswitch_config.h"
// KConfigSkeleton
#include "flipswitchconfig.h"

#include <kwineffects.h>

#include <kconfiggroup.h>
#include <KAction>
#include <KActionCollection>
#include <KDE/KAboutData>

#include <QVBoxLayout>

namespace KWin
{

KWIN_EFFECT_CONFIG_FACTORY

FlipSwitchEffectConfigForm::FlipSwitchEffectConfigForm(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
}

FlipSwitchEffectConfig::FlipSwitchEffectConfig(QWidget* parent, const QVariantList& args) :
    KCModule(KAboutData::pluginData(QStringLiteral("flipswitch")), parent, args)
{
    m_ui = new FlipSwitchEffectConfigForm(this);

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(m_ui);

#warning Global Shortcuts need porting
#if KWIN_QT5_PORTING
    // Shortcut config. The shortcut belongs to the component "kwin"!
    m_actionCollection = new KActionCollection(this, KComponentData("kwin"));
    KAction* a = (KAction*)m_actionCollection->addAction(QStringLiteral("FlipSwitchCurrent"));
    a->setText(i18n("Toggle Flip Switch (Current desktop)"));
    a->setGlobalShortcut(KShortcut(), KAction::ActiveShortcut);
    KAction* b = (KAction*)m_actionCollection->addAction(QStringLiteral("FlipSwitchAll"));
    b->setText(i18n("Toggle Flip Switch (All desktops)"));
    b->setGlobalShortcut(KShortcut(), KAction::ActiveShortcut);

    m_actionCollection->setConfigGroup(QStringLiteral("FlipSwitch"));
    m_actionCollection->setConfigGlobal(true);

    m_ui->shortcutEditor->addCollection(m_actionCollection);
#endif

    addConfig(FlipSwitchConfig::self(), m_ui);

    load();
}

FlipSwitchEffectConfig::~FlipSwitchEffectConfig()
{
}

void FlipSwitchEffectConfig::save()
{
    KCModule::save();
    m_ui->shortcutEditor->save();

    EffectsHandler::sendReloadMessage(QStringLiteral("flipswitch"));
}


} // namespace

#include "flipswitch_config.moc"
