/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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

#include <qtabwidget.h>
#include <qlayout.h>
#include <qradiobutton.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kimageio.h>

#include <dcopclient.h>

#include "main.h"
#include "main.moc"
#include "positiontab_impl.h"
#include "hidingtab_impl.h"
//#include "menutab_impl.h"
//#include "lookandfeeltab_impl.h"
//#include "applettab_impl.h"

#include "lookandfeeltab_kcm.h"
#include "menutab_kcm.h"

#include <X11/Xlib.h>
#include <kaboutdata.h>


// for multihead
int kickerconfig_screen_number = 0;


KickerConfig::KickerConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
    m_extensionInfo.setAutoDelete(true);

    if (qt_xdisplay())
    kickerconfig_screen_number = DefaultScreen(qt_xdisplay());

    QCString configname;
    if (kickerconfig_screen_number == 0)
        configname = "kickerrc";
    else
        configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);

    c->setGroup("General");
    m_extensionInfo.append(new extensionInfo(QString::null, configname));
    QStringList elist = c->readListEntry("Extensions2");
    for (QStringList::Iterator it = elist.begin(); it != elist.end(); ++it)
    {
        // extension id
        QString extensionId(*it);
        QString group = extensionId;

        // is there a config group for this extension?
        if(!c->hasGroup(group))
            continue;

        // create a matching applet container
        if (!extensionId.contains("Extension") > 0)
            continue;

        // set config group
        c->setGroup(group);

        QString df = KGlobal::dirs()->findResource("extensions", c->readEntry("DesktopFile"));
        QString cf = c->readEntry("ConfigFile");
        m_extensionInfo.append(new extensionInfo(df, cf));
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    tab = new QTabWidget(this);
    layout->addWidget(tab);

    positiontab = new PositionTab(this);
    tab->addTab(positiontab, i18n("Arran&gement"));
    connect(positiontab, SIGNAL(changed()), this, SLOT(configChanged()));

    hidingtab = new HidingTab(this);
    tab->addTab(hidingtab, i18n("&Hiding"));
    connect(hidingtab, SIGNAL(changed()), this, SLOT(configChanged()));

//    lookandfeeltab = new LookAndFeelTab(this);
//    tab->addTab(lookandfeeltab, i18n("A&ppearance"));
//    connect(lookandfeeltab, SIGNAL(changed()), this, SLOT(configChanged()));

//    menutab = new MenuTab(this);
//    tab->addTab(menutab, i18n("&Menus"));
//    connect(menutab, SIGNAL(changed()), this, SLOT(configChanged()));

    load();

    QObject::connect(positiontab->m_panelList, SIGNAL(selectionChanged(QListViewItem*)),
                     this, SLOT(positionPanelChanged(QListViewItem*)));
    QObject::connect(hidingtab->m_panelList, SIGNAL(selectionChanged(QListViewItem*)),
                     this, SLOT(hidingPanelChanged(QListViewItem*)));

    //applettab = new AppletTab(this);
    //tab->addTab(applettab, i18n("&Applets"));
    //connect(applettab, SIGNAL(changed()), this, SLOT(configChanged()));
}

void KickerConfig::configChanged()
{
    emit changed(true);
}

void KickerConfig::load()
{
    positiontab->load();
    hidingtab->load();
    //menutab->load();
    //lookandfeeltab->load();
    //applettab->load();
    emit changed(false);
}

void KickerConfig::save()
{
    positiontab->save();
    hidingtab->save();
    //menutab->save();
    //lookandfeeltab->save();
    //applettab->save();

    emit changed(false);

    // Tell kicker about the new config file.
    if (!kapp->dcopClient()->isAttached())
        kapp->dcopClient()->attach();
    QByteArray data;

    QCString appname;
    if (kickerconfig_screen_number == 0)
	appname = "kicker";
    else
	appname.sprintf("kicker-screen-%d", kickerconfig_screen_number);
    kapp->dcopClient()->send( appname, "kicker", "configure()", data );
}

void KickerConfig::defaults()
{
    positiontab->defaults();
    hidingtab->defaults();
    //menutab->defaults();
    //lookandfeeltab->defaults();
    //applettab->defaults();

    emit changed(true);
}

