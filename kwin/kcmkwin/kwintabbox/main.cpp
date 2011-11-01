/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2009 Martin Gräßlin <kde@martin-graesslin.com>

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
#include "main.h"

// Qt
#include <QtDBus/QtDBus>
#include <QVBoxLayout>

// KDE
#include <KAboutApplicationDialog>
#include <KAction>
#include <KActionCollection>
#include <KCModuleProxy>
#include <KPluginFactory>
#include <KPluginInfo>
#include <KPluginLoader>
#include <KTabWidget>
#include <KTitleWidget>
#include <KServiceTypeTrader>
#include <KShortcutsEditor>

// own
#include "tabboxconfig.h"
#include "layoutconfig.h"

K_PLUGIN_FACTORY(KWinTabBoxConfigFactory, registerPlugin<KWin::KWinTabBoxConfig>();)
K_EXPORT_PLUGIN(KWinTabBoxConfigFactory("kcm_kwintabbox"))

namespace KWin
{

KWinTabBoxConfigForm::KWinTabBoxConfigForm(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
}

KWinTabBoxConfig::KWinTabBoxConfig(QWidget* parent, const QVariantList& args)
    : KCModule(KWinTabBoxConfigFactory::componentData(), parent, args)
    , m_config(KSharedConfig::openConfig("kwinrc"))
{
    KGlobal::locale()->insertCatalog("kwin_effects");
    KTabWidget* tabWidget = new KTabWidget(this);
    m_primaryTabBoxUi = new KWinTabBoxConfigForm(tabWidget);
    m_alternativeTabBoxUi = new KWinTabBoxConfigForm(tabWidget);
    m_alternativeTabBoxUi->description->setText(
        i18n("These settings are used by the \"Walk Through Windows Alternative\" actions."));
    tabWidget->addTab(m_primaryTabBoxUi, i18n("Main"));
    tabWidget->addTab(m_alternativeTabBoxUi, i18n("Alternative"));
    QVBoxLayout* layout = new QVBoxLayout(this);
    KTitleWidget* infoLabel = new KTitleWidget(tabWidget);
    infoLabel->setText(i18n("Focus policy settings limit the functionality of navigating through windows."),
                       KTitleWidget::InfoMessage);
    infoLabel->setPixmap(KTitleWidget::InfoMessage, KTitleWidget::ImageLeft);
    layout->addWidget(infoLabel);
    layout->addWidget(tabWidget);

    m_editor = new KShortcutsEditor(m_primaryTabBoxUi, KShortcutsEditor::GlobalAction);
    // Shortcut config. The shortcut belongs to the component "kwin"!
    m_actionCollection = new KActionCollection(this, KComponentData("kwin"));
    m_actionCollection->setConfigGroup("Navigation");
    m_actionCollection->setConfigGlobal(true);
    KAction* a = qobject_cast<KAction*>(m_actionCollection->addAction("Walk Through Windows"));
    a->setProperty("isConfigurationAction", true);
    a->setText(i18n("Walk Through Windows"));
    a->setGlobalShortcut(KShortcut(Qt::ALT + Qt::Key_Tab));
    a = qobject_cast<KAction*>(m_actionCollection->addAction("Walk Through Windows (Reverse)"));
    a->setProperty("isConfigurationAction", true);
    a->setText(i18n("Walk Through Windows (Reverse)"));
    a->setGlobalShortcut(KShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_Backtab));
    a = qobject_cast<KAction*>(m_actionCollection->addAction("Walk Through Windows Alternative"));
    a->setProperty("isConfigurationAction", true);
    a->setText(i18n("Walk Through Windows Alternative"));
    a->setGlobalShortcut(KShortcut(), KAction::ActiveShortcut);
    a = qobject_cast<KAction*>(m_actionCollection->addAction("Walk Through Windows Alternative (Reverse)"));
    a->setProperty("isConfigurationAction", true);
    a->setText(i18n("Walk Through Windows Alternative (Reverse)"));
    a->setGlobalShortcut(KShortcut(), KAction::ActiveShortcut);
    m_editor->addCollection(m_actionCollection, i18n("Navigation"));
    layout->addWidget(m_editor);
    setLayout(layout);

