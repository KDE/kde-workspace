#include "keyboardpainter.h"
#include "ui_keyboardpainter.h"


keyboardpainter::keyboardpainter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::keyboardpainter)
{
    ui->setupUi(this);
}

keyboardpainter::~keyboardpainter()
{
    delete ui;
}


void keyboardpainter::paintEvent(QPaintEvent *){


    const QString lev12color="#d4d4d4",lev34color="#FF3300";

    setWindowTitle(kblayout.Layoutname);

    QPainter painter(this);

    QFont kbfont("Ubuntu",12);

    painter.setFont(kbfont);
    painter.setBrush(QBrush(Qt::darkGray));

    painter.drawRect(0,20,1030,490);

    painter.setPen(QColor(lev12color));
    painter.setBrush(QBrush(Qt::black));

    int x,y;
    const int row1x=10,row1y=30;
    x=row1x;
    y=row1y;

    const int escsz=50;
    painter.drawRect(x,y,escsz,escsz);
    painter.drawText(20,55,"ESC");

    const int spacex=50;
    x+=spacex;

    const int fnkeyspace=60,fnkeysizex=50,fnkeysizey=50;
    int f=1;

    QString str;

    for(int i=0;i<3;i++){
        x+=spacex;
        for(int j=0;j<4;j++){
            x+=fnkeyspace;
            painter.drawRect(x,y,fnkeysizex,fnkeysizey);
            painter.drawText(x+15,y+30,"F");
            painter.drawText(x+20,y+30,str.setNum(f));
            f++;
        }
    }


    const int kszx=70,kszy=70;

    const int row2x=10,row2y=90;
    x=row2x;
    y=row2y;


    painter.drawRect(x,y,kszx,kszy);
    painter.drawText(x+5,y+20,"~");
    painter.drawText(x+5,y+60,"`");

    for(int i=0;i<12;i++){
        x+=kszx;
        painter.drawRect(x,y,kszx,kszy);
        for(int j=0;j<kblayout.AE[i].klst.size();j++){
            if(j==1)
                painter.drawText(x+15,y+10,20,20,Qt::AlignBottom,symbol.getkeyuni(kblayout.AE[i].klst.at(1)));
            if(j==0)
                painter.drawText(x+15,y+40,20,20,Qt::AlignTop,symbol.getkeyuni(kblayout.AE[i].klst.at(0)));
            painter.setPen(QColor(lev34color));
            if(j==2)
                painter.drawText(x+40,y+40,20,20,Qt::AlignTop,symbol.getkeyuni(kblayout.AE[i].klst.at(2)));
            if(j==3)
                painter.drawText(x+40,y+25,20,20,Qt::AlignBottom,symbol.getkeyuni(kblayout.AE[i].klst.at(3)));
            painter.setPen(QColor(lev12color));
        }
    }
    x+=kszx;

    const int bkspszx=100;

    painter.drawRect(x,y,bkspszx,kszy);
    painter.drawText(x+10,y+20,"<--");
    painter.drawText(x+10,y+60,"Backspace");


    const int row3x=10,row3y=170;
    x=row3x;
    y=row3y;


    const int tabszx=100;

    painter.drawRect(x,y,tabszx,kszy);
    painter.drawText(x+20,y+25,"<--");
    painter.drawText(x+15,y+35,"-->");
    painter.drawText(x+10,y+60,"TAB");
    x+=tabszx;

    for(int i=0;i<12;i++){
        painter.drawRect(x,y,kszx,kszy);
        for(int j=0;j<kblayout.AD[i].klst.size();j++){
            if(j==1)
                painter.drawText(x+15,y+10,20,20,Qt::AlignBottom,symbol.getkeyuni(kblayout.AD[i].klst.at(1)));
            if(j==0)
                painter.drawText(x+15,y+40,20,20,Qt::AlignTop,symbol.getkeyuni(kblayout.AD[i].klst.at(0)));
            painter.setPen(QColor(lev34color));
            if(j==2)
                painter.drawText(x+40,y+40,20,20,Qt::AlignBottom,symbol.getkeyuni(kblayout.AD[i].klst.at(2)));
            if(j==3)
                painter.drawText(x+40,y+25,20,20,Qt::AlignTop,symbol.getkeyuni(kblayout.AD[i].klst.at(3)));
            painter.setPen(QColor(lev12color));
        }
        x+=kszx;
    }

    painter.drawRect(x,y,kszx,kszy);
    painter.drawText(x+20,y+20,"|");
    painter.drawText(x+20,y+60,"\\");

    const int row4x=10,row4y=250;

    x=row4x;
    y=row4y;

    const int capszx=100;

    painter.drawRect(x,y,capszx,kszy);
    painter.drawText(x+10,y+20,"^");
    painter.drawText(x+10,y+60,"Caps Lock");
    x+=capszx;

    for(int i=0;i<11;i++){
        painter.drawRect(x,y,kszx,kszy);
        for(int j=0;j<kblayout.AC[i].klst.size();j++){
            if(j==1)
                painter.drawText(x+15,y+10,20,20,Qt::AlignBottom,symbol.getkeyuni(kblayout.AC[i].klst.at(1)));
            if(j==0)
                painter.drawText(x+15,y+40,20,20,Qt::AlignTop,symbol.getkeyuni(kblayout.AC[i].klst.at(0)));
            painter.setPen(QColor(lev34color));
            if(j==2)
                painter.drawText(x+40,y+40,20,20,Qt::AlignBottom,symbol.getkeyuni(kblayout.AC[i].klst.at(2)));
            if(j==3)
                painter.drawText(x+40,y+25,20,20,Qt::AlignTop,symbol.getkeyuni(kblayout.AC[i].klst.at(3)));
            painter.setPen(QColor(lev12color));
        }
        x+=kszx;
    }

    const int retsz=140;

    painter.drawRect(x,y,retsz,kszy);
    painter.drawText(x+90,y+20,"|");
    painter.drawText(x+75,y+25,"<--");
    painter.drawText(x+10,y+60,"Enter");

    const int row5x=10,row5y=330;

    x=row5x;
    y=row5y;

    const int shiftsz=155;

    painter.drawRect(x,y,shiftsz,kszy);
    painter.drawText(x+10,y+60,"SHIFT");
    x+=shiftsz;

    for(int i=0;i<10;i++){
        painter.drawRect(x,y,kszx,kszy);
        for(int j=0;j<kblayout.AB[i].klst.size();j++){
            if(j==1)
                painter.drawText(x+15,y+10,20,20,Qt::AlignBottom,symbol.getkeyuni(kblayout.AB[i].klst.at(1)));
            if(j==0)
                painter.drawText(x+15,y+40,20,20,Qt::AlignTop,symbol.getkeyuni(kblayout.AB[i].klst.at(0)));
            painter.setPen(QColor(lev34color));
            if(j==2)
                painter.drawText(x+40,y+40,20,20,Qt::AlignBottom,symbol.getkeyuni(kblayout.AB[i].klst.at(2)));
            if(j==3)
                painter.drawText(x+40,y+25,20,20,Qt::AlignTop,symbol.getkeyuni(kblayout.AB[i].klst.at(3)));
            painter.setPen(QColor(lev12color));
        }
        x+=kszx;
    }
    painter.drawRect(x,y,shiftsz,kszy);
    painter.drawText(x+10,y+60,"SHIFT");

    const int row6x=110,row6y=410;
    const int ctrlsz=100,altsz=100,spsz=400;

    x=row6x;
    y=row6y;

    painter.drawRect(x,y,ctrlsz,kszy);
    painter.drawText(x+30,y+35,"Ctrl");
    x+=ctrlsz;
    painter.drawRect(x,y,altsz,kszy);
    painter.drawText(x+30,y+35,"Alt");
    x+=altsz;
    painter.drawRect(x,y,spsz,kszy);
    x+=spsz;
    painter.drawRect(x,y,ctrlsz,kszy);
    painter.drawText(x+30,y+35,"Ctrl");
    x+=ctrlsz;
    painter.drawRect(x,y,altsz,kszy);
    painter.setPen(QColor(lev34color));
    painter.drawText(x+30,y+35,"AltGr");
    painter.setPen(QColor(lev12color));

    if(symbol.nill>=120){
        painter.drawRect(0,0,1030,490);
        painter.drawText(470,240,"NO PREVIEW FOUND");
    }


}



