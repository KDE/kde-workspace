/*
 * Copyright 2013  Bhushan Shah <bhush94@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "icon_p.h"

#include <QUrl>
#include <QFileInfo>

#include <KFileItem>
#include <KDesktopFile>

QString IconPrivate::getName(const QUrl& url) const
{
    QString m_name;
    if(url.isLocalFile()) {
	KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, url);
	if (fileItem.isDesktopFile()) {
	    KDesktopFile f(url.toLocalFile());
	    m_name = f.readName();
	    if(m_name.isNull()) {
		m_name = QFileInfo(url.toLocalFile()).fileName();
	    }
	}
    }
    return m_name;
}

QString IconPrivate::getIcon(const QUrl& url) const
{
    QString m_icon;
    if(url.isLocalFile()) {
	KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, url);
	if (fileItem.isDesktopFile()) {
	    KDesktopFile f(url.toLocalFile());
	    m_icon = f.readIcon();	    
	}
    }
    return m_icon;
}