    // search the effect names
    // TODO: way to recognize if a effect is not found
    KServiceTypeTrader* trader = KServiceTypeTrader::self();
    KService::List services;
    QString boxswitch;
    QString presentwindows;
    QString coverswitch;
    QString flipswitch;
    services = trader->query("KWin/Effect", "[X-KDE-PluginInfo-Name] == 'kwin4_effect_boxswitch'");
    if (!services.isEmpty())
        boxswitch = services.first()->name();
    services = trader->query("KWin/Effect", "[X-KDE-PluginInfo-Name] == 'kwin4_effect_presentwindows'");
    if (!services.isEmpty())
        presentwindows = services.first()->name();
    services = trader->query("KWin/Effect", "[X-KDE-PluginInfo-Name] == 'kwin4_effect_coverswitch'");
    if (!services.isEmpty())
        coverswitch = services.first()->name();
    services = trader->query("KWin/Effect", "[X-KDE-PluginInfo-Name] == 'kwin4_effect_flipswitch'");
    if (!services.isEmpty())
        flipswitch = services.first()->name();

    m_primaryTabBoxUi->effectCombo->addItem(i18n("No Effect"));
    m_primaryTabBoxUi->effectCombo->addItem(boxswitch);
    m_primaryTabBoxUi->effectCombo->addItem(presentwindows);
    m_primaryTabBoxUi->effectCombo->addItem(coverswitch);
    m_primaryTabBoxUi->effectCombo->addItem(flipswitch);

    m_alternativeTabBoxUi->effectCombo->addItem(i18n("No Effect"));
    m_alternativeTabBoxUi->effectCombo->addItem(boxswitch);
    m_alternativeTabBoxUi->effectCombo->addItem(presentwindows);
    m_alternativeTabBoxUi->effectCombo->addItem(coverswitch);
    m_alternativeTabBoxUi->effectCombo->addItem(flipswitch);

    // effect config and info button
    m_primaryTabBoxUi->effectInfoButton->setIcon(KIcon("dialog-information"));
    m_primaryTabBoxUi->effectConfigButton->setIcon(KIcon("configure"));
    m_alternativeTabBoxUi->effectInfoButton->setIcon(KIcon("dialog-information"));
    m_alternativeTabBoxUi->effectConfigButton->setIcon(KIcon("configure"));