void keyboardpainter::getkeyboardlayout(QString country, QString layoutvariant){

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

QString keyboardpainter::getvariant(QString variant,QString selectedLayout){
    if (variant==""){
        variant="basic";
        if(selectedLayout=="ma")
            variant="arabic";
        if(selectedLayout=="az")
            variant="latin";
        if(selectedLayout=="bg")
            variant="bds";
        if(selectedLayout=="fi")
            variant="kotoistus";
        if(selectedLayout=="ca")
            variant="fr";
        if(selectedLayout=="in")
            variant="deva";
        if(selectedLayout=="jp")
            variant="106";
        if(selectedLayout=="ir")
            variant="pes";
        if(selectedLayout=="kr")
            variant="kr106";
        if(selectedLayout=="ru")
            variant="winkeys";
        if(selectedLayout=="lk")
            variant="sin_phonetic";
        if(selectedLayout=="ke")
            variant="swa";
        if(selectedLayout=="tz")
            variant="swa";
        if(selectedLayout=="tw")
            variant="tw";
        if(selectedLayout=="bw")
            variant="tswana";
        if(selectedLayout=="ua")
            variant="unicode";
        if(selectedLayout=="pk")
            variant="urd-phonetic";
        if(selectedLayout=="uz")
            variant="cyrillic";
    }
    return variant;
}
