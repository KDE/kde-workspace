/*
 *  kcmsmserver.cpp
 *  Copyright (c) 2000,2002 Oswald Buddenhagen <ossi@kde.org>
 *
 *  based on kcmtaskbar.cpp
 *  Copyright (c) 2000 Kurt Granroth <granroth@kde.org>
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
 */

#include <QCheckBox>
//Added by qt3to4:
#include <QVBoxLayout>


#include <kapplication.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kworkspace/kworkspace.h>
#include <kstandarddirs.h>
#include <qregexp.h>
#include <kdesktopfile.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <QtDBus/QtDBus>

#include "kcmsmserver.h"
#include "smserverconfigimpl.h"
#include <KPluginFactory>
#include <KPluginLoader>

K_PLUGIN_FACTORY(SMSFactory,
        registerPlugin<SMServerConfig>();
        )
K_EXPORT_PLUGIN(SMSFactory("kcmsmserver"))

SMServerConfig::SMServerConfig( QWidget *parent, const QVariantList & )
  : KCModule (SMSFactory::componentData(), parent)
{
    setQuickHelp( i18n("<h1>Session Manager</h1>"
    " You can configure the session manager here."
    " This includes options such as whether or not the session exit (logout)"
    " should be confirmed, whether the session should be restored again when logging in"
    " and whether the computer should be automatically shut down after session"
    " exit by default."));

    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setMargin(0);
    dialog = new SMServerConfigImpl(this);
    connect(dialog, SIGNAL(changed()), SLOT(changed()));

    topLayout->addWidget(dialog);
}

void SMServerConfig::load()
{
  KConfigGroup c(KSharedConfig::openConfig("ksmserverrc", KConfig::NoGlobals), "General");
  dialog->confirmLogoutCheck->setChecked(c.readEntry("confirmLogout", true));
  bool en = c.readEntry("offerShutdown", true);
  dialog->offerShutdownCheck->setChecked(en);
  dialog->sdGroup->setEnabled(en);

  QString s = c.readEntry( "loginMode" );
  if ( s == "default" )
      dialog->emptySessionRadio->setChecked(true);
  else if ( s == "restoreSavedSession" )
      dialog->savedSessionRadio->setChecked(true);
  else // "restorePreviousLogout"
      dialog->previousSessionRadio->setChecked(true);

  switch (c.readEntry("shutdownType", int(KWorkSpace::ShutdownTypeNone))) {
  case int(KWorkSpace::ShutdownTypeHalt):
    dialog->haltRadio->setChecked(true);
    break;
  case int(KWorkSpace::ShutdownTypeReboot):
    dialog->rebootRadio->setChecked(true);
    break;
  default:
    dialog->logoutRadio->setChecked(true);
    break;
  }
  dialog->excludeLineedit->setText( c.readEntry("excludeApps"));

  emit changed(false);
}

void SMServerConfig::save()
{
  KConfig c("ksmserverrc", KConfig::NoGlobals);
  KConfigGroup group = c.group("General");
  group.writeEntry( "confirmLogout", dialog->confirmLogoutCheck->isChecked());
  group.writeEntry( "offerShutdown", dialog->offerShutdownCheck->isChecked());
  QString s = "restorePreviousLogout";
  if ( dialog->emptySessionRadio->isChecked() )
      s = "default";
  else if ( dialog->savedSessionRadio->isChecked() )
      s = "restoreSavedSession";
  group.writeEntry( "loginMode", s );

  group.writeEntry( "shutdownType",
                 dialog->haltRadio->isChecked() ?
                   int(KWorkSpace::ShutdownTypeHalt) :
                   dialog->rebootRadio->isChecked() ?
                     int(KWorkSpace::ShutdownTypeReboot) :
                     int(KWorkSpace::ShutdownTypeNone));
  group.writeEntry("excludeApps", dialog->excludeLineedit->text());
  c.sync();
# if 0
  // update the k menu if necessary
  QDBusInterface kicker("org.kde.kicker", "/kicker", "org.kde.kicker");
  kicker.call("configure");
#endif
}

void SMServerConfig::defaults()
{
  dialog->previousSessionRadio->setChecked(true);
  dialog->confirmLogoutCheck->setChecked(true);
  dialog->offerShutdownCheck->setChecked(true);
  dialog->sdGroup->setEnabled(true);
  dialog->logoutRadio->setChecked(true);
  dialog->excludeLineedit->clear();
}

#include "kcmsmserver.moc"
