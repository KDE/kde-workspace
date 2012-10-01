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


#include "kbpreviewframe.h"

#include <QtGui/QPainter>
#include <QtGui/QFont>

#include <KApplication>
#include <KLocale>


KbPreviewFrame::KbPreviewFrame(QWidget *parent) :
    QFrame(parent)
{
     setFrameStyle( QFrame::Box );
     setFrameShadow(QFrame::Sunken);
}

void KbPreviewFrame::paintTLDE(QPainter &painter,int &x,int &y){
    const int gr1=0,gr2=1,gr3=2,gr4=3,sz=20,kszx=70,kszy=70;
    const int lv1x=15,lvl2x=40,lvly=10;
    const QString lev12color="#d4d4d4",lev34color="#FF3300";

    painter.drawRect(x,y,kszx,kszy);
    for(int j=0;j<kblayout.TLDE.klst.size();j++){
        if(j==gr2)
            painter.drawText(x+lv1x,y+lvly,sz,sz,Qt::AlignBottom,symbol.getkeyuni(kblayout.TLDE.klst.at(gr2)));
        if(j==gr1)
            painter.drawText(x+lv1x,y+lvl2x,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.TLDE.klst.at(gr1)));
        painter.setPen(QColor(lev34color));
        if(j==gr3)
            painter.drawText(x+lvl2x,y+lvl2x,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.TLDE.klst.at(gr3)));
        if(j==gr4)
            painter.drawText(x+lvl2x,y+lvly,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.TLDE.klst.at(gr4)));
        painter.setPen(QColor(lev12color));
    }

}

void KbPreviewFrame::paintAERow(QPainter &painter,int &x,int &y){

    const int gr1=0,gr2=1,gr3=2,gr4=3,sz=20,kszx=70,kszy=70;
    const int lv1x=15,lvl2x=40,lvly=10;
    const QString lev12color="#d4d4d4",lev34color="#FF3300";
    paintTLDE(painter,x,y);

    const int noAEk=12;
    for(int i=0;i<noAEk;i++){
        x+=kszx;
        painter.drawRect(x,y,kszx,kszy);
        for(int j=0;j<kblayout.AE[i].klst.size();j++){
            if(j==gr2)
                painter.drawText(x+lv1x,y+lvly,sz,sz,Qt::AlignBottom,symbol.getkeyuni(kblayout.AE[i].klst.at(gr2)));
            if(j==gr1)
                painter.drawText(x+lv1x,y+lvl2x,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AE[i].klst.at(gr1)));
            painter.setPen(QColor(lev34color));
            if(j==gr3)
                painter.drawText(x+lvl2x,y+lvl2x,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AE[i].klst.at(gr3)));
            if(j==gr4)
                painter.drawText(x+lvl2x,y+lvly,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AE[i].klst.at(gr4)));
            painter.setPen(QColor(lev12color));

        }
    }
    x+=kszx;

    const int bkspszx=100,bk1x=10,bk1y=20,bk2y=60;

    painter.drawRect(x,y,bkspszx,kszy);
    painter.drawText(x+bk1x,y+bk1y,i18n("<--"));
    painter.drawText(x+bk1x,y+bk2y,i18n("Backspace"));

}

void KbPreviewFrame::paintADRow(QPainter &painter,int &x,int&y){

    const int gr1=0,gr2=1,gr3=2,gr4=3,sz=20,kszx=70,kszy=70;
    const int lv1x=15,lvl2x=40,lvly=10,shify=60;
    const QString lev12color="#d4d4d4",lev34color="#FF3300";

    const int noADk=12;

    const int tabszx=100;
    const int tab1x=20,tab2x=10,tab1y=25,tab2y=65,tab3y=45;

    painter.drawRect(x,y,tabszx,kszy);
    painter.drawText(x+tab1x,y+tab1y,i18n("<--"));
    painter.drawText(x+lv1x,y+tab3y,i18n("TAB"));
    painter.drawText(x+tab2x,y+tab2y,i18n("-->"));
    x+=tabszx;

    for(int i=0;i<noADk;i++){
        painter.drawRect(x,y,kszx,kszy);
        for(int j=0;j<kblayout.AD[i].klst.size();j++){
            if(j==gr2)
                painter.drawText(x+lv1x,y+lvly,sz,sz,Qt::AlignBottom,symbol.getkeyuni(kblayout.AD[i].klst.at(gr2)));
            if(j==gr1)
                painter.drawText(x+lv1x,y+lvl2x,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AD[i].klst.at(gr1)));
            painter.setPen(QColor(lev34color));
            if(j==gr3)
                painter.drawText(x+lvl2x,y+lvl2x,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AD[i].klst.at(gr3)));
            if(j==gr4)
                painter.drawText(x+lvl2x,y+lvly,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AD[i].klst.at(gr4)));
            painter.setPen(QColor(lev12color));
        }
        x+=kszx;
    }
    painter.drawRect(x,y,kszx,kszy);
    painter.drawText(x+sz,y+sz,i18n("|"));
    painter.drawText(x+sz,y+shify,i18n("\\"));
}

