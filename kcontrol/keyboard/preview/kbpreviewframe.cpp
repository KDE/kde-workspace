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

#ifdef NEW_GEOMETRY
#include "geometry_parser.h"
#endif

#include <QtCore/QFile>
#include <QtGui/QFont>
#include <QFileDialog>
#include <math.h>

#include <KApplication>
#include <KLocale>




KbPreviewFrame::KbPreviewFrame(QWidget *parent) :
    QFrame(parent)
{
     setFrameStyle( QFrame::Box );
     setFrameShadow(QFrame::Sunken);
}


#ifdef NEW_GEOMETRY

static const QColor keyBorderColor("#d4d4d4");
static const QColor lev12color(Qt::black);
static const QColor lev34color("#FF3300");
static const int xOffset[] = {10, 10, -15, -15 };
static const int yOffset[] = {5, -20, 5, -20 };
static const QColor color[] = { lev12color, lev12color, lev34color, lev34color };


void KbPreviewFrame::drawKeySymbols(QPainter &painter,QPoint temp[],GShape s,QString name)
{
    QList<QString> symbols;
    int symbolset = 0;
    if (name == "TLDE"){
        symbols = keyboardLayout.TLDE.symbols;
        symbolset = 1;
    }
    if (name == "BKSL"){
        symbols = keyboardLayout.BKSL.symbols;
        symbolset = 1;
    }
    if (name.startsWith("AE")){
        for(int i = 0 ; i < 12 ; i++){
            if (name == keyboardLayout.AE[i].keyname){
                symbols = keyboardLayout.AE[i].symbols;
                symbolset = 1;
                break;
            }
        }
    }
    if (name.startsWith("AD")){
        for(int i = 0 ; i < 12 ; i++){
            if (name == keyboardLayout.AD[i].keyname){
                symbols = keyboardLayout.AD[i].symbols;
                symbolset = 1;
                break;
            }
        }
    }
    if (name.startsWith("AC")){
        for(int i = 0 ; i < 11 ; i++){
            if (name == keyboardLayout.AC[i].keyname){
                symbols = keyboardLayout.AC[i].symbols;
                symbolset = 1;
                break;
            }
        }
    }
    if (name.startsWith("AB")){
        for(int i = 0 ; i < 11 ; i++){
            if (name == keyboardLayout.AB[i].keyname){
                symbols = keyboardLayout.AB[i].symbols;
                symbolset = 1;
                break;
            }
        }
    }

    int sz = 20;
    int cordinate[] = {0, 3, 1, 2};
    if(symbolset == 1){
        for(int level=0; level<symbols.size(); level++) {
            painter.setPen(color[level]);
            painter.drawText(temp[cordinate[level]].x()+xOffset[level], temp[cordinate[level]].y()+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
        }
    }
    else{
        painter.setPen(Qt::black);
        painter.drawText(temp[0].x()+s.size(0)-10,temp[0].y()+3*s.size(1)/2,name);
    }
}


void KbPreviewFrame::drawShape(QPainter &painter,GShape s,int x,int y,int i,QString name){
    painter.setPen(Qt::black);
    int cordi_count = s.getCordi_count();
    if(geometry.sectionList[i].getAngle()==0){
        if (cordi_count == 1){
            int width = s.getCordii(0).x();
            int height = s.getCordii(0).y();
            painter.drawRoundedRect(scaleFactor*x+2,scaleFactor*y,scaleFactor*width,scaleFactor*height,4,4);
            QPoint temp[4];
            temp[0]=QPoint(scaleFactor*x,scaleFactor*y);
            temp[1]=QPoint(scaleFactor*(s.getCordii(0).x()+x),scaleFactor*y);
            temp[2]=QPoint(scaleFactor*(s.getCordii(0).x()+x),scaleFactor*(s.getCordii(0).y()+y));
            temp[3]=QPoint(scaleFactor*(x),scaleFactor*(s.getCordii(0).y()+y));
            drawKeySymbols(painter,temp,s,name);
        }
        else{
            QPoint temp[cordi_count];
            for(int i=0;i<cordi_count;i++){
                temp[i].setX(scaleFactor*(s.getCordii(i).x()+x+1));
                temp[i].setY(scaleFactor*(s.getCordii(i).y()+y+1));
            }
            painter.drawPolygon(temp,cordi_count);
            drawKeySymbols(painter,temp,s,name);
        }
    }
    else{
        QPoint temp[cordi_count == 1 ? 4 : cordi_count];
        int size;
        if(cordi_count== 1){
            temp[0]=QPoint(x,y);
            temp[1]=QPoint(s.getCordii(0).x()+x,y);
            temp[2]=QPoint(s.getCordii(0).x()+x,s.getCordii(0).y()+y);
            temp[3]=QPoint(x,s.getCordii(0).y()+y);
            size = 4;
        }
        else{
            size = cordi_count;
            for(int i=0;i<cordi_count;i++){
                temp[i].setX((s.getCordii(i).x()+x+1));
                temp[i].setY((s.getCordii(i).y()+y+1));
            }
        }
        double refX,refY;
        refX = geometry.sectionList[i].getLeft();
        refY = geometry.sectionList[i].getTop();
        //qDebug()<<"\ntransform";
        for(int j=0;j<size;j++){
            double x = temp[j].x()-refX;
            double y = temp[j].y()-refY;
            //qDebug()<<"("<<x<<","<<y<<")->";
            float theta = ( 3.1459 * geometry.sectionList[i].getAngle() )/180;
            double x_ = x*cos(theta)-y*sin(theta);
            //qDebug()<<"x_= "<<x<<"*"<<cos(theta)<<"-"<<y<<"*"<<sin(theta);
            double y_ = x*sin(theta)+y*cos(theta);
            //qDebug()<<"\ny_= "<<x<<"*"<<sin(theta)<<"+"<<y<<"*"<<cos(theta);
            //qDebug()<<"("<<x_<<","<<y_<<")\n";
            temp[j]=QPoint(scaleFactor*(x_+refX),scaleFactor*(y_+refY));
        }
        /*for(int i=0;i<size;i++){
            qDebug()<<temp[i];
        }*/
        painter.drawPolygon(temp,size);
        drawKeySymbols(painter,temp,s,name);
    }

}


void KbPreviewFrame::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QFont kbfont;
    kbfont.setPointSize(9);

    painter.setFont(kbfont);
    painter.setBrush(QBrush("#C3C8CB"));
    painter.setRenderHint(QPainter::Antialiasing);

    const int strtx=0,strty=0,endx=geometry.getWidth(),endy=geometry.getHeight(),kszy=70;
    //const int row1x=10,row1y=30,row2x=10,row2y=90,row5x=10,row5y=330,row3x=10,row3y=170,shifx=10,shify=60,row4x=10,row4y=250,row6x=110,row6y=410;
    //const int shiftsz=155;

    painter.setPen("#EDEEF2");
    scaleFactor = 1030/endx;
    if(scaleFactor<1)
        scaleFactor=1;
    qDebug()<<"scaleFactor = "<<scaleFactor;
    scaleFactor = 2.5;
    painter.drawRect(strtx, strty, scaleFactor*endx+60,scaleFactor*endy+60);

    painter.setPen(Qt::black);
    painter.setBrush(QBrush("#EDEEF2"));
    for(int i=0;i<geometry.getSectionCount();i++){
        painter.setPen(Qt::black);
        for(int j=0;j<geometry.sectionList[i].getRowCount();j++){
            int keyn = geometry.sectionList[i].rowList[j].getKeyCount();
            for(int k=0;k<keyn;k++){
                Key temp = geometry.sectionList[i].rowList[j].keyList[k];
                int x = temp.getPosition().x();
                int y = temp.getPosition().y();
                GShape s;
                s = geometry.findShape(temp.getShapeName());
                QString name = temp.getName();
                //painter.drawRoundedRect(2*x+2,2*y,2*width,2*height,4,4);
                drawShape(painter,s,x,y,i,name);
                //painter.drawText(scaleFactor*x+width/3,scaleFactor*y+3*height/2,name);
            }
        }
    }

    if( symbol.isFailed() ) {
        painter.setPen(keyBorderColor);
        painter.drawRect(strtx, strty, endx, endy);

        const int midx=470, midy=240;
        painter.setPen(lev12color);
        painter.drawText(midx, midy, i18n("No preview found"));
    }

}


