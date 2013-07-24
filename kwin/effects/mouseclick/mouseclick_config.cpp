/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2012 Filip Wieladek <wattos@gmail.com>

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

#include "mouseclick_config.h"
// KConfigSkeleton
#include "mouseclickconfig.h"

#include <kwineffects.h>

#include <KDE/KActionCollection>
#include <KDE/KAction>
#include <KDE/KAboutData>
#include <KDE/KShortcutsEditor>

#include <QWidget>

namespace KWin
{

KWIN_EFFECT_CONFIG_FACTORY

MouseClickEffectConfigForm::MouseClickEffectConfigForm(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
}

MouseClickEffectConfig::MouseClickEffectConfig(QWidget* parent, const QVariantList& args) :
    KCModule(KAboutData::pluginData(QStringLiteral("mouseclick")), parent, args)
{
    m_ui = new MouseClickEffectConfigForm(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_ui);

    connect(m_ui->editor, SIGNAL(keyChange()), this, SLOT(changed()));

#warning Global Shortcuts need porting
#if KWIN_QT5_PORTING
    // Shortcut config. The shortcut belongs to the component "kwin"!
    m_actionCollection = new KActionCollection(this, KComponentData("kwin"));

    KAction* a = static_cast<KAction*>(m_actionCollection->addAction(QStringLiteral("ToggleMouseClick")));
    a->setText(i18n("Toggle Effect"));
    a->setProperty("isConfigurationAction", true);
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Asterisk));

    m_ui->editor->addCollection(m_actionCollection);
#endif

    addConfig(MouseClickConfig::self(), m_ui);
    load();
}

MouseClickEffectConfig::~MouseClickEffectConfig()
{
    // Undo (only) unsaved changes to global key shortcuts
    m_ui->editor->undoChanges();
}

void MouseClickEffectConfig::save()
{
    KCModule::save();
    m_ui->editor->save();   // undo() will restore to this state from now on
    EffectsHandler::sendReloadMessage(QStringLiteral("mouseclick"));
}

} // namespace

#include "mouseclick_config.moc"
