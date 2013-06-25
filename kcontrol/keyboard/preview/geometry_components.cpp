#include "geometry_components.h"

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/QPoint>

Shape::Shape(){
    cordi_count = 0;
}

void Shape::setShape(QString n){
    sname = n;
}

void Shape::setCordinate(double &a, double &b){
    cordii[cordi_count++] = QPoint(a,b);
}

void Shape::setApprox(double &a, double &b){
    a-=approx.x();
    b-=approx.y();
    approx = QPoint(a,b);
}

void Shape::display(){
    qDebug()<<"shape: "<<sname<<"\n";
    qDebug()<<"( "<<approx.x()<<", "<<approx.y()<<"); ";
    for(int i=0;i<cordi_count;i++)
            qDebug()<<"( "<<cordii[i].x()<<", "<<cordii[i].y()<<"); ";
    qDebug()<<"\n";
}

double Shape::size(){
    if (approx.x() == 0 && approx.y() == 0)
        return cordii[0].x();
    else
        return approx.x();
}

Key::Key(){
    offset = 0;
}
void Key::getKey(double &o){
    offset = o;
}

void Key::setKeyPosition(double &x, double &y){
    position = QPoint(x,y);
}

void Key::showKey(){
    qDebug()<<"\n\tKey: "<<name<<"\tshape: "<<shapeName<<"\toffset: "<<offset;
    qDebug()<<"\tposition"<<position.x()<<" , "<<position.y()<<"\n";
}

Row::Row(){
    top = 0;
    left = 0;
    keyCount = 0;
}

void Row::getRow(double t,double l){
    top = t;
    left = l;
}

void Row::addKey(){
    qDebug()<<"keyCount: "<<keyCount;
    keyCount++;
}

void Row::displayRow(){
    qDebug()<<"\nRow: "<<top<<","<<left<<"\n";
    for(int i=0;i<keyCount;i++)
    keyList[i].showKey();
}

Section::Section(){
    top = 0;
    left = 0;
    angle = 0;
    rowCount = 0;
}

void Section::getName(QString n){
    name = n;
}

void Section::addRow(){
    qDebug()<<"\nrowCount: "<<rowCount;
    rowCount++;
}

void Section::displaySection(){
    qDebug()<<"\nSection: "<<name<<"\n\tposition: ("<<left<<","<<top<<");"<<angle<<"\n";
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
    shape_count=-1;
    width=0;
    height=0;
    sectionCount = 0;
}

void Geometry::getName(QString n){
    name = n;
}

void Geometry::getDescription(QString n){
    description = n;
}

void Geometry::getWidth(int a){
    width = a;
}

void Geometry::getHeight(int a){
    height = a;
}

void Geometry::getShapeName(QString n){
    shapes[shape_count].setShape(n);
}

void Geometry::getShapeCord(double a, double b){
    shapes[shape_count].setCordinate(a,b);
}

void Geometry::getShapeApprox(double a, double b){
    shapes[shape_count].setApprox(a,b);
}

void Geometry::getShape(){
    shape_count++;
}

void Geometry::display(){
    qDebug()<<name<<"\n"<<description<<"\nwidth: "<<width<<"\nheight: "<<height<<"\n"<<"sectionTop: "<<sectionTop;
    qDebug()<<"\nsectionLeft: "<<sectionLeft<<"\nrowTop: "<<rowTop<<"\nrowLeft: "<<rowLeft<<"\nkeyGap: "<<keyGap<<"\nkeyShape: "<<keyShape<<"\n";
    for (int i=0;i<shape_count;i++){
            shapes[i].display();
    }
    for (int j=0;j<sectionCount;j++)
            sectionList[j].displaySection();
}

void Geometry::addSection(){
    qDebug()<<"\nsectionCount: "<<sectionCount;
    sectionCount++;
}

Shape Geometry::findShape(QString name){
    Shape l;
    for(int i=0;i<shape_count;i++){
            if (shapes[i].sname == name){
                    return shapes[i];
            }
    }
    return l;
}