void KbPreviewFrame::generateKeyboardLayout(const QString& layout, const QString& layoutVariant,QString model)
{
    QString filename = keyboardLayout.findSymbolBaseDir();
    filename.append(layout);

    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = file.readAll();
    file.close();

    geometry = grammar::parseGeometry(model);

    QList<QString> symstr = content.split("xkb_symbols ");

    if( layoutVariant.isEmpty() ) {
        keyboardLayout.generateLayout(symstr.at(1), layout);
    }
    else {
        for(int i=1;i<symstr.size();i++) {
            QString h=symstr.at(i);
            int k=h.indexOf("\"");
            h=h.mid(k);
            k=h.indexOf("{");
            h=h.left(k);
            h=h.remove(" ");
            QString f="\"";
            f.append(layoutVariant);
            f.append("\"");
            f=f.remove(" ");

            if(h==f){
                keyboardLayout.generateLayout(symstr.at(i), layout);
                break;
            }
        }
    }


}
#else

static const int keygap = 4, cornerRadius = 7;
static const QColor keyBorderColor("#d4d4d4");
static const QColor lev12color("#d4d4d4");
static const QColor lev34color("#FF3300");
static const int sz=20, kszx=70, kszy=70;

static const int xOffset[] = {15, 15, 40, 40 };
static const int yOffset[] = {10, 40, 10, 40 };
static const QColor color[] = { lev12color, lev12color, lev34color, lev34color };

