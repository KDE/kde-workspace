#include "geometry_components.h"

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/QPoint>

GShape::GShape(){
    cordi_count = 0;
}

void GShape::setCordinate(double a, double b){
    cordii << QPoint(a,b);
    cordi_count++;
}

void GShape::setApprox(double a, double b){
    a-=approx.x();
    b-=approx.y();
    approx = QPoint(a,b);
}


QPoint GShape :: getCordii(int i){
    if(i < cordi_count)
        return cordii[i];
    else
        return QPoint();
}


void GShape::display(){
    qDebug()<<"shape: "<<sname<<"\n";
    qDebug()<<"( "<<approx.x()<<", "<<approx.y()<<"); ";
    for(int i=0;i<cordi_count;i++)
        qDebug()<<cordii[i];
    qDebug()<<"\n";
}

double GShape::size(int vertical){
    if(vertical == 0){
        if (approx.x() == 0 && approx.y() == 0)
            return cordii[0].x();
        else
            return approx.x();
    }
    else
        if (approx.x() == 0 && approx.y() == 0)
            return cordii[0].y();
        else
            return approx.y();

}

Key::Key(){
    offset = 0;
}

void Key::setKeyPosition(double x, double y){
    position = QPoint(x,y);
}

void Key::showKey(){
    qDebug()<<"\n\tKey: "<<name<<"\tshape: "<<shapeName<<"\toffset: "<<offset;
    qDebug()<<"\tposition"<<position<<"\n";
}


Row::Row(){
    top = 0;
    left = 0;
    keyCount = 0;
    vertical = 0;
    keyList << Key();
}


void Row::addKey(){
    qDebug() << "keyCount: "<<keyCount;
    keyCount++;
    keyList << Key();
}



void Row::displayRow(){
    qDebug() << "\nRow: ("<<left<<","<<top<<")\n";
    qDebug() << "vertical: "<<vertical;
    for(int i=0;i<keyCount;i++)
    keyList[i].showKey();
}


Section::Section(){
    top = 0;
    left = 0;
    angle = 0;
    rowCount = 0;
    vertical = 0;
    rowList << Row();
}

void Section::addRow(){
    qDebug()<<"\nrowCount: "<<rowCount;
    rowCount++;
    rowList << Row();
}

void Section::displaySection(){
    qDebug()<<"\nSection: "<<name<<"\n\tposition: ("<<left<<","<<top<<");"<<angle<<"\n";
    qDebug()<<"vertical: "<<vertical;
    for(int i=0;i<rowCount;i++){
            qDebug()<<"\n\t";
            rowList[i].displayRow();
    }
}

Geometry::Geometry(){
    sectionTop = 0;
    sectionLeft = 0;
    rowTop = 0;
    rowLeft = 0;
    keyGap = 0;
    shape_count=0;
    width=0;
    height=0;
    sectionCount = 0;
    vertical = 0;
    sectionList << Section();
    shapes << GShape();
    keyShape = QString("NORM");
}


void Geometry::setShapeName(QString n){
    shapes[shape_count].setShapeName(n);
}

void Geometry::setShapeCord(double a, double b){
    shapes[shape_count].setCordinate(a,b);
}

void Geometry::setShapeApprox(double a, double b){
    shapes[shape_count].setApprox(a,b);
}

void Geometry::addShape(){
    shape_count++;
    shapes << GShape();
}

void Geometry::display(){
    qDebug()<<name<<"\n"<<description<<"\nwidth: "<<width<<"\nheight: "<<height<<"\n"<<"sectionTop: "<<sectionTop;
    qDebug()<<"\nsectionLeft: "<<sectionLeft<<"\nrowTop: "<<rowTop<<"\nrowLeft: "<<rowLeft<<"\nkeyGap: "<<keyGap<<"\nkeyShape: "<<keyShape<<"\n";
    qDebug()<<"vertical: "<<vertical;
    for (int i=0;i<shape_count;i++){
            shapes[i].display();
    }
    for (int j=0;j<sectionCount;j++)
            sectionList[j].displaySection();
}

void Geometry::addSection(){
    qDebug()<<"\nsectionCount: "<<sectionCount;
    sectionCount++;
    sectionList << Section();
}

GShape Geometry::findShape(QString name){
    GShape l;
    for(int i=0;i<shape_count;i++){
        if (shapes[i].getShapeName() == name){
                    return shapes[i];
            }
    }
    return l;
}







