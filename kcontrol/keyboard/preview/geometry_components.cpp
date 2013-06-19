#include "geometry_components.h"

#include <iostream>

cordinate::cordinate(){
    x=0;
    y=0;
}

cordinate::cordinate(int &a, int &b){
    x=a;
    y=b;
}

Shape::Shape(){
    cordi_count = 0;
}

void Shape::setShape(std::string n){
    sname = n;
}

void Shape::setCordinate(int &a, int &b){
    cordii[cordi_count++] = cordinate(a,b);
}

void Shape::setApprox(int &a, int &b){
    approx = cordinate(a,b);
}

void Shape::display(){
    std::cout<<"shape: "<<sname<<"\n";
    std::cout<<"( "<<approx.x<<", "<<approx.y<<"); ";
    for(int i=0;i<cordi_count;i++)
            std::cout<<"( "<<cordii[i].x<<", "<<cordii[i].y<<"); ";
    std::cout<<"\n";
}

int Shape::size(){
    if (approx.x == 0 && approx.y== 0)
        return cordii[0].x;
    else
        return approx.x;
}

Key::Key(){
    offset = 0;
}
void Key::getKey(int &o){
    offset = o;
}

void Key::setKeyPosition(int &x, int &y){
    position = cordinate(x,y);
}

void Key::showKey(){
    std::cout<<"\n\tKey: "<<name<<"\tshape: "<<shapeName<<"\toffset: "<<offset;
    std::cout<<"\tposition"<<position.x<<" , "<<position.y<<std::endl;
}

Row::Row(){
    top = 0;
    left = 0;
    keyCount = 0;
}

void Row::getRow(int t,int l){
    top = t;
    left = l;
}

void Row::addKey(){
    std::cout<<"keyCount: "<<keyCount;
    keyCount++;
}

void Row::displayRow(){
    std::cout<<"\nRow: "<<top<<","<<left<<std::endl;
    for(int i=0;i<keyCount;i++)
    keyList[i].showKey();
}

Section::Section(){
    top = 0;
    left = 0;
    angle = 0;
    rowCount = 0;
}

void Section::getName(std::string n){
    name = n;
}

void Section::addRow(){
    std::cout<<"\nrowCount: "<<rowCount;
    rowCount++;
}

void Section::displaySection(){
    std::cout<<"\nSection: "<<name<<"\n\tposition: ("<<left<<","<<top<<");"<<angle<<std::endl;
    for(int i=0;i<rowCount;i++){
            std::cout<<"\n\t";
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

void Geometry::getName(std::string n){
    name = n;
}

void Geometry::getDescription(std::string n){
    description = n;
}

void Geometry::getWidth(int a){
    width = a;
}

void Geometry::getHeight(int a){
    height = a;
}

void Geometry::getShapeName(std::string n){
    shapes[shape_count].setShape(n);
}

void Geometry::getShapeCord(int a, int b){
    shapes[shape_count].setCordinate(a,b);
}

void Geometry::getShapeApprox(int a, int b){
    shapes[shape_count].setApprox(a,b);
}

void Geometry::getShape(){
    shape_count++;
}

void Geometry::display(){
    std::cout<<name<<"\n"<<description<<"\nwidth: "<<width<<"\nheight: "<<height<<"\n"<<"sectionTop: "<<sectionTop;
    std::cout<<"\nsectionLeft: "<<sectionLeft<<"\nrowTop: "<<rowTop<<"\nrowLeft: "<<rowLeft<<"\nkeyGap: "<<keyGap<<"\nkeyShape: "<<keyShape<<std::endl;
    for (int i=0;i<shape_count;i++){
            shapes[i].display();
    }
    for (int j=0;j<sectionCount;j++)
            sectionList[j].displaySection();
}

void Geometry::addSection(){
    std::cout<<"\nsectionCount: "<<sectionCount;
    sectionCount++;
}

Shape Geometry::findShape(std::string name){
    Shape l;
    for(int i=0;i<shape_count;i++){
            if (shapes[i].sname == name){
                    return shapes[i];
            }
    }
    return l;
}