void KbPreviewFrame::paintTLDE(QPainter &painter,int &x,int &y)
{
    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y, kszx, kszy,cornerRadius, cornerRadius);

    const QList <QString> symbols = keyboardLayout.TLDE.symbols;

    for(int level=0; level<symbols.size(); level++) {
        painter.setPen(color[level]);
        painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
    }

}

void KbPreviewFrame::paintAERow(QPainter &painter,int &x,int &y)
{
    paintTLDE(painter, x, y);

    const int noAEk=12;
    for(int i=0; i<noAEk; i++){
        x+=kszx;
        x+=keygap;

        painter.setPen(keyBorderColor);
        painter.drawRoundedRect(x, y, kszx, kszy,cornerRadius, cornerRadius);

        QList<QString> symbols = keyboardLayout.AE[i].symbols;

        for(int level=0; level<symbols.size(); level++) {
            painter.setPen(color[level]);
            painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
        }
    }

    x += kszx;
    x += keygap;
    const int bkspszx=100,bk1x=10;//,bk1y=20,
    const int bk2y=60;

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y,bkspszx,kszy,cornerRadius, cornerRadius);

    painter.setPen(lev12color);
//    painter.drawText(x+bk1x, y+bk1y,i18n("<--"));
    painter.drawText(x+bk1x, y+bk2y,i18n("Backspace"));
}

void KbPreviewFrame::paintADRow(QPainter &painter,int &x,int&y)
{
    const int noADk=12;
    const int tabszx=100;
    const int tab3y=45;

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y, tabszx,kszy,cornerRadius, cornerRadius);

    painter.setPen(lev12color);
//    painter.drawText(x+tab1x, y+tab1y,i18n("<--"));
    painter.drawText(x+xOffset[0], y+tab3y, i18nc("Tab key", "Tab"));