    // combo boxes
    connect(m_primaryTabBoxUi->listModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    connect(m_primaryTabBoxUi->switchingModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    connect(m_primaryTabBoxUi->effectCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    // check boxes
    connect(m_primaryTabBoxUi->showOutlineCheck, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(m_primaryTabBoxUi->showTabBox, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_primaryTabBoxUi->highlightWindowCheck, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(m_primaryTabBoxUi->showDesktopBox, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    // combo boxes alternative
    connect(m_alternativeTabBoxUi->listModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    connect(m_alternativeTabBoxUi->switchingModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    connect(m_alternativeTabBoxUi->effectCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    // check boxes alternative
    connect(m_alternativeTabBoxUi->showOutlineCheck, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(m_alternativeTabBoxUi->showTabBox, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_alternativeTabBoxUi->highlightWindowCheck, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(m_alternativeTabBoxUi->showDesktopBox, SIGNAL(stateChanged(int)), this, SLOT(changed()));

    // effects
    connect(m_primaryTabBoxUi->effectCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEffectSelectionChanged(int)));
    connect(m_primaryTabBoxUi->effectInfoButton, SIGNAL(clicked(bool)), this, SLOT(slotAboutEffectClicked()));
    connect(m_primaryTabBoxUi->effectConfigButton, SIGNAL(clicked(bool)), this, SLOT(slotConfigureEffectClicked()));

    connect(m_primaryTabBoxUi->layoutConfigButton, SIGNAL(clicked(bool)), this, SLOT(slotConfigureLayoutClicked()));

    // effects alternative
    connect(m_alternativeTabBoxUi->effectCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEffectSelectionChangedAlternative(int)));
    connect(m_alternativeTabBoxUi->effectInfoButton, SIGNAL(clicked(bool)), this, SLOT(slotAboutEffectClickedAlternative()));
    connect(m_alternativeTabBoxUi->effectConfigButton, SIGNAL(clicked(bool)), this, SLOT(slotConfigureEffectClickedAlternative()));

    connect(m_alternativeTabBoxUi->layoutConfigButton, SIGNAL(clicked(bool)), this, SLOT(slotConfigureLayoutClickedAlternative()));

    // check focus policy - we don't offer configs for unreasonable focus policies
    KConfigGroup config(m_config, "Windows");
    QString policy = config.readEntry("FocusPolicy", "ClickToFocus");
    if ((policy == "FocusUnderMouse") || (policy == "FocusStrictlyUnderMouse")) {
        tabWidget->setEnabled(false);
        infoLabel->show();
    } else
        infoLabel->hide();
}

KWinTabBoxConfig::~KWinTabBoxConfig()
{
}

void KWinTabBoxConfig::load()
{
    KCModule::load();

    KConfigGroup config(m_config, "TabBox");
    KConfigGroup alternativeConfig(m_config, "TabBoxAlternative");
    loadConfig(config, m_tabBoxConfig);
    loadConfig(alternativeConfig, m_tabBoxAlternativeConfig);

    // sync to ui
    updateUiFromConfig(m_primaryTabBoxUi, m_tabBoxConfig);
    updateUiFromConfig(m_alternativeTabBoxUi, m_tabBoxAlternativeConfig);

    // effects
    // Set current option to "none" if no plugin is activated.
    m_primaryTabBoxUi->effectCombo->setCurrentIndex(0);
    m_alternativeTabBoxUi->effectCombo->setCurrentIndex(0);
    KConfigGroup effectconfig(m_config, "Plugins");
    KConfigGroup boxswitchconfig(m_config, "Effect-BoxSwitch");
    if (effectEnabled("boxswitch", effectconfig)) {
        if (boxswitchconfig.readEntry("TabBox", true))
            m_primaryTabBoxUi->effectCombo->setCurrentIndex(1);
        if (boxswitchconfig.readEntry("TabBoxAlternative", false))
            m_alternativeTabBoxUi->effectCombo->setCurrentIndex(1);
    }
    KConfigGroup presentwindowsconfig(m_config, "Effect-PresentWindows");
    if (effectEnabled("presentwindows", effectconfig)) {
        if (presentwindowsconfig.readEntry("TabBox", false))
            m_primaryTabBoxUi->effectCombo->setCurrentIndex(2);
        if (presentwindowsconfig.readEntry("TabBoxAlternative", false))
            m_alternativeTabBoxUi->effectCombo->setCurrentIndex(2);
    }
    KConfigGroup coverswitchconfig(m_config, "Effect-CoverSwitch");
    if (effectEnabled("coverswitch", effectconfig)) {
        if (coverswitchconfig.readEntry("TabBox", false))
            m_primaryTabBoxUi->effectCombo->setCurrentIndex(3);
        if (coverswitchconfig.readEntry("TabBoxAlternative", false))
            m_alternativeTabBoxUi->effectCombo->setCurrentIndex(3);
    }
    KConfigGroup flipswitchconfig(m_config, "Effect-FlipSwitch");
    if (effectEnabled("flipswitch", effectconfig)) {
        if (flipswitchconfig.readEntry("TabBox", false))
            m_primaryTabBoxUi->effectCombo->setCurrentIndex(4);
        if (flipswitchconfig.readEntry("TabBoxAlternative", false))
            m_alternativeTabBoxUi->effectCombo->setCurrentIndex(4);
    }
    slotEffectSelectionChanged(m_primaryTabBoxUi->effectCombo->currentIndex());
    slotEffectSelectionChangedAlternative(m_alternativeTabBoxUi->effectCombo->currentIndex());

    emit changed(false);
}

void KWinTabBoxConfig::loadConfig(const KConfigGroup& config, KWin::TabBox::TabBoxConfig& tabBoxConfig)
{
    tabBoxConfig.setClientListMode(TabBox::TabBoxConfig::ClientListMode(
                                       config.readEntry<int>("ListMode", TabBox::TabBoxConfig::defaultListMode())));
    tabBoxConfig.setClientSwitchingMode(TabBox::TabBoxConfig::ClientSwitchingMode(
                                            config.readEntry<int>("SwitchingMode", TabBox::TabBoxConfig::defaultSwitchingMode())));
    tabBoxConfig.setLayout(TabBox::TabBoxConfig::LayoutMode(
                               config.readEntry<int>("LayoutMode", TabBox::TabBoxConfig::defaultLayoutMode())));
    tabBoxConfig.setSelectedItemViewPosition(TabBox::TabBoxConfig::SelectedItemViewPosition(
                config.readEntry<int>("SelectedItem", TabBox::TabBoxConfig::defaultSelectedItemViewPosition())));
    tabBoxConfig.setShowDesktop(config.readEntry<bool>("ShowDesktop",
                                TabBox::TabBoxConfig::defaultShowDesktop()));

    tabBoxConfig.setShowOutline(config.readEntry<bool>("ShowOutline",
                                TabBox::TabBoxConfig::defaultShowOutline()));
    tabBoxConfig.setShowTabBox(config.readEntry<bool>("ShowTabBox",
                               TabBox::TabBoxConfig::defaultShowTabBox()));
    tabBoxConfig.setHighlightWindows(config.readEntry<bool>("HighlightWindows",
                                     TabBox::TabBoxConfig::defaultHighlightWindow()));

    tabBoxConfig.setMinWidth(config.readEntry<int>("MinWidth",
                             TabBox::TabBoxConfig::defaultMinWidth()));
    tabBoxConfig.setMinHeight(config.readEntry<int>("MinHeight",
                              TabBox::TabBoxConfig::defaultMinHeight()));

    tabBoxConfig.setLayoutName(config.readEntry<QString>("LayoutName", TabBox::TabBoxConfig::defaultLayoutName()));
    tabBoxConfig.setSelectedItemLayoutName(config.readEntry<QString>("SelectedLayoutName", TabBox::TabBoxConfig::defaultSelectedItemLayoutName()));
}

void KWinTabBoxConfig::saveConfig(KConfigGroup& config, const KWin::TabBox::TabBoxConfig& tabBoxConfig)
{
    // combo boxes
    config.writeEntry("ListMode",           int(tabBoxConfig.clientListMode()));
    config.writeEntry("SwitchingMode",      int(tabBoxConfig.clientSwitchingMode()));
    config.writeEntry("LayoutMode",         int(tabBoxConfig.layout()));
    config.writeEntry("SelectedItem",       int(tabBoxConfig.selectedItemViewPosition()));
    config.writeEntry("LayoutName",         tabBoxConfig.layoutName());
    config.writeEntry("SelectedLayoutName", tabBoxConfig.selectedItemLayoutName());
    config.writeEntry("ShowDesktop",        tabBoxConfig.isShowDesktop());

    // check boxes
    config.writeEntry("ShowOutline",      tabBoxConfig.isShowOutline());
    config.writeEntry("ShowTabBox",       tabBoxConfig.isShowTabBox());
    config.writeEntry("HighlightWindows", tabBoxConfig.isHighlightWindows());

    // spin boxes
    config.writeEntry("MinWidth",  tabBoxConfig.minWidth());
    config.writeEntry("MinHeight", tabBoxConfig.minHeight());
    config.sync();
}

void KWinTabBoxConfig::save()
{
    KCModule::save();
    KConfigGroup config(m_config, "TabBox");

    // sync ui to config
    updateConfigFromUi(m_primaryTabBoxUi, m_tabBoxConfig);
    updateConfigFromUi(m_alternativeTabBoxUi, m_tabBoxAlternativeConfig);
    saveConfig(config, m_tabBoxConfig);
    config = KConfigGroup(m_config, "TabBoxAlternative");
    saveConfig(config, m_tabBoxAlternativeConfig);

    // effects
    bool boxSwitch              = false;
    bool presentWindowSwitching = false;
    bool coverSwitch            = false;
    bool flipSwitch             = false;
    bool boxSwitchAlternative               = false;
    bool presentWindowSwitchingAlternative  = false;
    bool coverSwitchAlternative             = false;
    bool flipSwitchAlternative              = false;
    switch(m_primaryTabBoxUi->effectCombo->currentIndex()) {
    case 1:
        boxSwitch = true;
        break;
    case 2:
        presentWindowSwitching = true;
        break;
    case 3:
        coverSwitch = true;
        break;
    case 4:
        flipSwitch = true;
        break;
    default:
        break; // nothing
    }
    switch(m_alternativeTabBoxUi->effectCombo->currentIndex()) {
    case 1:
        boxSwitchAlternative = true;
        break;
    case 2:
        presentWindowSwitchingAlternative = true;
        break;
    case 3:
        coverSwitchAlternative = true;
        break;
    case 4:
        flipSwitchAlternative = true;
        break;
    default:
        break; // nothing
    }
    // activate effects if not active
    KConfigGroup effectconfig(m_config, "Plugins");
    if (boxSwitch || boxSwitchAlternative)
        effectconfig.writeEntry("kwin4_effect_boxswitchEnabled", true);
    if (presentWindowSwitching || presentWindowSwitchingAlternative)
        effectconfig.writeEntry("kwin4_effect_presentwindowsEnabled", true);
    if (coverSwitch || coverSwitchAlternative)
        effectconfig.writeEntry("kwin4_effect_coverswitchEnabled", true);
    if (flipSwitch || flipSwitchAlternative)
        effectconfig.writeEntry("kwin4_effect_flipswitchEnabled", true);
    effectconfig.sync();
    KConfigGroup boxswitchconfig(m_config, "Effect-BoxSwitch");
    boxswitchconfig.writeEntry("TabBox", boxSwitch);
    boxswitchconfig.writeEntry("TabBoxAlternative", boxSwitchAlternative);
    boxswitchconfig.sync();
    KConfigGroup presentwindowsconfig(m_config, "Effect-PresentWindows");
    presentwindowsconfig.writeEntry("TabBox", presentWindowSwitching);
    presentwindowsconfig.writeEntry("TabBoxAlternative", presentWindowSwitchingAlternative);
    presentwindowsconfig.sync();
    KConfigGroup coverswitchconfig(m_config, "Effect-CoverSwitch");
    coverswitchconfig.writeEntry("TabBox", coverSwitch);
    coverswitchconfig.writeEntry("TabBoxAlternative", coverSwitchAlternative);
    coverswitchconfig.sync();
    KConfigGroup flipswitchconfig(m_config, "Effect-FlipSwitch");
    flipswitchconfig.writeEntry("TabBox", flipSwitch);
    flipswitchconfig.writeEntry("TabBoxAlternative", flipSwitchAlternative);
    flipswitchconfig.sync();

    m_editor->save();

    // Reload KWin.
    QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
    QDBusConnection::sessionBus().send(message);

    emit changed(false);
}

void KWinTabBoxConfig::defaults()
{
    // combo boxes
    m_primaryTabBoxUi->listModeCombo->setCurrentIndex(TabBox::TabBoxConfig::defaultListMode());
    m_primaryTabBoxUi->switchingModeCombo->setCurrentIndex(TabBox::TabBoxConfig::defaultSwitchingMode());

    // checkboxes
    m_primaryTabBoxUi->showOutlineCheck->setChecked(TabBox::TabBoxConfig::defaultShowOutline());
    m_primaryTabBoxUi->showTabBox->setChecked(TabBox::TabBoxConfig::defaultShowTabBox());
    m_primaryTabBoxUi->highlightWindowCheck->setChecked(TabBox::TabBoxConfig::defaultHighlightWindow());
    m_primaryTabBoxUi->showDesktopBox->setChecked(TabBox::TabBoxConfig::defaultShowDesktop());

    // effects
    m_primaryTabBoxUi->effectCombo->setCurrentIndex(1);

    // alternative
    // combo boxes
    m_alternativeTabBoxUi->listModeCombo->setCurrentIndex(TabBox::TabBoxConfig::defaultListMode());
    m_alternativeTabBoxUi->switchingModeCombo->setCurrentIndex(TabBox::TabBoxConfig::defaultSwitchingMode());

    // checkboxes
    m_alternativeTabBoxUi->showOutlineCheck->setChecked(TabBox::TabBoxConfig::defaultShowOutline());
    m_alternativeTabBoxUi->showTabBox->setChecked(TabBox::TabBoxConfig::defaultShowTabBox());
    m_alternativeTabBoxUi->highlightWindowCheck->setChecked(TabBox::TabBoxConfig::defaultHighlightWindow());
    m_alternativeTabBoxUi->showDesktopBox->setChecked(TabBox::TabBoxConfig::defaultShowDesktop());

    // effects
    m_alternativeTabBoxUi->effectCombo->setCurrentIndex(0);

    m_editor->allDefault();

    emit changed(true);
}

bool KWinTabBoxConfig::effectEnabled(const QString& effect, const KConfigGroup& cfg) const
{
    KService::List services = KServiceTypeTrader::self()->query(
                                  "KWin/Effect", "[X-KDE-PluginInfo-Name] == 'kwin4_effect_" + effect + '\'');
    if (services.isEmpty())
        return false;
    QVariant v = services.first()->property("X-KDE-PluginInfo-EnabledByDefault");
    return cfg.readEntry("kwin4_effect_" + effect + "Enabled", v.toBool());
}

void KWinTabBoxConfig::updateUiFromConfig(KWinTabBoxConfigForm* ui, const KWin::TabBox::TabBoxConfig& config)
{
    // combo boxes
    ui->listModeCombo->setCurrentIndex(config.clientListMode());
    ui->switchingModeCombo->setCurrentIndex(config.clientSwitchingMode());

    // check boxes
    ui->showOutlineCheck->setChecked(config.isShowOutline());
    ui->showTabBox->setChecked(config.isShowTabBox());
    ui->highlightWindowCheck->setChecked(config.isHighlightWindows());
    ui->showDesktopBox->setChecked(config.isShowDesktop());
}

void KWinTabBoxConfig::updateConfigFromUi(const KWin::KWinTabBoxConfigForm* ui, TabBox::TabBoxConfig& config)
{
    config.setClientListMode(TabBox::TabBoxConfig::ClientListMode(ui->listModeCombo->currentIndex()));
    config.setClientSwitchingMode(TabBox::TabBoxConfig::ClientSwitchingMode(ui->switchingModeCombo->currentIndex()));

    config.setShowOutline(ui->showOutlineCheck->isChecked());
    config.setShowTabBox(ui->showTabBox->isChecked());
    config.setHighlightWindows(ui->highlightWindowCheck->isChecked());
    config.setShowDesktop(ui->showDesktopBox->isChecked());
}

void KWinTabBoxConfig::slotEffectSelectionChanged(int index)
{
    effectSelectionChanged(m_primaryTabBoxUi, index);
}

void KWinTabBoxConfig::slotEffectSelectionChangedAlternative(int index)
{
    effectSelectionChanged(m_alternativeTabBoxUi, index);
}

void KWinTabBoxConfig::effectSelectionChanged(KWinTabBoxConfigForm* ui, int index)
{
    bool enabled = false;
    if (index > 0)
        enabled = true;
    ui->effectInfoButton->setEnabled(enabled);
    ui->effectConfigButton->setEnabled(enabled);
}

void KWinTabBoxConfig::slotAboutEffectClicked()
{
    aboutEffectClicked(m_primaryTabBoxUi);
}

void KWinTabBoxConfig::slotAboutEffectClickedAlternative()
{
    aboutEffectClicked(m_alternativeTabBoxUi);
}

void KWinTabBoxConfig::aboutEffectClicked(KWinTabBoxConfigForm* ui)
{
    KServiceTypeTrader* trader = KServiceTypeTrader::self();
    KService::List services;
    QString effect;
    switch(ui->effectCombo->currentIndex()) {
    case 1:
        effect = "boxswitch";
        break;
    case 2:
        effect = "presentwindows";
        break;
    case 3:
        effect = "coverswitch";
        break;
    case 4:
        effect = "flipswitch";
        break;
    default:
        return;
    }
    services = trader->query("KWin/Effect", "[X-KDE-PluginInfo-Name] == 'kwin4_effect_" + effect + '\'');
    if (services.isEmpty())
        return;
    KPluginInfo pluginInfo(services.first());

    const QString name    = pluginInfo.name();
    const QString comment = pluginInfo.comment();
    const QString author  = pluginInfo.author();
    const QString email   = pluginInfo.email();
    const QString website = pluginInfo.website();
    const QString version = pluginInfo.version();
    const QString license = pluginInfo.license();
    const QString icon    = pluginInfo.icon();

    KAboutData aboutData(name.toUtf8(), name.toUtf8(), ki18n(name.toUtf8()), version.toUtf8(), ki18n(comment.toUtf8()), KAboutLicense::byKeyword(license).key(), ki18n(QByteArray()), ki18n(QByteArray()), website.toLatin1());
    aboutData.setProgramIconName(icon);
    const QStringList authors = author.split(',');
    const QStringList emails = email.split(',');
    int i = 0;
    if (authors.count() == emails.count()) {
        foreach (const QString & author, authors) {
            if (!author.isEmpty()) {
                aboutData.addAuthor(ki18n(author.toUtf8()), ki18n(QByteArray()), emails[i].toUtf8(), 0);
            }
            i++;
        }
    }
    QPointer< KAboutApplicationDialog > aboutPlugin = new KAboutApplicationDialog(&aboutData, this);
    aboutPlugin->exec();
    delete aboutPlugin;
}

void KWinTabBoxConfig::slotConfigureEffectClicked()
{
    configureEffectClicked(m_primaryTabBoxUi);
}

void KWinTabBoxConfig::slotConfigureEffectClickedAlternative()
{
    configureEffectClicked(m_alternativeTabBoxUi);
}

void KWinTabBoxConfig::configureEffectClicked(KWinTabBoxConfigForm* ui)
{
    QString effect;
    switch(ui->effectCombo->currentIndex()) {
    case 1:
        effect = "boxswitch_config";
        break;
    case 2:
        effect = "presentwindows_config";
        break;
    case 3:
        effect = "coverswitch_config";
        break;
    case 4:
        effect = "flipswitch_config";
        break;
    default:
        return;
    }
    KCModuleProxy* proxy = new KCModuleProxy(effect);
    QPointer< KDialog > configDialog = new KDialog(this);
    configDialog->setWindowTitle(ui->effectCombo->currentText());
    configDialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);
    connect(configDialog, SIGNAL(defaultClicked()), proxy, SLOT(defaults()));

    QWidget *showWidget = new QWidget(configDialog);
    QVBoxLayout *layout = new QVBoxLayout;
    showWidget->setLayout(layout);
    layout->addWidget(proxy);
    layout->insertSpacing(-1, KDialog::marginHint());
    configDialog->setMainWidget(showWidget);

    if (configDialog->exec() == QDialog::Accepted) {
        proxy->save();
    } else {
        proxy->load();
    }
    delete configDialog;
}

void KWinTabBoxConfig::slotConfigureLayoutClicked()
{
    QPointer<KDialog> dialog = new KDialog(this);
    dialog->setCaption(i18n("Configure Layout"));
    dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);
    connect(dialog, SIGNAL(okClicked()), this, SLOT(slotLayoutChanged()));

    m_configForm = new TabBox::LayoutConfig(dialog);
    m_configForm->setLayout(m_tabBoxConfig.layoutName());
    dialog->setMainWidget(m_configForm);

    dialog->exec();
    delete dialog;
}

void KWinTabBoxConfig::slotLayoutChanged()
{
    m_tabBoxConfig.setLayoutName(m_configForm->selectedLayout());
    emit changed(true);
}

void KWinTabBoxConfig::slotConfigureLayoutClickedAlternative()
{
    QPointer<KDialog> dialog = new KDialog(this);
    dialog->setCaption(i18n("Configure Layout"));
    dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);
    connect(dialog, SIGNAL(okClicked()), this, SLOT(slotLayoutChangedAlternative()));

    m_configForm = new TabBox::LayoutConfig(dialog);
    m_configForm->setLayout(m_tabBoxAlternativeConfig.layoutName());
    dialog->setMainWidget(m_configForm);

    dialog->exec();
    delete dialog;
}

void KWinTabBoxConfig::slotLayoutChangedAlternative()
{
    m_tabBoxAlternativeConfig.setLayoutName(m_configForm->selectedLayout());
    emit changed(true);
}

} // namespace
