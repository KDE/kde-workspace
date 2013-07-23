#ifndef GEOMETRY_COMPONENTS_H
#define GEOMETRY_COMPONENTS_H

#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtCore/QList>

class GShape{
  public:
    QString sname;
    static const int maxShapeCordinates = 20;
    QPoint approx,cordii[maxShapeCordinates];
    int cordi_count;
    
    GShape();
    void setCordinate(double a, double b);
    void setApprox(double a, double b);
    void display();
    double size(int vertical);
};

class Key{
public:
  QString name,shapeName;
  double offset;
  QPoint position;
  
  Key();
  void setKeyPosition(double x,double y);
  void showKey();
};

class Row{
public:
  double top,left;
  int keyCount,vertical;
  QString shapeName;
  static const int maxKeys = 50;
  Key keyList[maxKeys];
  
  Row();
  void addKey();
  void displayRow();
};

class Section{
public:
  QString name,shapeName;
  double top,left,angle;
  int rowCount,vertical;
  static const int maxRows = 50;
  Row rowList[maxRows];
  
  
  Section();
  void addRow();
  void displaySection();
};

class Geometry{
public:
  QString name,description,keyShape;
  int shape_count,vertical;
  double width,height,sectionTop,sectionLeft,rowTop,rowLeft,keyGap;
  int sectionCount;
  static const int maxShapes = 50, maxSections = 20;
  GShape shapes[maxShapes];
  Section sectionList[maxSections];
  
  Geometry();
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