//    painter.drawText(x+tab2x, y+tab2y,i18n("-->"));
    x+=tabszx;
    x+=keygap;


    for(int i=0; i<noADk; i++){
        QList<QString> symbols = keyboardLayout.AD[i].symbols;

        painter.setPen(keyBorderColor);
        painter.drawRoundedRect(x, y,kszx,kszy,cornerRadius, cornerRadius);

        for(int level=0; level<symbols.size(); level++) {
            painter.setPen(color[level]);
            painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
        }

        x+=kszx;
        x+=keygap;
    }

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y,kszx,kszy,cornerRadius, cornerRadius);

    QList<QString> symbols = keyboardLayout.BKSL.symbols;

    for(int level=0; level<symbols.size(); level++) {
        painter.setPen(color[level]);
        painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
    }
}

void KbPreviewFrame::paintACRow(QPainter &painter,int &x,int &y)
{
    const int sz = 20, kszx = 70, kszy = 70, capszx = 100;
    const int noACk = 11;
    const int lvl2x = 40, shifx = 10, shify = 60, retsz = 140;

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y,capszx,kszy,cornerRadius, cornerRadius);

    painter.setPen(lev12color);
//    painter.drawText(x+shifx, y+sz,i18n("^"));
    painter.drawText(x+shifx, y+shify,i18n("Caps Lock"));
    x+=capszx;
    x+=keygap;

    for(int i=0; i<noACk; i++){
        painter.setPen(keyBorderColor);
        painter.drawRoundedRect(x, y,kszx,kszy, cornerRadius, cornerRadius);

        QList<QString> symbols = keyboardLayout.AC[i].symbols;

        for(int level=0; level<symbols.size(); level++) {
            painter.setPen(color[level]);
            painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
        }

        x+=kszx;
        x+=keygap;
    }

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y,retsz,kszy,cornerRadius, cornerRadius);

    painter.setPen(lev12color);
//    painter.drawText(x+ret1x, y+ret1y,i18n("|"));
//    painter.drawText(x+ret2x, y+ret2y,i18n("<--"));
    painter.drawText(x+shify,y+lvl2x,i18n("Enter"));
}

void KbPreviewFrame::paintABRow(QPainter &painter,int &x,int &y)
{
    const int noABk=10;
    for(int i=0; i<noABk; i++) {
        painter.setPen(keyBorderColor);
        painter.drawRoundedRect(x, y,kszx,kszy,cornerRadius, cornerRadius);

        QList<QString> symbols = keyboardLayout.AB[i].symbols;

        for(int level=0; level<symbols.size(); level++) {
            painter.setPen(color[level]);
            painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
        }

        x+=kszx;
        x+=keygap;
    }
}

void KbPreviewFrame::paintBottomRow(QPainter &painter,int &x,int &y)
{
    const int txtx=30, txty=35, ctrlsz=100, altsz=100, spsz=400, kszy=70;

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y, ctrlsz, kszy, cornerRadius, cornerRadius);
    painter.setPen(lev12color);
    painter.drawText(x+txtx, y+txty,i18n("Ctrl"));

    x+=ctrlsz;
    x+=keygap;

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y, altsz, kszy, cornerRadius, cornerRadius);
    painter.setPen(lev12color);
    painter.drawText(x+txtx, y+txty,i18n("Alt"));

    x+=altsz;
    x+=keygap;

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y, spsz, kszy, cornerRadius, cornerRadius);

    x+=spsz;
    x+=keygap;

    painter.drawRoundedRect(x, y, altsz, kszy,cornerRadius, cornerRadius);

    painter.setPen(lev34color);
    painter.drawText(x+txtx, y+txty,i18n("AltGr"));

    x+=ctrlsz;
    x+=keygap;

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y, ctrlsz, kszy, cornerRadius, cornerRadius);

    painter.setPen(lev12color);
    painter.drawText(x+txtx, y+txty, i18n("Ctrl"));
}

