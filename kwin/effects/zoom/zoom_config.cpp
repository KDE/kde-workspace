/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>
Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>

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

#include "zoom_config.h"
// KConfigSkeleton
#include "zoomconfig.h"

#include <kwineffects.h>

#include <KDE/KLocalizedString>
#include <kdebug.h>
#include <KActionCollection>
#include <kaction.h>
#include <KShortcutsEditor>
#include <KDE/KAboutData>

#include <QVBoxLayout>

namespace KWin
{

KWIN_EFFECT_CONFIG_FACTORY

ZoomEffectConfigForm::ZoomEffectConfigForm(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
}

ZoomEffectConfig::ZoomEffectConfig(QWidget* parent, const QVariantList& args) :
    KCModule(KAboutData::pluginData(QStringLiteral("zoom")), parent, args)
{
    m_ui = new ZoomEffectConfigForm(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_ui);

    addConfig(ZoomConfig::self(), m_ui);

#warning Global Shortcuts need porting
#if KWIN_QT5_PORTING
    // Shortcut config. The shortcut belongs to the component "kwin"!
    KActionCollection *actionCollection = new KActionCollection(this, KComponentData("kwin"));
    actionCollection->setConfigGroup(QStringLiteral("Zoom"));
    actionCollection->setConfigGlobal(true);

    KAction* a;
    a = static_cast< KAction* >(actionCollection->addAction(KStandardAction::ZoomIn));
    a->setProperty("isConfigurationAction", true);
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Equal));

    a = static_cast< KAction* >(actionCollection->addAction(KStandardAction::ZoomOut));
    a->setProperty("isConfigurationAction", true);
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Minus));

    a = static_cast< KAction* >(actionCollection->addAction(KStandardAction::ActualSize));
    a->setProperty("isConfigurationAction", true);
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_0));

    a = static_cast< KAction* >(actionCollection->addAction(QStringLiteral("MoveZoomLeft")));
    a->setIcon(KIcon(QStringLiteral("go-previous")));
    a->setText(i18n("Move Left"));
    a->setProperty("isConfigurationAction", true);
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Left));

    a = static_cast< KAction* >(actionCollection->addAction(QStringLiteral("MoveZoomRight")));
    a->setIcon(KIcon(QStringLiteral("go-next")));
    a->setText(i18n("Move Right"));
    a->setProperty("isConfigurationAction", true);
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Right));

    a = static_cast< KAction* >(actionCollection->addAction(QStringLiteral("MoveZoomUp")));
    a->setIcon(KIcon("go-up"));
    a->setText(i18n("Move Up"));
    a->setProperty("isConfigurationAction", true);
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Up));

    a = static_cast< KAction* >(actionCollection->addAction(QStringLiteral("MoveZoomDown")));
    a->setIcon(KIcon(QStringLiteral("go-down")));
    a->setText(i18n("Move Down"));
    a->setProperty("isConfigurationAction", true);
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Down));

    a = static_cast< KAction* >(actionCollection->addAction(QStringLiteral("MoveMouseToFocus")));
    a->setIcon(KIcon(QStringLiteral("view-restore")));
    a->setText(i18n("Move Mouse to Focus"));
    a->setProperty("isConfigurationAction", true);
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_F5));

    a = static_cast< KAction* >(actionCollection->addAction(QStringLiteral("MoveMouseToCenter")));
    a->setIcon(KIcon(QStringLiteral("view-restore")));
    a->setText(i18n("Move Mouse to Center"));
    a->setProperty("isConfigurationAction", true);
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_F6));

    m_ui->editor->addCollection(actionCollection);
#endif
    load();
}

ZoomEffectConfig::~ZoomEffectConfig()
{
    // Undo (only) unsaved changes to global key shortcuts
    m_ui->editor->undoChanges();
}

void ZoomEffectConfig::save()
{
    m_ui->editor->save(); // undo() will restore to this state from now on
    KCModule::save();
    EffectsHandler::sendReloadMessage(QStringLiteral("zoom"));
}

} // namespace

#include "zoom_config.moc"
