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

    //getkeyboardlayout("am","Armenian (eastern)");
    setWindowTitle(kblayout.Layoutname);
    QPainter painter(this);
    painter.setBrush(QBrush(Qt::darkGray));
    //painter.setPen(Qt::black);
    //painter.drawText(450,10,kblayout.Layoutname);
    painter.drawRect(0,20,1030,490);
    painter.setPen(QColor("#d4d4d4"));
    painter.setBrush(QBrush(Qt::black));

    int x=10,y=30;
    painter.drawRect(x,y,50,50);
    QChar a=0xa4;
    QString b=QString(a);
    painter.drawText(20,55,b);
    int spacex=50;
    x+=50;
    int f=1;
    QString str;
    for(int i=0;i<3;i++){
        x+=spacex;
        for(int j=0;j<4;j++){
            x+=60;
            painter.drawRect(x,y,50,50);
            painter.drawText(x+15,y+30,"F");
            painter.drawText(x+20,y+30,str.setNum(f));
            f++;
        }
    }

    x=10;
    y=90;

    painter.drawRect(x,y,70,70);
    painter.drawText(x+5,y+20,"~");
    painter.drawText(x+5,y+60,"`");
    f=1;
    for(int i=0;i<12;i++){
        x+=70;
        painter.drawRect(x,y,70,70);
        for(int j=0;j<kblayout.AE[i].klst.size();j++){
            /*QMessageBox fg;
            fg.setText("H!");
            fg.exec();*/
            if(j==1){
                /*QMessageBox fg;*/

                QString p;
                p=kblayout.AE[i].klst.at(1);
                painter.drawText(x+5,y+20,symbol.getkeyuni(p));
            }
            if(j==0){
                //fg.setText("H2");

                painter.drawText(x+5,y+60,symbol.getkeyuni(kblayout.AE[i].klst.at(0)));

            }
            if(j==2)
                painter.drawText(x+60,y+60,symbol.getkeyuni(kblayout.AE[i].klst.at(2)));
            if(j==3)
                painter.drawText(x+60,y+20,symbol.getkeyuni(kblayout.AE[i].klst.at(3)));
        }
        f++;
    }
    x+=70;
    painter.drawRect(x,y,100,70);
    painter.drawText(x+10,y+20,"<--");
    painter.drawText(x+10,y+60,"Backspace");

    x=10;
    y=170;

    painter.drawRect(x,y,100,70);
    painter.drawText(x+10,y+20,"<--");
    painter.drawText(x+10,y+25,"-->");
    painter.drawText(x+10,y+60,"TAB");
    x+=100;

    for(int i=0;i<12;i++){
        painter.drawRect(x,y,70,70);
        for(int j=0;j<kblayout.AD[i].klst.size();j++){
            /*QMessageBox fg;
            fg.setText("H3");
            fg.exec();*/
            if(j==1)
                painter.drawText(x+5,y+20,symbol.getkeyuni(kblayout.AD[i].klst.at(1)));
            if(j==0)
                painter.drawText(x+5,y+60,symbol.getkeyuni(kblayout.AD[i].klst.at(0)));
            if(j==2)
                painter.drawText(x+60,y+60,symbol.getkeyuni(kblayout.AD[i].klst.at(2)));
            if(j==3)
                painter.drawText(x+60,y+20,symbol.getkeyuni(kblayout.AD[i].klst.at(3)));
        }
        x+=70;
    }
    x+=100;
    painter.drawRect(x,y,400,70);

    painter.drawRect(x,y,70,70);
    painter.drawText(x+10,y+20,"|");
    painter.drawText(x+10,y+60,"\\");

    x=10;
    y=250;
    painter.drawRect(x,y,100,70);
    painter.drawText(x+10,y+20,"^");
    painter.drawText(x+10,y+60,"Caps Lock");
    x+=100;

    for(int i=0;i<11;i++){
        painter.drawRect(x,y,70,70);
        for(int j=0;j<kblayout.AC[i].klst.size();j++){
            /*QMessageBox fg;
            fg.setText("H4");
            fg.exec();*/
            if(j==1)
                painter.drawText(x+5,y+20,symbol.getkeyuni(kblayout.AC[i].klst.at(1)));
            if(j==0)
                painter.drawText(x+5,y+60,symbol.getkeyuni(kblayout.AC[i].klst.at(0)));
            if(j==2)
                painter.drawText(x+60,y+60,symbol.getkeyuni(kblayout.AC[i].klst.at(2)));
            if(j==3)
                painter.drawText(x+60,y+20,symbol.getkeyuni(kblayout.AC[i].klst.at(3)));
        }
        x+=70;
    }

    painter.drawRect(x,y,140,70);
    painter.drawText(x+90,y+20,"|");
    painter.drawText(x+75,y+25,"<--");
    painter.drawText(x+10,y+60,"Enter");

    x=10;
    y=330;

    painter.drawRect(x,y,155,70);
    painter.drawText(x+10,y+60,"SHIFT");
    x+=155;

    for(int i=0;i<10;i++){
        painter.drawRect(x,y,70,70);
        for(int j=0;j<kblayout.AB[i].klst.size();j++){
            if(j==1)
                painter.drawText(x+5,y+20,symbol.getkeyuni(kblayout.AB[i].klst.at(1)));
            if(j==0)
                painter.drawText(x+5,y+60,symbol.getkeyuni(kblayout.AB[i].klst.at(0)));
            if(j==2)
                painter.drawText(x+60,y+60,symbol.getkeyuni(kblayout.AB[i].klst.at(2)));
            if(j==3)
                painter.drawText(x+60,y+20,symbol.getkeyuni(kblayout.AB[i].klst.at(3)));
        }
        x+=70;
    }
    painter.drawRect(x,y,155,70);
    painter.drawText(x+10,y+60,"SHIFT");

    x=110;
    y=410;

    painter.drawRect(x,y,100,70);
    painter.drawText(x+30,y+35,"Ctrl");
    x+=100;
    painter.drawRect(x,y,100,70);
    painter.drawText(x+30,y+35,"Alt");
    x+=100;
    painter.drawRect(x,y,400,70);
    x+=400;
    painter.drawRect(x,y,100,70);
    painter.drawText(x+30,y+35,"Ctrl");
    x+=100;
    painter.drawRect(x,y,100,70);
    painter.drawText(x+30,y+35,"Alt");

    if(symbol.nill>=120){
        painter.drawRect(0,0,1030,490);
        painter.drawText(470,240,"NO PREVIEW FOUND");
    }


}
void keyboardpainter::getkeyboardlayout(QString country, QString layoutvariant){
    QString filename="/usr/share/X11/xkb/symbols/";
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
            kblayout.getLayout(symstr.at(i));
            /*QString e;
            e=kblayout.Layoutname;
            for(int l=0;l<12;l++){
                e.append(kblayout.AE[l].keyname);
                for(int u=0;u<kblayout.AE[l].klst.size();u++){
                    e.append(kblayout.AE[l].klst.at(u));
                }
                e.append("\n");
            }
            for(int l=0;l<12;l++){
                e.append(kblayout.AD[l].keyname);
                for(int u=0;u<kblayout.AD[l].klst.size();u++){
                    e.append(kblayout.AD[l].klst.at(u));
                }
                e.append("\n");
            }
            for(int l=0;l<11;l++){
                e.append(kblayout.AC[l].keyname);
                for(int u=0;u<kblayout.AC[l].klst.size();u++){
                    e.append(kblayout.AC[l].klst.at(u));
                }
                e.append("\n");
            }
            for(int l=0;l<10;l++){
                e.append(kblayout.AB[l].keyname);
                for(int u=0;u<kblayout.AB[l].klst.size();u++){
                    e.append(kblayout.AB[l].klst.at(u));
                }
                e.append("\n");
            }
            QMessageBox q;
            q.setText(e);
            q.exec();*/
            break;
        }
    }
}