QString KickerConfig::quickHelp() const
{
    return i18n("<h1>Panel</h1> Here you can configure the KDE panel (also"
                " referred to as 'kicker'). This includes options like the position and"
                " size of the panel, as well as its hiding behavior and its looks.<p>"
                " Note that you can also access some of these options directly by clicking"
                " on the panel, e.g. dragging it with the left mouse button or using the"
                " context menu on right mouse button click. This context menu also offers you"
                " manipulation of the panel's buttons and applets.");
}

const KAboutData* KickerConfig::aboutData() const
{

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmkicker"), I18N_NOOP("KDE Panel Control Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 1999 - 2001 Matthias Elter\n(c) 2002 Aaron J. Seigo"));

    about->addAuthor("Matthias Elter", 0, "elter@kde.org");
    about->addAuthor("Aaron J. Seigo", 0, "aseigo@olympusproject.org");

    return about;
}

void KickerConfig::populateExtensionInfoList(QListView* list)
{
    extensionInfoItem* last(0);
    for (QPtrListIterator<extensionInfo> it(m_extensionInfo); it; ++it)
    {
       last = new extensionInfoItem(*it, list, last);
    }
}

const extensionInfoList& KickerConfig::extensionsInfo()
{
    return m_extensionInfo;
}

void KickerConfig::reloadExtensionInfo()
{
    for (QPtrListIterator<extensionInfo> it(m_extensionInfo); it; ++it)
    {
       (*it)->load();
    }

    emit extensionInfoChanged();
}

void KickerConfig::saveExtentionInfo()
{
    for (QPtrListIterator<extensionInfo> it(m_extensionInfo); it; ++it)
    {
       (*it)->save();
    }
}

void KickerConfig::positionPanelChanged(QListViewItem* item)
{
    if (!item)
    {
        return;
    }

    extensionInfo* info = static_cast<extensionInfoItem*>(item)->info();
    extensionInfoItem* hidingItem =
        static_cast<extensionInfoItem*>(hidingtab->m_panelList->firstChild());

    while (hidingItem)
    {
        if (hidingItem->info() == info)
        {
            hidingtab->m_panelList->setSelected(hidingItem, true);
            return;
        }

        hidingItem = static_cast<extensionInfoItem*>(hidingItem->nextSibling());
    }
}


void KickerConfig::hidingPanelChanged(QListViewItem* item)
{
    if (!item)
    {
        return;
    }

    extensionInfo* info = static_cast<extensionInfoItem*>(item)->info();
    extensionInfoItem* positionItem =
        static_cast<extensionInfoItem*>(positiontab->m_panelList->firstChild());

    while (positionItem)
    {
        if (positionItem->info() == info)
        {
            positiontab->m_panelList->setSelected(positionItem, true);
            return;
        }

        positionItem = static_cast<extensionInfoItem*>(positionItem->nextSibling());
    }
}

extern "C"
{
    KCModule *create_kicker(QWidget *parent, const char *)
    {
        KGlobal::dirs()->addResourceType("applets", KStandardDirs::kde_default("data") +
                                         "kicker/applets");
        KGlobal::dirs()->addResourceType("extensions", KStandardDirs::kde_default("data") +
                                         "kicker/extensions");
        return new KickerConfig(parent, "kcmkicker");
    };

    KCModule *create_kicker_behaviour(QWidget *parent, const char *)
    {
        KImageIO::registerFormats();
        KGlobal::dirs()->addResourceType("tiles", KStandardDirs::kde_default("data") +
                                         "kicker/tiles");
        KGlobal::dirs()->addResourceType("hb_pics", KStandardDirs::kde_default("data") +
                                         "kcmkicker/pics");
        return new LookAndFeelConfig(parent, "kcmkicker");
    };

    KCModule *create_kicker_menus(QWidget *parent, const char *)
    {
        KImageIO::registerFormats();
        KGlobal::dirs()->addResourceType("applets", KStandardDirs::kde_default("data") +
                                         "kicker/applets");
        KGlobal::dirs()->addResourceType("extensions", KStandardDirs::kde_default("data") +
                                         "kicker/extensions");
        return new MenuConfig(parent, "kcmkicker");
    };
}
