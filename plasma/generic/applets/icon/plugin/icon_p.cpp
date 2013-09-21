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