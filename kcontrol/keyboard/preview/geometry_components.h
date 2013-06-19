#ifndef GEOMETRY_COMPONENTS_H
#define GEOMETRY_COMPONENTS_H

#include <iostream>


class cordinate{
  public : 
    int x,y;
    cordinate();
    cordinate(int &a,int &b);

};

class Shape{
  public:
    std::string sname;
    cordinate cordii[20],approx;
    int cordi_count;
    Shape();
    void setShape(std::string n);
    void setCordinate(int &a, int &b);
    void setApprox(int &a, int &b);
    void display();
    int size();
};

class Key{
public:
  std::string name,shapeName;
  int offset;
  cordinate position;
  Key();
  void getKey(int &o);
  void setKeyPosition(int &x,int &y);
  void showKey();
};

class Row{
public:
  int top,left,keyCount;
  Key keyList[30];
  Row();
  void getRow(int t,int l);
  void addKey();
  void displayRow();
};

class Section{
public:
  std::string name,shapeName;
  int top,left,angle,rowCount;
  Row rowList[20];
  Section();
  void getName(std::string n);
  void addRow();
  void displaySection();
};

class Geometry{
public:
  std::string name,description,keyShape;
  int width,height,shape_count;
  int sectionTop,sectionLeft,rowTop,rowLeft,keyGap,sectionCount;
  Shape shapes[40];
  Section sectionList[20];
  Geometry();
  void getName(std::string n);
  void getDescription(std::string n);
  void getWidth(int a);
  void getHeight(int a);
  void getShapeName(std::string n);
  void getShapeCord(int a, int b);
  void getShapeApprox(int a, int b);
  void getShape();
  void display();
  void addSection();
  Shape findShape(std::string name);
};



#endif //geometry_componets.h
