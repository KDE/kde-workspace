#include "kbpreviewframe.h"
#include<QPainter>
#include<QFont>
#include <KApplication>
#include <KLocale>


KbPreviewFrame::KbPreviewFrame(QWidget *parent) :
    QFrame(parent)
{
     setFrameStyle( QFrame::Box );
     setFrameShadow(QFrame::Sunken);
}

void KbPreviewFrame::paintEvent(QPaintEvent *){
    const QString lev12color="#d4d4d4",lev34color="#FF3300";

    QPainter painter(this);

    QFont kbfont;
    kbfont.setPointSize(12);

    painter.setFont(kbfont);
    painter.setBrush(QBrush(Qt::darkGray));

    const int strtx=0,strty=0,endx=1390,endy=490;

    painter.drawRect(strtx,strty,endx,endy);

    painter.setPen(QColor(lev12color));
    painter.setBrush(QBrush(Qt::black));

    int x,y;
    const int row1x=10,row1y=30;
    x=row1x;
    y=row1y;

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

    const int lv1x=15,lvl2x=40,lvly=10,sz=20;
    const int kszx=70,kszy=70;

    const int gr1=0,gr2=1,gr3=2,gr4=3;

    const int row2x=10,row2y=90;
    x=row2x;
    y=row2y;


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


    const int row3x=10,row3y=170;
    x=row3x;
    y=row3y;


    const int tabszx=100;
    const int tab1x=20,tab2x=10,tab1y=25,tab2y=65,tab3y=45;

    painter.drawRect(x,y,tabszx,kszy);
    painter.drawText(x+tab1x,y+tab1y,i18n("<--"));
    painter.drawText(x+lv1x,y+tab3y,i18n("TAB"));
    painter.drawText(x+tab2x,y+tab2y,i18n("-->"));
    x+=tabszx;


    const int noADk=12;
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

    const int shifx=10,shify=60;
    painter.drawRect(x,y,kszx,kszy);
    painter.drawText(x+sz,y+sz,i18n("|"));
    painter.drawText(x+sz,y+shify,i18n("\\"));

    const int row4x=10,row4y=250;

    x=row4x;
    y=row4y;

    const int capszx=100;

    painter.drawRect(x,y,capszx,kszy);
    painter.drawText(x+shifx,y+sz,i18n("^"));
    painter.drawText(x+shifx,y+shify,i18n("Caps Lock"));
    x+=capszx;

    const int noACk=11;
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

    const int retsz=140,ret1x=50,ret2x=30,ret1y=38,ret2y=43;

    painter.drawRect(x,y,retsz,kszy);
    painter.drawText(x+ret1x,y+ret1y,i18n("|"));
    painter.drawText(x+ret2x,y+ret2y,i18n("<--"));
    painter.drawText(x+shify,y+lvl2x,i18n("Enter"));

    const int row5x=10,row5y=330;

    x=row5x;
    y=row5y;

    const int shiftsz=155;

    painter.drawRect(x,y,shiftsz,kszy);
    painter.drawText(x+shifx,y+shify,i18n("SHIFT"));
    x+=shiftsz;

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
    painter.drawRect(x,y,shiftsz,kszy);
    painter.drawText(x+shifx,y+shify,i18n("SHIFT"));

    const int row6x=110,row6y=410;
    const int ctrlsz=100,altsz=100,spsz=400;

    x=row6x;
    y=row6y;

    const int txtx=30,txty=35;

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

    if(symbol.nill>=120){
        painter.drawRect(strtx,strty,endx,endy);
        const int midx=470,midy=240;
        painter.drawText(midx,midy,i18n("NO PREVIEW FOUND"));
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

    for(int i=0;i<symstr.size();i++){
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

