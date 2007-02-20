/*
 * main.cpp for the samba kcontrol module
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QLayout>
#include <QTabWidget>

#include <kaboutdata.h>
#include <kcmodule.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include "kcmsambaimports.h"
#include "kcmsambalog.h"
#include "kcmsambastatistics.h"
#include "ksmbstatus.h"

class SambaContainer:public KCModule
{
   public:
      SambaContainer(QWidget *parent=0, const QStringList &list = QStringList() );
      virtual ~SambaContainer();
      virtual void load();
      virtual void save();

   private:
      KConfig config;
      QTabWidget tabs;
      NetMon status;
      ImportsView imports;
      LogView logView;
      StatisticsView statisticsView;
};

typedef KGenericFactory<SambaContainer, QWidget > SambaFactory;
K_EXPORT_COMPONENT_FACTORY (samba, SambaFactory("kcmsamba") )

SambaContainer::SambaContainer(QWidget *parent, const QStringList&)
:KCModule(SambaFactory::componentData(), parent)
,config("kcmsambarc")
,tabs(this)
,status(&tabs,&config)
,imports(&tabs,&config)
,logView(&tabs,&config)
,statisticsView(&tabs,&config)
{
   QVBoxLayout *layout = new QVBoxLayout( this );
   layout->setMargin(0);
   layout->setSpacing(KDialog::spacingHint());
   layout->addWidget(&tabs);
   tabs.addTab(&status,i18n("&Exports"));
   tabs.addTab(&imports,i18n("&Imports"));
   tabs.addTab(&logView,i18n("&Log"));
   tabs.addTab(&statisticsView,i18n("&Statistics"));
   connect(&logView,SIGNAL(contentsChanged(Q3ListView* , int, int)),&statisticsView,SLOT(setListInfo(Q3ListView *, int, int)));
   setButtons(Help);
   load();

   setQuickHelp( i18n("The Samba and NFS Status Monitor is a front end to the programs"
     " <em>smbstatus</em> and <em>showmount</em>. Smbstatus reports on current"
     " Samba connections, and is part of the suite of Samba tools, which"
     " implements the SMB (Session Message Block) protocol, also called the"
     " NetBIOS or LanManager protocol. This protocol can be used to provide"
     " printer sharing or drive sharing services on a network including"
     " machines running the various flavors of Microsoft Windows.<p>"
     " Showmount is part of the NFS software package. NFS stands for Network"
     " File System and is the traditional UNIX way to share directories over"
     " the network. In this case the output of <em>showmount -a localhost</em>"
     " is parsed. On some systems showmount is in /usr/sbin, check if you have"
     " showmount in your PATH."));

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmsamba"),
		I18N_NOOP("KDE Panel System Information Control Module"),
		0, 0, KAboutData::License_GPL,
		I18N_NOOP("(c) 2002 KDE Information Control Module Samba Team"));
    about->addAuthor("Michael Glauche", 0, "glauche@isa.rwth-aachen.de");
    about->addAuthor("Matthias Hoelzer", 0, "hoelzer@kde.org");
    about->addAuthor("David Faure", 0, "faure@kde.org");
    about->addAuthor("Harald Koschinski", 0, "Harald.Koschinski@arcormail.de");
    about->addAuthor("Wilco Greven", 0, "greven@kde.org");
    about->addAuthor("Alexander Neundorf", 0, "neundorf@kde.org");
    setAboutData( about );
}

SambaContainer::~SambaContainer()
{
   save();
}

void SambaContainer::load()
{
   status.loadSettings();
   imports.loadSettings();
   logView.loadSettings();
   statisticsView.loadSettings();
}

void SambaContainer::save()
{
   status.saveSettings();
   imports.saveSettings();
   logView.saveSettings();
   statisticsView.saveSettings();
   config.sync();
}

