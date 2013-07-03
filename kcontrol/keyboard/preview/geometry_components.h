#ifndef GEOMETRY_COMPONENTS_H
#define GEOMETRY_COMPONENTS_H

#include <QtCore/QString>
#include <QtCore/QPoint>

class GShape{
  public:
    QString sname;
    QPoint cordii[20],approx;
    int cordi_count;
    GShape();
    void setShape(QString n);
    void setCordinate(double &a, double &b);
    void setApprox(double &a, double &b);
    void display();
    double size(int vertical);
};

class Key{
public:
  QString name,shapeName;
  double offset;
  QPoint position;
  Key();
  void getKey(double &o);
  void setKeyPosition(double &x,double &y);
  void showKey();
};

class Row{
public:
  double top,left;
  int keyCount,vertical;
  QString shapeName;
  Key keyList[50];
  Row();
  void getRow(double t,double l);
  void addKey();
  void displayRow();
};

class Section{
public:
  QString name,shapeName;
  double top,left,angle;
  int rowCount,vertical;
  Row rowList[20];
  Section();
  void getName(QString n);
  void addRow();
  void displaySection();
};

class Geometry{
public:
  QString name,description,keyShape;
  int shape_count,vertical;
  double width,height,sectionTop,sectionLeft,rowTop,rowLeft,keyGap;
  int sectionCount;
  GShape shapes[40];
  Section sectionList[20];
  Geometry();
  void getName(QString n);
  void getDescription(QString n);
  void getWidth(double a);
  void getHeight(double a);
  void getShapeName(QString n);
  void getShapeCord(double a, double b);
  void getShapeApprox(double a, double b);
  void getShape();
  void display();
  void addSection();
  GShape findShape(QString name);
};



#endif //geometry_componets.h
