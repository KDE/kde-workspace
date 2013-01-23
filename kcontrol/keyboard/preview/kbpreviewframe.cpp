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

#include <QtCore/QFile>
#include <QtGui/QFont>
#include <QHelpEvent>
#include <QToolTip>

#include <KApplication>
#include <KLocale>


static const QColor keyBorderColor("#d4d4d4");
static const QColor lev12color("#d4d4d4");
static const QColor lev34color("#FF3300");
static const QColor color[] = { lev12color, lev12color, lev34color, lev34color };




KbPreviewFrame::KbPreviewFrame(QWidget *parent) :
    QFrame(parent)
{
     setFrameStyle( QFrame::Box );
     setFrameShadow(QFrame::Sunken);
     setMouseTracking(true);
}

int KbPreviewFrame::keyAt(QPoint pos){
    const int tooltip_range = 10;
    for(int i=0;i<toolTipList.size();i++){
        ToolTipText temp=toolTipList.at(i);
        if(temp.ttPoint.x()<=pos.x()&&temp.ttPoint.x()+tooltip_range>=pos.x()&&temp.ttPoint.y()<=pos.y()&&temp.ttPoint.y()+tooltip_range>=pos.y()){

            return i;
        }
    }
    return -1;
}

bool KbPreviewFrame::event(QEvent *event){
    if (event->type() == QEvent::ToolTip) {
         QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
         int index = keyAt(helpEvent->pos());
         if (index != -1) {
             QToolTip::showText(helpEvent->globalPos(), toolTipList[index].ttString);
         } else {
             QToolTip::hideText();
             event->ignore();
         }

         return true;
     }
     return QWidget::event(event);
}

void KbPreviewFrame::paintEvent(QPaintEvent *){
    //getkeyboardlayout("ua","Ukrainian");
    //setWindowTitle(kblayout.Layoutname);
    QPainter painter(this);
    QFont kbfont("Ubuntu",9);
    //alias.getAlias("qwerty","AD01");
    painter.setFont(kbfont);
    painter.setBrush(QBrush(Qt::black));
    //painter.setPen(Qt::black);
    //painter.drawText(450,10,kblayout.Layoutname);
    painter.drawRect(0,0,1030,490);
    painter.setPen(keyBorderColor);
    painter.setBrush(QBrush(Qt::transparent));
    setScaleFactor(geometry.kbWidth,geometry.kbHieght);
    for(int i=0;i<geometry.keys.size();i++){
        KeyGm dummy = geometry.keys.at(i);
        painter.setPen(keyBorderColor);
        painter.drawRect(scalex*dummy.cordx,scaley*dummy.cordy,scalex*dummy.shape.sizex,scaley*dummy.shape.sizey);
        writeKeySym(painter,dummy);
    }
}

void KbPreviewFrame :: plotSym(QPainter &painter, KeyGm key, QString sym){
    const int x= key.cordx,y= key.cordy;
    int symx = key.shape.sizex/3-sym.length(),symy=key.shape.sizey/2+3;
    if(key.shape.sizex==0){
        symx=6-sym.length();
    }
    if(key.shape.sizey==0)
        symy=12;
    painter.setPen(keyBorderColor);
    QFont kbfont;
    kbfont.setPointSize(7);
    painter.setFont(kbfont);
    painter.drawText(scalex*(x+symx),scaley*(y+symy),sym);
    kbfont.setPointSize(9);
    painter.setFont(kbfont);
}

void KbPreviewFrame :: plotSym(QPainter &painter,KeyGm key,Keys sym){
    const int x=key.cordx,y=key.cordy;
    const int lvl1x = key.shape.sizex/5+1,lvl1y=key.shape.sizey/2+5;
    const int lvl2x =key.shape.sizex/5+1,lvl2y=key.shape.sizey/5+5;
    const int lvl3x=key.shape.sizex/2+1,lvl3y=key.shape.sizey/2+5;
    const int lvl4x=key.shape.sizex/2+1,lvl4y=key.shape.sizey/5+5;
    const int levelx_Pos[] = { lvl1x , lvl2x , lvl3x , lvl4x };
    const int levely_Pos[] = { lvl1y , lvl2y , lvl3y ,lvl4y };

    QPoint tp;
    tp.setX(scalex*x);
    tp.setY(scaley*y);
    QString ts=sym.keyVal;
    ToolTipText temp;
    temp.ttPoint=tp;
    temp.ttString=ts;
    toolTipList<<temp;
    for(int level = 0; level < sym.klst.size(); level++){
        painter.setPen(color[level]);
        painter.drawText(scalex*(x+levelx_Pos[level]),scaley*(y+levely_Pos[level]),symbol.getKeySymbol(sym.klst.at(level)));
    }
}

void KbPreviewFrame :: writeKeySym(QPainter &painter, KeyGm key){

    int plotted=0;

    const QString key_group[] = { "AE" , "AD" , "AC" , "AB" , "TLDE" , "BKSL" };

    for(int group_no = 0 ; group_no < 6 ; group_no++){
        if(key.name.startsWith(key_group[group_no])){
            QString ind=key.name.right(2);
            int index=ind.toInt();
            Keys sym;
            if (group_no == 0)
                sym=kblayout.AE[index-1];
            if (group_no == 1)
                sym=kblayout.AD[index-1];
            if (group_no == 2)
                sym=kblayout.AC[index-1];
            if (group_no == 3)
                sym=kblayout.AB[index-1];
            if (group_no == 4)
                sym=kblayout.TLDE;
            if (group_no == 5)
                sym=kblayout.BKSL;
            plotSym(painter,key,sym);
            plotted=1;

        }
    }
    if(plotted==0){
        QString sym=key.name;
        plotSym(painter,key,sym);
    }
}


void KbPreviewFrame :: setScaleFactor(int width,int height){
    scalex=1030/width;
    scaley=490/height;
    scalex=2;
    scaley=2;
}


void KbPreviewFrame::generateKeyboardLayout(const QString& country, const QString& layoutvariant,const QString& model)
{
    QString filename=kblayout.findSymbolbasedir();
    filename.append(country);

    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = file.readAll();
    file.close();

    QList<QString> symstr;
    symstr=content.split("xkb_symbols ");

    geometry.extractKeys(model);

    if(layoutvariant=="")
        kblayout.generateLayout(symstr.at(1),country);

    else
    {
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
                kblayout.generateLayout(symstr.at(i),country);
                break;
            }
        }
    }
}

