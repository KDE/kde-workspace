/*
 *  lookandfeeltab.cpp
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2000 Aaron J. Seigo <aseigo@olympusproject.org>
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

#include <qlayout.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qimage.h>

#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kimageio.h>
#include <kiconeffect.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <klineedit.h>

#include "lookandfeeltab_impl.h"
#include "lookandfeeltab_impl.moc"


extern int kickerconfig_screen_number;

LookAndFeelTab::LookAndFeelTab( QWidget *parent, const char* name )
  : LookAndFeelTabBase (parent, name)
{
  connect(m_zoom_cb, SIGNAL(clicked()), SIGNAL(changed()));
  connect(m_showToolTips, SIGNAL(clicked()), SIGNAL(changed()));
  
  connect(m_kmenuTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_desktopTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_browserTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_urlTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_exeTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_wlTile, SIGNAL(activated(int)), SIGNAL(changed()));

  connect(m_backgroundImage, SIGNAL(clicked()), SIGNAL(changed()));

  m_backgroundInput->fileDialog()->setFilter(KImageIO::pattern(KImageIO::Reading));
  m_backgroundInput->fileDialog()->setCaption(i18n("Select an Image File"));
  m_backgroundInput->lineEdit()->setReadOnly(true);

  connect(m_backgroundInput, SIGNAL(urlSelected(const QString&)), SLOT(browse_theme(const QString&)));
  connect(m_showToolTips, SIGNAL(clicked()), SIGNAL(changed()));

  fillTileCombos();
}

void LookAndFeelTab::browse_theme(const QString& newtheme)
{
    if (theme == newtheme) return;
    if (newtheme.isEmpty()) return;

    QImage tmpImg(newtheme);
    if( !tmpImg.isNull() ) {
        tmpImg = tmpImg.smoothScale(m_backgroundLabel->contentsRect().width(),
                                    m_backgroundLabel->contentsRect().height());
        theme_preview.convertFromImage(tmpImg);
        if( !theme_preview.isNull() ) {
            theme = newtheme;
            m_backgroundInput->lineEdit()->setText(theme);
            m_backgroundLabel->setPixmap(theme_preview);
            emit changed();
            return;
        }
    }

    KMessageBox::error(this, i18n("Failed to load image file."), i18n("Failed to Load Image File"));
}

int LookAndFeelTab::findComboEntry(QComboBox* combo, const QString& searchFor)
{
    int index = 0;

    for (int i = 0; i < combo->count(); i++) 
    {
      if (searchFor == combo->text(i))
      {
        index = i;
        break;
      }
    }

    return index;
}

void LookAndFeelTab::load()
{
  QCString configname;
  if (kickerconfig_screen_number == 0)
      configname = "kickerrc";
  else
      configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
  KConfig c(configname, false, false);

  c.setGroup("General");

  bool use_theme = c.readBoolEntry("UseBackgroundTheme", false);
  theme = c.readEntry("BackgroundTheme", QString::null).stripWhiteSpace();

  m_backgroundImage->setChecked(use_theme);
  m_backgroundInput->setEnabled(use_theme);
  m_backgroundLabel->setEnabled(use_theme);

  if (theme.length() > 0)
  {
    QString themepath;
    if (theme[0] == '/')
        themepath = theme;
    else
        themepath = locate("data", "kicker/"+theme);
    QImage tmpImg(themepath);
    if(!tmpImg.isNull())
    {
        tmpImg = tmpImg.smoothScale(m_backgroundLabel->contentsRect().width(),
                                    m_backgroundLabel->contentsRect().height());
        theme_preview.convertFromImage(tmpImg);
        if(!theme_preview.isNull()) {
            m_backgroundInput->lineEdit()->setText(theme);
            m_backgroundLabel->setPixmap(theme_preview);
        }
        else
        {
            m_backgroundInput->lineEdit()->setText(i18n("Error loading theme image file. '%1' '%2'")
                                                                                .arg(theme).arg(themepath));
        }
    }
    else
    {
        m_backgroundInput->lineEdit()->setText(i18n("Error loading theme image file. '%1' '%2'")
                                                                            .arg(theme).arg(themepath));
    }
  }

  m_showToolTips->setChecked( c.readBoolEntry( "ShowToolTips", true ) );
  m_fadeOutAppletHandles=c.readBoolEntry( "FadeOutAppletHandles", false);

  c.setGroup("buttons");

  bool zoom = c.readBoolEntry("EnableIconZoom", false);
  m_zoom_cb->setChecked(zoom);

  QString tile;
  c.setGroup("buttons");

  if (c.readBoolEntry("EnableTileBackground", false))
  {
    c.setGroup("button_tiles");

    if (c.readBoolEntry("EnableKMenuTiles", false))
    {
      tile = c.readEntry("KMenuTile", "solid_blue");
      m_kmenuTile->setCurrentItem(findComboEntry(m_kmenuTile, tile));
    }
    else
    {
      m_kmenuTile->setCurrentItem(0);
    }

    if (c.readBoolEntry("EnableDesktopButtonTiles", false))
    {
      tile = c.readEntry("DesktopButtonTile", "solid_orange");
      m_desktopTile->setCurrentItem(findComboEntry(m_desktopTile, tile));
    }
    else
    {
      m_desktopTile->setCurrentItem(0);
    }

    if (c.readBoolEntry("EnableURLTiles", false))
    {
      tile = c.readEntry("URLTile", "solid_gray");
      m_urlTile->setCurrentItem(findComboEntry(m_urlTile, tile));
    }
    else
    {
      m_urlTile->setCurrentItem(0);
    }

    if (c.readBoolEntry("EnableBrowserTiles", false))
    {
      tile = c.readEntry("BrowserTile", "solid_green");
      m_browserTile->setCurrentItem(findComboEntry(m_browserTile, tile));
    }
    else
    {
      m_browserTile->setCurrentItem(0);
    }

    if (c.readBoolEntry("EnableExeTiles", false))
    {
      tile = c.readEntry("ExeTile", "solid_red");
      m_exeTile->setCurrentItem(findComboEntry(m_exeTile, tile));
    }
    else
    {
      m_exeTile->setCurrentItem(0);
    }

    if (c.readBoolEntry("EnableWindowListTiles", false))
    {
      tile = c.readEntry("WindowListTile", "solid_green");
      m_wlTile->setCurrentItem(findComboEntry(m_wlTile, tile));
    }
    else
    {
      m_wlTile->setCurrentItem(0);
    }
  }
  else
  {
    m_kmenuTile->setCurrentItem(0);
    m_desktopTile->setCurrentItem(0);
    m_urlTile->setCurrentItem(0);
    m_browserTile->setCurrentItem(0);
    m_exeTile->setCurrentItem(0);
    m_wlTile->setCurrentItem(0);
  }
}

void LookAndFeelTab::save()
{
  QCString configname;
  if (kickerconfig_screen_number == 0)
      configname = "kickerrc";
  else
      configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
  KConfig c(configname, false, false);

  c.setGroup("General");
  c.writeEntry("UseBackgroundTheme", m_backgroundImage->isChecked());
  c.writeEntry("BackgroundTheme", theme);
  c.writeEntry( "ShowToolTips", m_showToolTips->isChecked() );
  c.writeEntry( "FadeOutAppletHandles", m_fadeOutAppletHandles );

  c.setGroup("button_tiles");
  bool enableTiles = false;
  int tile = m_kmenuTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnableKMenuTiles", tile > 0);
    c.writeEntry("KMenuTile", m_kmenuTile->currentText());
  }
  else
  {
    c.writeEntry("EnableKMenuTiles", false);  
  }

  tile = m_desktopTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnableDesktopButtonTiles", tile > 0);
    c.writeEntry("DesktopButtonTile", m_desktopTile->currentText());
  }
  else
  {
    c.writeEntry("EnableDesktopButtonTiles", false);  
  }
  
  tile = m_urlTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnableURLTiles", tile > 0);
    c.writeEntry("URLTile", m_urlTile->currentText());
  }
  else
  {
    c.writeEntry("EnableURLTiles", false);  
  }

  tile = m_browserTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnablEnableBrowserTiles", tile > 0);
    c.writeEntry("BrowserTile", m_browserTile->currentText());
  }
  else
  {
    c.writeEntry("EnableBrowserTiles", false);  
  }

  tile = m_exeTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnableExeTiles", tile > 0);
    c.writeEntry("ExeTile", m_exeTile->currentText());
  }
  else
  {
    c.writeEntry("EnableExeTiles", false);  
  }

  tile = m_wlTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnableWindowListTiles", tile > 0);
    c.writeEntry("WindowListTile", m_wlTile->currentText());
  }
  else
  {
    c.writeEntry("EnableWindowListTiles", false);  
  }

  c.setGroup("buttons");
  c.writeEntry("EnableTileBackground", enableTiles);
  c.writeEntry("EnableIconZoom", m_zoom_cb->isChecked());
  
  c.sync();
}

void LookAndFeelTab::defaults()
{
  m_zoom_cb->setChecked(true);
  m_showToolTips->setChecked(true);

  m_kmenuTile->setCurrentItem(0);
  m_urlTile->setCurrentItem(0);
  m_browserTile->setCurrentItem(0);
  m_exeTile->setCurrentItem(0);
  m_wlTile->setCurrentItem(0);
  m_desktopTile->setCurrentItem(0);

  theme = QString::null;

  m_backgroundImage->setChecked(false);
  m_backgroundInput->lineEdit()->setText(theme);
  m_backgroundLabel->clear();

  m_backgroundInput->setEnabled(false);
  m_backgroundLabel->setEnabled(false);
}

void LookAndFeelTab::fillTileCombos()
{
  m_kmenuTile->clear();
  m_kmenuTile->insertItem(i18n("No Tile"));
  m_desktopTile->clear();
  m_desktopTile->insertItem(i18n("No Tile"));
  m_urlTile->clear();
  m_urlTile->insertItem(i18n("No Tile"));
  m_browserTile->clear();
  m_browserTile->insertItem(i18n("No Tile"));
  m_exeTile->clear();
  m_exeTile->insertItem(i18n("No Tile"));
  m_wlTile->clear();
  m_wlTile->insertItem(i18n("No Tile"));

  QStringList list = KGlobal::dirs()->findAllResources("tiles","*_tiny_up.png");
  int minHeight = 0;

  for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
  {
    QString tile = (*it);
    QPixmap pix(tile);
    QFileInfo fi(tile);
    tile = fi.fileName();
    tile.truncate(tile.find("_tiny_up.png"));

    m_kmenuTile->insertItem(pix, tile);
    m_desktopTile->insertItem(pix, tile);
    m_urlTile->insertItem(pix, tile);
    m_browserTile->insertItem(pix, tile);
    m_exeTile->insertItem(pix, tile);
    m_wlTile->insertItem(pix, tile);
    
    if (pix.height() > minHeight)    
    {
        minHeight = pix.height();
    }
  }
  
  minHeight += 6;
  m_kmenuTile->setMinimumHeight(minHeight);
  m_desktopTile->setMinimumHeight(minHeight);
  m_urlTile->setMinimumHeight(minHeight);
  m_browserTile->setMinimumHeight(minHeight);
  m_exeTile->setMinimumHeight(minHeight);
  m_wlTile->setMinimumHeight(minHeight);
}