void KbPreviewFrame::paintACRow(QPainter &painter,int &x,int &y){

    const int gr1=0,gr2=1,gr3=2,gr4=3,sz=20,kszx=70,kszy=70,capszx=100;
    const int noACk=11;
    const int lv1x=15,lvl2x=40,lvly=10,shifx=10,shify=60,retsz=140,ret1x=50,ret2x=30,ret1y=38,ret2y=43;
    const QString lev12color="#d4d4d4",lev34color="#FF3300";

    painter.drawRect(x,y,capszx,kszy);
    painter.drawText(x+shifx,y+sz,i18n("^"));
    painter.drawText(x+shifx,y+shify,i18n("Caps Lock"));
    x+=capszx;

    for(int i=0;i<noACk;i++){
        painter.drawRect(x,y,kszx,kszy);
        for(int j=0;j<kblayout.AC[i].klst.size();j++){
            if(j==gr2)
                painter.drawText(x+lv1x,y+lvly,sz,sz,Qt::AlignBottom,symbol.getkeyuni(kblayout.AC[i].klst.at(gr2)));
            if(j==gr1)
                painter.drawText(x+lv1x,y+lvl2x,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AC[i].klst.at(gr1)));
            painter.setPen(QColor(lev34color));
            if(j==gr3)
                painter.drawText(x+lvl2x,y+lvl2x,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AC[i].klst.at(gr3)));
            if(j==gr4)
                painter.drawText(x+lvl2x,y+lvly,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AC[i].klst.at(gr4)));
            painter.setPen(QColor(lev12color));
        }
        x+=kszx;
    }
    painter.drawRect(x,y,retsz,kszy);
    painter.drawText(x+ret1x,y+ret1y,i18n("|"));
    painter.drawText(x+ret2x,y+ret2y,i18n("<--"));
    painter.drawText(x+shify,y+lvl2x,i18n("Enter"));


}

void KbPreviewFrame::paintABRow(QPainter &painter,int &x,int &y){

    const int gr1=0,gr2=1,gr3=2,gr4=3,sz=20,kszx=70,kszy=70;
    const int lv1x=15,lvl2x=40,lvly=10;
    const QString lev12color="#d4d4d4",lev34color="#FF3300";

    const int noABk=10;
    for(int i=0;i<noABk;i++){
        painter.drawRect(x,y,kszx,kszy);
        for(int j=0;j<kblayout.AB[i].klst.size();j++){
            if(j==gr2)
                painter.drawText(x+lv1x,y+lvly,sz,sz,Qt::AlignBottom,symbol.getkeyuni(kblayout.AB[i].klst.at(gr2)));
            if(j==gr1)
                painter.drawText(x+lv1x,y+lvl2x,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AB[i].klst.at(gr1)));
            painter.setPen(QColor(lev34color));
            if(j==gr3)
                painter.drawText(x+lvl2x,y+lvl2x,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AB[i].klst.at(gr3)));
            if(j==gr4)
                painter.drawText(x+lvl2x,y+lvly,sz,sz,Qt::AlignTop,symbol.getkeyuni(kblayout.AB[i].klst.at(gr4)));
            painter.setPen(QColor(lev12color));
        }
        x+=kszx;
    }


}

