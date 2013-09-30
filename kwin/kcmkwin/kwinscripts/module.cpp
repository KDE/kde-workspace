/*
 * Copyright (c) 2011 Tamas Krutki <ktamasw@gmail.com>
 * Copyright (c) 2012 Martin Gräßlin <mgraesslin@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "module.h"
#include "ui_module.h"

#include <QFileDialog>
#include <QStringList>
#include <QStandardPaths>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingCall>

#include <KDE/KAboutData>
#include <KDE/KPluginFactory>
#include <KDE/KMessageBox>
#include <KDE/KMessageWidget>
#include <KDE/KPluginInfo>
#include <KDE/KServiceTypeTrader>
#include <Plasma/Package>
#include <KNS3/DownloadDialog>

#include "version.h"

K_PLUGIN_FACTORY_DECLARATION(KcmKWinScriptsFactory);

Module::Module(QWidget *parent, const QVariantList &args) :
    KCModule(parent, args),
    ui(new Ui::Module),
    m_kwinConfig(KSharedConfig::openConfig("kwinrc"))
{
    KAboutData *about = new KAboutData("kwin-scripts", QString(),
                                       i18n("KWin Scripts"),
                                       global_s_versionStringFull,
                                       i18n("Configure KWin scripts"),
                                       KAboutData::License_GPL_V2);

    about->addAuthor(i18n("Tamás Krutki"));
    setAboutData(about);

    ui->setupUi(this);
    ui->ghnsButton->setIcon(QIcon::fromTheme("get-hot-new-stuff"));

    connect(ui->scriptSelector, SIGNAL(changed(bool)), this, SLOT(changed()));
    connect(ui->importScriptButton, SIGNAL(clicked()), SLOT(importScript()));
    connect(ui->ghnsButton, SIGNAL(clicked(bool)), SLOT(slotGHNSClicked()));

    ui->importScriptButton->setEnabled(false);

    updateListViewContents();
}

Module::~Module()
{
    delete ui;
}

void Module::importScript()
{
    QString path = QFileDialog::getOpenFileName(nullptr, i18n("Import KWin Script"), QDir::homePath(),
                                                i18n("*.kwinscript|KWin scripts (*.kwinscript)"));

    if (path.isNull()) {
        return;
    }
#warning Needs adjustments to changes in Plasma::Package
#if 0
    if (!Plasma::Package::installPackage(path, QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kwin/scripts/", "kwin-script-")) {
        KMessageWidget* msgWidget = new KMessageWidget;
        msgWidget->setText(ki18n("Cannot import selected script: maybe a script already exists with the same name or there is a permission problem.").toString());
        msgWidget->setMessageType(KMessageWidget::Error);
        ui->verticalLayout2->insertWidget(0, msgWidget);
        msgWidget->animatedShow();
    }
#endif
    // TODO: reload list after successful import
}

void Module::updateListViewContents()
{
    KService::List offers = KServiceTypeTrader::self()->query("KWin/Script", "not (exist [X-KWin-Exclude-Listing]) or [X-KWin-Exclude-Listing] == false");
    QList<KPluginInfo> scriptinfos = KPluginInfo::fromServices(offers);
    ui->scriptSelector->addPlugins(scriptinfos, KPluginSelector::ReadConfigFile, QString(), QString(), m_kwinConfig);
}

void Module::defaults()
{
    ui->scriptSelector->defaults();
    emit changed(true);
}

void Module::load()
{
    updateListViewContents();
    ui->scriptSelector->load();

    emit changed(false);
}

void Module::save()
{
    ui->scriptSelector->save();
    m_kwinConfig->sync();
    QDBusMessage message = QDBusMessage::createMethodCall("org.kde.kwin", "/Scripting", "org.kde.kwin.Scripting", "start");
    QDBusConnection::sessionBus().asyncCall(message);

    emit changed(false);
}

void Module::slotGHNSClicked()
{
    QPointer<KNS3::DownloadDialog> downloadDialog = new KNS3::DownloadDialog("kwinscripts.knsrc", this);
    if (downloadDialog->exec() == QDialog::Accepted) {
        if (!downloadDialog->changedEntries().isEmpty()) {
            updateListViewContents();
        }
    }
    delete downloadDialog;
}

#include "module.moc"