void KbPreviewFrame::paintFnKeys(QPainter &painter,int &x,int &y)
{
    const int escsz=50, escx=20, escy=55;

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y, escsz, escsz, cornerRadius, cornerRadius);

    painter.setPen(lev12color);
    painter.drawText(escx, escy, i18n("Esc"));

    const int spacex=50;
    x+=spacex;
    x+=keygap;

    const int fnkeyspace=60, fnkeysizex=50, fnkeysizey=50, fkc=15, fky=30, fnkig=4, fng=3;
    int f=1;

    for(int i=0;i<fng;i++){
        x+=spacex;
        x+=keygap;

        for(int j=0;j<fnkig;j++){
            x += fnkeyspace;
            painter.setPen(keyBorderColor);
            painter.drawRoundedRect(x, y, fnkeysizex, fnkeysizey, cornerRadius, cornerRadius);

            painter.setPen(lev12color);
            painter.drawText(x+fkc, y+fky, i18nc("Function key", "F%1", f));
            f++;
        }
    }
}

void KbPreviewFrame::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QFont kbfont;
    kbfont.setPointSize(12);

    painter.setFont(kbfont);
    painter.setBrush(QBrush(Qt::darkGray));

    const int strtx=0,strty=0,endx=1390,endy=490,kszy=70;
    const int row1x=10,row1y=30,row2x=10,row2y=90,row5x=10,row5y=330,row3x=10,row3y=170,shifx=10,shify=60,row4x=10,row4y=250,row6x=110,row6y=410;
    const int shiftsz=155;

    painter.setPen(keyBorderColor);
    painter.drawRect(strtx, strty, endx, endy);

    painter.setPen(lev12color);
    painter.setBrush(QBrush(Qt::black));

    int x, y;
    x=row1x;
    y=row1y;

    paintFnKeys(painter,x, y);

    x=row2x;
    y=row2y;

    paintAERow(painter,x, y);

    x=row3x;
    y=row3y;

    paintADRow(painter,x, y);

    x=row4x;
    y=row4y;

    paintACRow(painter,x, y);

    x=row5x;
    y=row5y;

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y,shiftsz,kszy, cornerRadius, cornerRadius);
    painter.setPen(lev12color);
    painter.drawText(x+shifx, y+shify,i18n("Shift"));
    x+=shiftsz;
    x+=keygap;

    paintABRow(painter,x, y);

    painter.setPen(keyBorderColor);
    painter.drawRoundedRect(x, y,shiftsz,kszy, cornerRadius, cornerRadius);
    painter.setPen(lev12color);
    painter.drawText(x+shifx, y+shify,i18n("Shift"));

    x=row6x;
    y=row6y;

    paintBottomRow(painter,x, y);

    if( symbol.isFailed() ) {
        painter.setPen(keyBorderColor);
        painter.drawRect(strtx, strty, endx, endy);

        const int midx=470, midy=240;
        painter.setPen(lev12color);
        painter.drawText(midx, midy, i18n("No preview found"));
    }

}


void KbPreviewFrame::generateKeyboardLayout(const QString& layout, const QString& layoutVariant)
{
    QString filename = keyboardLayout.findSymbolBaseDir();
    filename.append(layout);

    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = file.readAll();
    file.close();

    QList<QString> symstr = content.split("xkb_symbols ");

    if( layoutVariant.isEmpty() ) {
        keyboardLayout.generateLayout(symstr.at(1), layout);
    }
    else {
        for(int i=1;i<symstr.size();i++) {
            QString h=symstr.at(i);
            int k=h.indexOf("\"");
            h=h.mid(k);
            k=h.indexOf("{");
            h=h.left(k);
            h=h.remove(" ");
            QString f="\"";
            f.append(layoutVariant);
            f.append("\"");
            f=f.remove(" ");

            if(h==f){
                keyboardLayout.generateLayout(symstr.at(i), layout);
                break;
            }
        }
    }
}

#endif
