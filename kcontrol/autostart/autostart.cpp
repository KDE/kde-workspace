/***************************************************************************
 *   Copyright (C) 2006-2007 by Stephen Leaf                               *
 *   smileaf@gmail.com                                                     *
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "autostart.h"
#include "autostartitem.h"
#include "addscriptdialog.h"
#include "advanceddialog.h"

#include <QDir>
#include <QTreeWidget>
#include <QStringList>

#include <KLocale>
#include <KConfig>
#include <KConfigGroup>
#include <KGlobalSettings>
#include <KShell>
#include <KStandardDirs>
#include <KOpenWithDialog>
#include <KPropertiesDialog>
#include <KDesktopFile>
#include <KMessageBox>
#include <KAboutData>
#include <KDebug>
#include <KIO/NetAccess>
#include <KIO/DeleteJob>
#include <KIO/CopyJob>


K_PLUGIN_FACTORY(AutostartFactory, registerPlugin<Autostart>();)
    K_EXPORT_PLUGIN(AutostartFactory( "kcmautostart", "kcm_autostart" ))

    Autostart::Autostart( QWidget* parent, const QVariantList& )
        : KCModule( AutostartFactory::componentData(), parent )
{
    widget = new Ui_AutostartConfig();
    widget->setupUi(this);

    QStringList lstHeader;
    lstHeader << i18n( "Name" )
              << i18n( "Command" )
              << i18n( "Status" )
              << i18nc("@title:column The name of the column that decides if the program is run on kde startup, on kde shutdown, etc", "Run On" );
    widget->listCMD->setHeaderLabels(lstHeader);
    widget->listCMD->setFocus();

    setButtons(Help);

    connect( widget->btnAddScript, SIGNAL(clicked()), SLOT(slotAddScript()) );
    connect( widget->btnAddProgram, SIGNAL(clicked()), SLOT(slotAddProgram()) );
    connect( widget->btnRemove, SIGNAL(clicked()), SLOT(slotRemoveCMD()) );
    connect( widget->btnAdvanced, SIGNAL(clicked()), SLOT(slotAdvanced()) );
    connect( widget->listCMD, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(slotEditCMD(QTreeWidgetItem*)) );
    connect( widget->listCMD, SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(slotItemClicked(QTreeWidgetItem*,int)) );
    connect( widget->btnProperties, SIGNAL(clicked()), SLOT(slotEditCMD()) );
    connect( widget->listCMD, SIGNAL(itemSelectionChanged()), SLOT(slotSelectionChanged()) );


    KAboutData* about = new KAboutData("Autostart", 0, ki18n("KDE Autostart Manager"), "1.0",
                                       ki18n("KDE Autostart Manager Control Panel Module"),
                                       KAboutData::License_GPL,
                                       ki18n("Copyright © 2006–2010 Autostart Manager team"));
    about->addAuthor(ki18n("Stephen Leaf"), KLocalizedString(), "smileaf@gmail.com");
    about->addAuthor(ki18n("Montel Laurent"), ki18n( "Maintainer" ), "montel@kde.org");
    setAboutData( about );

}

Autostart::~Autostart()
{
   delete widget;
}


void Autostart::slotItemClicked( QTreeWidgetItem *item, int col)
{
    if ( item && col == COL_STATUS ) {
        DesktopStartItem *entry = dynamic_cast<DesktopStartItem*>( item );
        if ( entry ) {
            bool disable = ( item->checkState( col ) == Qt::Unchecked );
            KDesktopFile kc(entry->fileName().path());
            KConfigGroup grp = kc.desktopGroup();
            if ( grp.hasKey( "Hidden" ) && !disable) {
                grp.deleteEntry( "Hidden" );
            }
            else
                grp.writeEntry("Hidden", disable);

            kc.sync();
            if ( disable )
                item->setText( COL_STATUS, i18nc( "The program won't be run", "Disabled" ) );
            else
                item->setText( COL_STATUS, i18nc( "The program will be run", "Enabled" ) );
        }
    }
}

void Autostart::addItem( DesktopStartItem* item, const QString& name, const QString& run, const QString& command, bool disabled )
{
    Q_ASSERT( item );
    item->setText( COL_NAME, name );
    item->setText( COL_RUN, run );
    item->setText( COL_COMMAND, command );
    item->setCheckState( COL_STATUS, disabled ? Qt::Unchecked : Qt::Checked );
    item->setText( COL_STATUS, disabled ? i18nc( "The program won't be run", "Disabled" ) : i18nc( "The program will be run", "Enabled" ));
}

void Autostart::addItem(ScriptStartItem* item, const QString& name, const QString& command, ScriptStartItem::ENV type )
{
    Q_ASSERT( item );
    item->setText( COL_NAME, name );
    item->setText( COL_COMMAND, command );
    item->changeStartup( type );
}


void Autostart::load()
{
    // share/autostart may *only* contain .desktop files
    // shutdown and env may *only* contain scripts, links or binaries
    // autostart on the otherhand may contain all of the above.
    // share/autostart is special as it overrides entries found in $KDEDIR/share/autostart
    m_paths << KGlobalSettings::autostartPath()	// All new entries should go here
            << componentData().dirs()->localkdedir() + "shutdown/"
            << componentData().dirs()->localkdedir() + "env/"
            << componentData().dirs()->localkdedir() + "share/autostart/"	// For Importing purposes
            << componentData().dirs()->localxdgconfdir() + "autostart/" ; //xdg-config autostart dir

    // share/autostart shouldn't be an option as this should be reserved for global autostart entries
    m_pathName << i18n("Startup")
             << i18n("Shutdown")
             << i18n("Pre-KDE startup")
        ;
    widget->listCMD->clear();

    m_programItem = new QTreeWidgetItem( widget->listCMD );
    m_programItem->setText( 0, i18n( "Desktop File" ));
    m_programItem->setFlags(m_programItem->flags()^Qt::ItemIsSelectable );

    QFont boldFont =  m_programItem->font(0);
    boldFont.setBold( true );
    m_programItem->setData ( 0, Qt::FontRole, boldFont );

    m_scriptItem = new QTreeWidgetItem( widget->listCMD );
    m_scriptItem->setText( 0, i18n( "Script File" ));
    m_scriptItem->setFlags(m_scriptItem->flags()^Qt::ItemIsSelectable);
    m_scriptItem->setData ( 0, Qt::FontRole, boldFont);

    widget->listCMD->expandItem( m_programItem );
    widget->listCMD->expandItem( m_scriptItem );

    foreach (const QString& path, m_paths) {
        if (! KStandardDirs::exists(path))
            KStandardDirs::makeDir(path);

        QDir autostartdir( path );
        autostartdir.setFilter( QDir::Files );
        const QFileInfoList list = autostartdir.entryInfoList();
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fi = list.at(i);
            QString filename = fi.fileName();
            bool desktopFile = filename.endsWith(".desktop");
            if ( desktopFile )
            {
                KDesktopFile config(fi.absoluteFilePath());
                //kDebug() << fi.absoluteFilePath() << "trying" << config.desktopGroup().readEntry("Exec");
                QStringList commandLine = KShell::splitArgs(config.desktopGroup().readEntry("Exec"));
                if (commandLine.isEmpty()) {
                    continue;
                }

                const QString exe = commandLine.first();
                if (exe.isEmpty() || KStandardDirs::findExe(exe).isEmpty()) {
                    continue;
                }

                DesktopStartItem *item = new DesktopStartItem( fi.absoluteFilePath(), m_programItem, this );

                const KConfigGroup grp = config.desktopGroup();
                const bool hidden = grp.readEntry("Hidden", false);
                const QStringList notShowList = grp.readXdgListEntry("NotShowIn");
                const QStringList onlyShowList = grp.readXdgListEntry("OnlyShowIn");

                const bool disabled = hidden ||
                                      notShowList.contains("KDE") ||
                                      (!onlyShowList.isEmpty() && !onlyShowList.contains("KDE"));

                int indexPath = m_paths.indexOf((item->fileName().directory()+'/' ) );
                if ( indexPath > 2 )
                    indexPath = 0; //.kde/share/autostart and .config/autostart load destkop at startup
                addItem(item, config.readName(), m_pathName.value(indexPath),  grp.readEntry("Exec"), disabled );
            }
            else
            {
                ScriptStartItem *item = new ScriptStartItem( fi.absoluteFilePath(), m_scriptItem,this );
                int typeOfStartup = m_paths.indexOf((item->fileName().directory()+'/') );
                ScriptStartItem::ENV type = ScriptStartItem::START;
                switch( typeOfStartup )
                {
                case 0:
                    type =ScriptStartItem::START;
                    break;
                case 1:
                    type = ScriptStartItem::SHUTDOWN;
                    break;
                case 2:
                    type = ScriptStartItem::PRE_START;
                    break;
                default:
                    kDebug()<<" type is not defined :"<<type;
                    break;
                }
                if ( fi.isSymLink() ) {
                    QString link = fi.readLink();
                    addItem(item, filename, link, type );
                }
                else
                {
                    addItem( item, filename, filename,type );
                }
            }
        }
    }
    //Update button
    slotSelectionChanged();
    widget->listCMD->resizeColumnToContents(COL_NAME);
    //widget->listCMD->resizeColumnToContents(COL_COMMAND);
    widget->listCMD->resizeColumnToContents(COL_STATUS);
    widget->listCMD->resizeColumnToContents(COL_RUN);
}

void Autostart::slotAddProgram()
{
    KOpenWithDialog owdlg( this );
    if (owdlg.exec() != QDialog::Accepted)
        return;

    KService::Ptr service = owdlg.service();

    Q_ASSERT(service);
    if (!service) {
        return; // Don't crash if KOpenWith wasn't able to create service.
    }

    // It is important to ensure that we make an exact copy of an existing
    // desktop file (if selected) to enable users to override global autostarts.
    // Also see
    // https://bugs.launchpad.net/ubuntu/+source/kde-workspace/+bug/923360
    QString desktopPath;
    KUrl desktopTemplate;
    if ( service->desktopEntryName().isEmpty() ) {
        // Build custom desktop file (e.g. when the user entered an executable
        // name in the OpenWithDialog).
        desktopPath = m_paths[4] + service->name() + ".desktop";
        desktopTemplate = KUrl( desktopPath );
        KConfig kc(desktopTemplate.path(), KConfig::SimpleConfig);
        KConfigGroup kcg = kc.group("Desktop Entry");
        kcg.writeEntry("Exec",service->exec());
        kcg.writeEntry("Icon","system-run");
        kcg.writeEntry("Path","");
        kcg.writeEntry("Terminal",false);
        kcg.writeEntry("Type","Application");
        kc.sync();

        KPropertiesDialog dlg( desktopTemplate, this );
        if ( dlg.exec() != QDialog::Accepted )
        {
            return;
        }
    }
    else
    {
        // Use existing desktop file and use same file name to enable overrides.
        desktopPath = m_paths[4] + service->desktopEntryName() + ".desktop";
        desktopTemplate = KUrl( KStandardDirs::locate("apps", service->entryPath()) );

        KPropertiesDialog dlg( desktopTemplate, KUrl(m_paths[4]), service->desktopEntryName() + ".desktop", this );
        if ( dlg.exec() != QDialog::Accepted )
            return;
    }
    DesktopStartItem * item = new DesktopStartItem( desktopPath, m_programItem,this );
    addItem( item, service->name(), m_pathName[0],  service->exec() , false);
}

void Autostart::slotAddScript()
{
    AddScriptDialog * addDialog = new AddScriptDialog(this);
    int result = addDialog->exec();
    if (result == QDialog::Accepted) {
        if (addDialog->symLink())
            KIO::link(addDialog->importUrl(), m_paths[0]);
        else
            KIO::copy(addDialog->importUrl(), m_paths[0]);

        ScriptStartItem * item = new ScriptStartItem( m_paths[0] + addDialog->importUrl().fileName(), m_scriptItem,this );
        addItem( item,  addDialog->importUrl().fileName(), addDialog->importUrl().fileName(),ScriptStartItem::START );
    }
    delete addDialog;
}

void Autostart::slotRemoveCMD()
{
    QTreeWidgetItem* item = widget->listCMD->currentItem();
    if (!item)
        return;
    DesktopStartItem *startItem = dynamic_cast<DesktopStartItem*>( item );
    if ( startItem )
    {
        m_programItem->takeChild( m_programItem->indexOfChild( startItem ) );
        KIO::del(startItem->fileName().path() );
        delete item;
    }
    else
    {
        ScriptStartItem * scriptItem = dynamic_cast<ScriptStartItem*>( item );
        if ( scriptItem )
        {
            m_scriptItem->takeChild( m_scriptItem->indexOfChild( scriptItem ) );
            KIO::del(scriptItem->fileName().path() );
            delete item;
        }
    }
}

void Autostart::slotEditCMD(QTreeWidgetItem* ent)
{
    if (!ent) return;
    AutoStartItem *entry = dynamic_cast<AutoStartItem*>( ent );
    if ( entry )
    {
        const KFileItem kfi = KFileItem( KFileItem::Unknown, KFileItem::Unknown, KUrl( entry->fileName() ), true );
        if (! slotEditCMD( kfi ))
            return;
        DesktopStartItem *desktopEntry = dynamic_cast<DesktopStartItem*>( entry );
        if (desktopEntry) {
            KService service(desktopEntry->fileName().path());
            addItem( desktopEntry, service.name(), m_pathName.value(m_paths.indexOf((desktopEntry->fileName().directory()+'/') )), service.exec(),false );
        }
    }
}

bool Autostart::slotEditCMD( const KFileItem &item)
{
    KPropertiesDialog dlg( item, this );
    bool c = ( dlg.exec() == QDialog::Accepted );
    return c;
}

void Autostart::slotEditCMD()
{
    if ( widget->listCMD->currentItem() == 0 )
        return;
    slotEditCMD( (AutoStartItem*)widget->listCMD->currentItem() );
}

void Autostart::slotAdvanced()
{
    if ( widget->listCMD->currentItem() == 0 )
        return;

    DesktopStartItem *entry = static_cast<DesktopStartItem *>( widget->listCMD->currentItem() );
    KDesktopFile kc(entry->fileName().path());
    KConfigGroup grp = kc.desktopGroup();
    bool status = false;
    QStringList lstEntry;
    if (grp.hasKey("OnlyShowIn"))
    {
        lstEntry = grp.readXdgListEntry("OnlyShowIn");
        status = lstEntry.contains("KDE");
    }

    AdvancedDialog *dlg = new AdvancedDialog( this,status );
    if ( dlg->exec() )
    {
        status = dlg->onlyInKde();
        if ( lstEntry.contains( "KDE" ) && !status )
        {
            lstEntry.removeAll( "KDE" );
            grp.writeXdgListEntry( "OnlyShowIn", lstEntry );
        }
        else if ( !lstEntry.contains( "KDE" ) && status )
        {
            lstEntry.append( "KDE" );
            grp.writeXdgListEntry( "OnlyShowIn", lstEntry );
        }
    }
    delete dlg;
}

void Autostart::slotChangeStartup( ScriptStartItem* item, int index )
{
    Q_ASSERT(item);

    if ( item )
    {
        item->setPath(m_paths.value(index));
        widget->listCMD->setCurrentItem( item );
        if ( ( index == 2 ) && !item->fileName().path().endsWith( ".sh" ))
            KMessageBox::information( this, i18n( "Only files with “.sh” extensions are allowed for setting up the environment." ) );

    }
}

void Autostart::slotSelectionChanged()
{
    const bool hasItems = ( dynamic_cast<AutoStartItem*>( widget->listCMD->currentItem() )!=0 ) ;
    widget->btnRemove->setEnabled(hasItems);

    const bool isDesktopItem = (dynamic_cast<DesktopStartItem*>(widget->listCMD->currentItem() ) != 0) ;
    widget->btnProperties->setEnabled(isDesktopItem);
    widget->btnAdvanced->setEnabled(isDesktopItem) ;
}

void Autostart::defaults()
{
}

void Autostart::save()
{
}

#include "autostart.moc"
