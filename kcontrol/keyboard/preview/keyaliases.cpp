/*
 *  Copyright (C) 2012 Shivam Makkar (amourphious1992@gmail.com)
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

#include "keyaliases.h"
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtGui/QMessageBox>
#include <QtCore/QFile>
#include <QtCore/QDir>

#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#include <fixx11h.h>
#include <config-workspace.h>


Aliases::Aliases()
{
    QString filename=findaliasdir();
    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = file.readAll();
    file.close();
    QList<QString>als;
    als=content.split("xkb_keycodes");
    for(int i=1;i<als.size();i++){
        QString temp=als.at(i);
        temp=temp.remove(" ");
        temp=temp.remove("\n");
        temp=temp.remove("\"");
        temp=temp.remove(">");
        temp=temp.remove("<");
        temp=temp.remove(";");
        temp=temp.remove("}");
        temp=temp.remove("{");
        QList<QString>alskeys;
        alskeys=temp.split("alias");
        if(temp.startsWith("qwerty")){
            for(int k=1;k<alskeys.size();k++){
                QString tmp=alskeys.at(k);
                int inofeq=tmp.indexOf("=");
                QString lat=tmp.left(inofeq);
                QString key=tmp.mid(inofeq+1);
                qwerty[lat]=key;
            }
        }
        if(temp.startsWith("azerty")){
            for(int k=1;k<alskeys.size();k++){
                QString tmp=alskeys.at(k);
                int inofeq=tmp.indexOf("=");
                QString lat=tmp.left(inofeq);
                QString key=tmp.mid(inofeq+1);
                azerty[lat]=key;
            }
      }
       if(temp.startsWith("qwertz")){
            for(int k=1;k<alskeys.size();k++){
                 QString tmp=alskeys.at(k);
                 int inofeq=tmp.indexOf("=");
                 QString lat=tmp.left(inofeq);
                 QString key=tmp.mid(inofeq+1);
                 qwertz[lat]=key;
            }
       }
   }

}

QString Aliases::getAlias(const QString& cname, const QString& name)
{
    QMessageBox q;
    QString a=name;
    if(cname=="ma"){
        a=azerty.value(name);
    }
    else{
        a=qwerty.value(name);
    }
    return a;
}

QString Aliases::findaliasdir(){

    QString aliasdir;
    QString xkbParentDir;

    QString base(XLIBDIR);
    if( base.count('/') >= 3 ) {
        // .../usr/lib/X11 -> /usr/share/X11/xkb vs .../usr/X11/lib -> /usr/X11/share/X11/xkb
        QString delta = base.endsWith("X11") ? "/../../share/X11" : "/../share/X11";
        QDir baseDir(base + delta);
        if( baseDir.exists() ) {
            xkbParentDir = baseDir.absolutePath();
        }
        else {
            QDir baseDir(base + "/X11");	// .../usr/X11/lib/X11/xkb (old XFree)
            if( baseDir.exists() ) {
                xkbParentDir = baseDir.absolutePath();
            }
        }
    }

    if( xkbParentDir.isEmpty() ) {
        xkbParentDir = "/usr/share/X11";
    }
    aliasdir=QString("%1/xkb/keycodes/aliases").arg(xkbParentDir);
    return(aliasdir);
}