void KbPreviewFrame::paintBottomRow(QPainter &painter,int &x,int &y){

    const int txtx=30,txty=35,ctrlsz=100,altsz=100,spsz=400,kszy=70;
    const QString lev12color="#d4d4d4",lev34color="#FF3300";

    painter.drawRect(x,y,ctrlsz,kszy);
    painter.drawText(x+txtx,y+txty,i18n("Ctrl"));

    x+=ctrlsz;

    painter.drawRect(x,y,altsz,kszy);
    painter.drawText(x+txtx,y+txty,i18n("Alt"));

    x+=altsz;

    painter.drawRect(x,y,spsz,kszy);

    x+=spsz;

    painter.drawRect(x,y,altsz,kszy);
    painter.setPen(QColor(lev34color));
    painter.drawText(x+txtx,y+txty,i18n("AltGr"));
    painter.setPen(QColor(lev12color));

    x+=ctrlsz;

    painter.drawRect(x,y,ctrlsz,kszy);
    painter.drawText(x+txtx,y+txty,i18n("Ctrl"));


}

void KbPreviewFrame::paintFnKeys(QPainter &painter,int &x,int &y){

    const int escsz=50,escx=20,escy=55;
    painter.drawRect(x,y,escsz,escsz);

    painter.drawText(escx,escy,i18n("ESC"));

    const int spacex=50;
    x+=spacex;

    const int fnkeyspace=60,fnkeysizex=50,fnkeysizey=50,fkc=15,fkn=25,fky=30,fnkig=4,fng=3;
    int f=1;

    QString str;

    for(int i=0;i<fng;i++){
        x+=spacex;
        for(int j=0;j<fnkig;j++){
            x+=fnkeyspace;
            painter.drawRect(x,y,fnkeysizex,fnkeysizey);
            painter.drawText(x+fkc,y+fky,i18n("F"));
            painter.drawText(x+fkn,y+fky,str.setNum(f));
            f++;
        }
    }
}

void KbPreviewFrame::paintEvent(QPaintEvent *){

    const QString lev12color="#d4d4d4";
    QPainter painter(this);

    QFont kbfont;
    kbfont.setPointSize(12);

    painter.setFont(kbfont);
    painter.setBrush(QBrush(Qt::darkGray));

    const int strtx=0,strty=0,endx=1390,endy=490,kszy=70;
    const int row1x=10,row1y=30,row2x=10,row2y=90,row5x=10,row5y=330,row3x=10,row3y=170,shifx=10,shify=60,row4x=10,row4y=250,row6x=110,row6y=410;
    const int shiftsz=155;

    painter.drawRect(strtx,strty,endx,endy);

    painter.setPen(QColor(lev12color));
    painter.setBrush(QBrush(Qt::black));

    int x,y;
    x=row1x;
    y=row1y;

    paintFnKeys(painter,x,y);

    x=row2x;
    y=row2y;

    paintAERow(painter,x,y);

    x=row3x;
    y=row3y;

    paintADRow(painter,x,y);

    x=row4x;
    y=row4y;

    paintACRow(painter,x,y);

    x=row5x;
    y=row5y;

    painter.drawRect(x,y,shiftsz,kszy);
    painter.drawText(x+shifx,y+shify,i18n("SHIFT"));
    x+=shiftsz;

    paintABRow(painter,x,y);

    painter.drawRect(x,y,shiftsz,kszy);
    painter.drawText(x+shifx,y+shify,i18n("SHIFT"));

    x=row6x;
    y=row6y;

    paintBottomRow(painter,x,y);

    if(symbol.nill>=120){
        painter.drawRect(strtx,strty,endx,endy);
        const int midx=470,midy=240;
        painter.drawText(midx,midy,i18n("No preview found"));
    }

}



void KbPreviewFrame::getKeyboardLayout(QString country, QString layoutvariant){

    QString filename=kblayout.findSymbolbasedir();
    filename.append(country);

    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = file.readAll();
    file.close();

    QList<QString> symstr;
    symstr=content.split("xkb_symbols ");

    if(layoutvariant=="")
        kblayout.getLayout(symstr.at(1),country);

    for(int i=1;i<symstr.size();i++){
        QString h=symstr.at(i);
        int k=h.indexOf("\"");
        h=h.mid(k);
        k=h.indexOf("{");
        h=h.left(k);
        h=h.remove(" ");
        QString f="\"";
        f.append(layoutvariant);
        f.append("\"");
        f=f.remove(" ");

        if(h==f){
            kblayout.getLayout(symstr.at(i),country);
            break;
        }
    }
}

