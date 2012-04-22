/*
    This is the new kwindecoration kcontrol module

    Copyright (c) 2001
        Karol Szwed <gallium@kde.org>
        http://gallium.n3.net/
    Copyright 2009, 2010 Martin Gräßlin <kde@martin-graesslin.com>

    Supports new kwin configuration plugins, and titlebar button position
    modification via dnd interface.

    Based on original "kwintheme" (Window Borders)
    Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
// own
#include "kwindecoration.h"
#include "buttonsconfigdialog.h"
#include "configdialog.h"
#include "decorationmodel.h"
#include "auroraetheme.h"
// Qt
#include <QtDBus/QtDBus>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeView>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QGraphicsObject>
// KDE
#include <KAboutData>
#include <KDialog>
#include <KLocale>
#include <KNS3/DownloadDialog>
#include <KDE/KStandardDirs>
#include <KPluginFactory>
#include <qdeclarative.h>

// KCModule plugin interface
// =========================
K_PLUGIN_FACTORY(KWinDecoFactory,
                 registerPlugin<KWin::KWinDecorationModule>();
                )
K_EXPORT_PLUGIN(KWinDecoFactory("kcmkwindecoration"))

namespace KWin
{

KWinDecorationForm::KWinDecorationForm(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
}


KWinDecorationModule::KWinDecorationModule(QWidget* parent, const QVariantList &)
    : KCModule(KWinDecoFactory::componentData(), parent)
    , kwinConfig(KSharedConfig::openConfig("kwinrc"))
    , m_showTooltips(false)
    , m_configLoaded(false)
    , m_decorationButtons(new DecorationButtons(this))
{
    qmlRegisterType<Aurorae::AuroraeTheme>("org.kde.kwin.aurorae", 0, 1, "AuroraeTheme");
    m_ui = new KWinDecorationForm(this);
    m_ui->configureDecorationButton->setIcon(KIcon("configure"));
    m_ui->configureButtonsButton->setIcon(KIcon("configure"));
    m_ui->ghnsButton->setIcon(KIcon("get-hot-new-stuff"));
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_ui);

    KConfigGroup style(kwinConfig, "Style");

    // Set up the decoration lists and other UI settings
    m_model = new DecorationModel(kwinConfig, this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_ui->decorationList->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    m_ui->decorationList->setAttribute(Qt::WA_TranslucentBackground);
    QPalette pal = m_ui->decorationList->palette();
    pal.setColor(m_ui->decorationList->backgroundRole(), Qt::transparent);
    m_ui->decorationList->setPalette(pal);
    foreach (const QString &importPath, KGlobal::dirs()->findDirs("module", "imports")) {
        m_ui->decorationList->engine()->addImportPath(importPath);
    }
    m_ui->decorationList->rootContext()->setContextProperty("decorationModel", m_proxyModel);
    m_ui->decorationList->rootContext()->setContextProperty("options", m_decorationButtons);
    m_ui->decorationList->rootContext()->setContextProperty("auroraeSource", KStandardDirs::locate("data", "kwin/aurorae/aurorae.qml"));
    m_ui->decorationList->setSource(KStandardDirs::locate("data", "kwin/kcm_kwindecoration/main.qml"));

    readConfig(style);

    connect(m_ui->decorationList->rootObject(), SIGNAL(currentIndexChanged()), SLOT(slotSelectionChanged()));
    connect(m_ui->decorationList->rootObject(), SIGNAL(widthChanged()), m_model, SLOT(regeneratePreviews()));
    connect(m_ui->configureButtonsButton, SIGNAL(clicked(bool)), this, SLOT(slotConfigureButtons()));
    connect(m_ui->ghnsButton, SIGNAL(clicked(bool)), SLOT(slotGHNSClicked()));
    connect(m_ui->searchEdit, SIGNAL(textChanged(QString)), m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(m_ui->configureDecorationButton, SIGNAL(clicked(bool)), SLOT(slotConfigureDecoration()));

    KAboutData *about =
        new KAboutData(I18N_NOOP("kcmkwindecoration"), 0,
                       ki18n("Window Decoration Control Module"),
                       0, KLocalizedString(), KAboutData::License_GPL,
                       ki18n("(c) 2001 Karol Szwed"));
    about->addAuthor(ki18n("Karol Szwed"), KLocalizedString(), "gallium@kde.org");
    setAboutData(about);
    m_model->regeneratePreviews();
}


KWinDecorationModule::~KWinDecorationModule()
{
}

int KWinDecorationModule::itemWidth() const
{
    return m_ui->decorationList->rootObject()->property("width").toInt();
}

// This is the selection handler setting
void KWinDecorationModule::slotSelectionChanged()
{
    emit KCModule::changed(true);
}

// Reads the kwin config settings, and sets all UI controls to those settings
// Updating the config plugin if required
void KWinDecorationModule::readConfig(const KConfigGroup & conf)
{
    m_showTooltips = conf.readEntry("ShowToolTips", true);

    // Find the corresponding decoration name to that of
    // the current plugin library name

    QString libraryName = conf.readEntry("PluginLib",
                                         ((QPixmap::defaultDepth() > 8) ? "kwin3_oxygen" : "kwin3_plastik"));

    if (libraryName.isEmpty()) {
        // Selected decoration doesn't exist, use the default
        libraryName = ((QPixmap::defaultDepth() > 8) ? "kwin3_oxygen" : "kwin3_plastik");
    }

    const int bsize = conf.readEntry("BorderSize", (int)BorderNormal);
    BorderSize borderSize = BorderNormal;
    if (bsize >= BorderTiny && bsize < BordersCount)
        borderSize = static_cast< BorderSize >(bsize);
    if (libraryName == "kwin3_aurorae") {
        KConfig auroraeConfig("auroraerc");
        KConfigGroup group(&auroraeConfig, "Engine");
        const QString themeName = group.readEntry("ThemeName", "example-deco");
        const QModelIndex index = m_proxyModel->mapFromSource(m_model->indexOfAuroraeName(themeName));
        if (index.isValid()) {
            m_ui->decorationList->rootObject()->setProperty("currentIndex", index.row());
        }
    } else {
        const QModelIndex index = m_proxyModel->mapFromSource(m_model->indexOfLibrary(libraryName));
        if (index.isValid()) {
            m_model->setBorderSize(index, borderSize);
            m_ui->decorationList->rootObject()->setProperty("currentIndex", index.row());
        }
    }

    // Buttons tab
    // ============
    m_decorationButtons->setCustomPositions(conf.readEntry("CustomButtonPositions", false));
    // Menu and onAllDesktops buttons are default on LHS
    m_decorationButtons->setLeftButtons(conf.readEntry("ButtonsOnLeft", KDecorationOptions::defaultTitleButtonsLeft()));
    // Help, Minimize, Maximize and Close are default on RHS
    m_decorationButtons->setRightButtons(conf.readEntry("ButtonsOnRight", KDecorationOptions::defaultTitleButtonsRight()));
    if (m_configLoaded)
        m_model->changeButtons(m_decorationButtons);
    else {
        m_configLoaded = true;
        m_model->setButtons(m_decorationButtons->customPositions(), m_decorationButtons->leftButtons(), m_decorationButtons->rightButtons());
    }

    emit KCModule::changed(false);
}


// Writes the selected user configuration to the kwin config file
void KWinDecorationModule::writeConfig(KConfigGroup & conf)
{
    const QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(m_ui->decorationList->rootObject()->property("currentIndex").toInt(), 0));
    const QString libName = m_model->data(index, DecorationModel::LibraryNameRole).toString();

    // General settings
    conf.writeEntry("PluginLib", libName);
    conf.writeEntry("CustomButtonPositions", m_decorationButtons->customPositions());
    conf.writeEntry("ShowToolTips", m_showTooltips);

    // Button settings
    conf.writeEntry("ButtonsOnLeft", m_decorationButtons->leftButtons());
    conf.writeEntry("ButtonsOnRight", m_decorationButtons->rightButtons());
    conf.writeEntry("BorderSize",
                    static_cast<int>(m_model->data(index, DecorationModel::BorderSizeRole).toInt()));

    if (m_model->data(index, DecorationModel::TypeRole).toInt() == DecorationModelData::AuroraeDecoration) {
        KConfig auroraeConfig("auroraerc");
        KConfigGroup group(&auroraeConfig, "Engine");
        group.writeEntry("ThemeName", m_model->data(index, DecorationModel::AuroraeNameRole).toString());
        group.sync();
    }

    // We saved, so tell kcmodule that there have been  no new user changes made.
    emit KCModule::changed(false);
}


// Virutal functions required by KCModule
void KWinDecorationModule::load()
{
    const KConfigGroup config(kwinConfig, "Style");

    // Reset by re-reading the config
    readConfig(config);
}


void KWinDecorationModule::save()
{
    KConfigGroup config(kwinConfig, "Style");
    writeConfig(config);
    config.sync();

    // Send signal to all kwin instances
    QDBusMessage message =
        QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
    QDBusConnection::sessionBus().send(message);
}


void KWinDecorationModule::defaults()
{
    // Set the KDE defaults
    m_showTooltips = true;
    const QModelIndex index = m_proxyModel->mapFromSource(m_model->indexOfName(i18n("Oxygen")));
    if (index.isValid())
        m_ui->decorationList->rootObject()->setProperty("currentIndex", index.row());

    m_decorationButtons->resetToDefaults();

    m_model->changeButtons(m_decorationButtons);

    emit changed(true);
}

QString KWinDecorationModule::quickHelp() const
{
    return i18n("<h1>Window Manager Decoration</h1>"
                "<p>This module allows you to choose the window border decorations, "
                "as well as titlebar button positions and custom decoration options.</p>"
                "To choose a theme for your window decoration click on its name and apply your choice by clicking the \"Apply\" button below."
                " If you do not want to apply your choice you can click the \"Reset\" button to discard your changes."
                "<p>You can configure each theme. There are different options specific for each theme.</p>"
                "<p>On the \"Buttons\" tab check the \"Use custom titlebar button positions\" box "
                "and you can change the positions of the buttons to your liking.</p>");
}

void KWinDecorationModule::slotConfigureButtons()
{
    QPointer< KWinDecorationButtonsConfigDialog > configDialog = new KWinDecorationButtonsConfigDialog(m_decorationButtons, m_showTooltips, this);
    if (configDialog->exec() == KDialog::Accepted) {
        m_decorationButtons->setCustomPositions(configDialog->customPositions());
        m_showTooltips = configDialog->showTooltips();
        m_decorationButtons->setLeftButtons(configDialog->buttonsLeft());
        m_decorationButtons->setRightButtons(configDialog->buttonsRight());
        m_model->changeButtons(m_decorationButtons);
        emit changed(true);
    }

    delete configDialog;
}

void KWinDecorationModule::slotGHNSClicked()
{
    QPointer<KNS3::DownloadDialog> downloadDialog = new KNS3::DownloadDialog("aurorae.knsrc", this);
    if (downloadDialog->exec() == KDialog::Accepted) {
        if (!downloadDialog->changedEntries().isEmpty()) {
            const QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(m_ui->decorationList->rootObject()->property("currentIndex").toInt(), 0));
            const QString libraryName = m_model->data(index, DecorationModel::LibraryNameRole).toString();
            bool aurorae = m_model->data(index, DecorationModel::TypeRole).toInt() == DecorationModelData::AuroraeDecoration;
            const QString auroraeName = m_model->data(index, DecorationModel::AuroraeNameRole).toString();
            m_model->reload();
            if (aurorae) {
                const QModelIndex proxyIndex = m_proxyModel->mapFromSource(m_model->indexOfAuroraeName(auroraeName));
                if (proxyIndex.isValid())
                    m_ui->decorationList->rootObject()->setProperty("currentIndex", proxyIndex.row());
            } else {
                const QModelIndex proxyIndex = m_proxyModel->mapFromSource(m_model->indexOfLibrary(libraryName));
                if (proxyIndex.isValid())
                    m_ui->decorationList->rootObject()->setProperty("currentIndex", proxyIndex.row());
            }
        }
    }
    delete downloadDialog;
}

void KWinDecorationModule::slotConfigureDecoration()
{
    const QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(m_ui->decorationList->rootObject()->property("currentIndex").toInt(), 0));
    bool reload = false;
    if (index.data(DecorationModel::TypeRole).toInt() == DecorationModelData::AuroraeDecoration) {
        QPointer< KDialog > dlg = new KDialog(this);
        dlg->setCaption(i18n("Decoration Options"));
        dlg->setButtons(KDialog::Ok | KDialog::Cancel);
        KWinAuroraeConfigForm *form = new KWinAuroraeConfigForm(dlg);
        dlg->setMainWidget(form);
        form->borderSizesCombo->setCurrentIndex(index.data(DecorationModel::BorderSizeRole).toInt());
        form->buttonSizesCombo->setCurrentIndex(index.data(DecorationModel::ButtonSizeRole).toInt());
        if (dlg->exec() == KDialog::Accepted) {
            m_model->setData(index, form->borderSizesCombo->currentIndex(), DecorationModel::BorderSizeRole);
            m_model->setData(index, form->buttonSizesCombo->currentIndex(), DecorationModel::ButtonSizeRole);
            reload = true;
        }
        delete dlg;
    } else {
        QString name = index.data(DecorationModel::LibraryNameRole).toString();
        QList< QVariant > borderSizes = index.data(DecorationModel::BorderSizesRole).toList();
        const KDecorationDefines::BorderSize size =
            static_cast<KDecorationDefines::BorderSize>(index.data(DecorationModel::BorderSizeRole).toInt());
        QPointer< KWinDecorationConfigDialog > configDialog =
            new KWinDecorationConfigDialog(name, borderSizes, size, this);
        if (configDialog->exec() == KDialog::Accepted) {
            m_model->setData(index, configDialog->borderSize(), DecorationModel::BorderSizeRole);
            reload = true;
        }

        delete configDialog;
    }
    if (reload) {
        // Send signal to all kwin instances
        QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
        QDBusConnection::sessionBus().send(message);
    }
}

DecorationButtons::DecorationButtons(QObject *parent)
    : QObject(parent)
    , m_customPositions(false)
    , m_leftButtons(KDecorationOptions::defaultTitleButtonsLeft())
    , m_rightButtons(KDecorationOptions::defaultTitleButtonsRight())
{
}

DecorationButtons::~DecorationButtons()
{
}

bool DecorationButtons::customPositions() const
{
    return m_customPositions;
}

const QString &DecorationButtons::leftButtons() const
{
    return m_leftButtons;
}

const QString &DecorationButtons::rightButtons() const
{
    return m_rightButtons;
}

void DecorationButtons::setCustomPositions(bool set)
{
    if (m_customPositions == set) {
        return;
    }
    m_customPositions = set;
    emit customPositionsChanged();
}

void DecorationButtons::setLeftButtons(const QString &leftButtons)
{
    if (m_leftButtons == leftButtons) {
        return;
    }
    m_leftButtons = leftButtons;
    emit leftButtonsChanged();
}

void DecorationButtons::setRightButtons (const QString &rightButtons)
{
    if (m_rightButtons == rightButtons) {
        return;
    }
    m_rightButtons = rightButtons;
    emit rightButtonsChanged();
}

void DecorationButtons::resetToDefaults()
{
    setCustomPositions(false);
    setLeftButtons(KDecorationOptions::defaultTitleButtonsLeft());
    setRightButtons(KDecorationOptions::defaultTitleButtonsRight());
}

} // namespace KWin

#include "kwindecoration.moc"
