#ifndef GEOMETRY_COMPONENTS_H
#define GEOMETRY_COMPONENTS_H

#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtCore/QList>

class GShape{
  private:
    QString sname;
    QPoint approx;
    QList <QPoint> cordii;
    int cordi_count;
  public:
    GShape();
    void setCordinate(double a, double b);
    void setApprox(double a, double b);
    QPoint getCordii(int i) const;
    void display();
    double size(int vertical) const;

    void setShapeName(QString n){
        sname  = n;
    }

    QPoint getApprox() const{
        return approx;
    }

    QString getShapeName(){
        return sname;
    }

    int getCordi_count() const{
        return cordi_count;
    }
};

class Key{
 private:
  QString name,shapeName;
  double offset;
  QPoint position;

public:
  Key();
  void setKeyPosition(double x, double y);

  void setOffset(double o){
      offset = o;
  }

  void setKeyName(QString n){
      name = n;
  }

  void setShapeName(QString n){
      shapeName = n;
  }

  QString getName(){
      return name;
  }

  QString getShapeName(){
      return shapeName;
  }

  double getOffset(){
      return offset;
  }

  QPoint getPosition(){
      return position;
  }

  void showKey();
};

class Row{
private:
  double top, left;
  int keyCount, vertical;
  QString shapeName;

public :
  QList <Key> keyList;
  
  Row();
  void addKey();

  void setTop(double t){
      top = t;
  }

  void setLeft(double l){
      left = l;
  }

  void setVertical(int v){
      vertical = v;
  }

  void setShapeName(QString n){
      shapeName = n;
  }

  double getTop(){
      return top;
  }

  double getLeft(){
      return left;
  }

  int getKeyCount(){
      return keyCount;
  }

  int getVertical(){
      return vertical;
  }

  QString getShapeName(){
      return shapeName;
  }

  void displayRow();
};

class Section{
private:
  QString name, shapeName;
  double top, left, angle;
  int rowCount, vertical;

public:
  QList <Row> rowList;
  
  
  Section();
  void addRow();

  void setName(QString n){
      name = n;
  }

  void setShapeName(QString n){
      shapeName = n;
  }

  void setTop(double t){
      top = t;
  }

  void setLeft(double l){
      left = l;
  }

  void setAngle(double a){
      angle = a;
  }

  void setVertical(int v){
      vertical = v;
  }

  QString getName(){
      return name;
  }

  QString getShapeName(){
      return shapeName;
  }

  double getTop(){
      return top;
  }

  double getLeft(){
      return left;
  }

  double getAngle(){
      return angle;
  }

  int getVertical(){
      return vertical;
  }

  int getRowCount(){
      return rowCount;
  }

  void displaySection();
};

class Geometry{
private:
  QString name, description, keyShape;
  int shape_count, vertical;
  int sectionCount;

public:
  QList <GShape> shapes;
  QList <Section> sectionList;
  double width, height, sectionTop, sectionLeft, rowTop, rowLeft, keyGap;

  Geometry();

  void setWidth(double a){
      width = a;
  }

  void setHeight(double a){
      height = a;
  }

  void setName(QString n){
      name = n;
  }

  void setDescription(QString d){
      description = d;
  }

  void setKeyShape(QString s){
      keyShape = s;
  }

  void setVertical(int v){
      vertical = v;
  }

  double getWidth(){
      return width;
  }

  double getHeight(){
      return height;
  }

  QString getName(){
      return name;
  }

  QString getDescription(){
      return description;
  }

  QString getKeyShape(){
      return keyShape;
  }

  int getVertical(){
      return vertical;
  }

  int getShapeCount(){
    return shape_count;
  }

  int getSectionCount(){
      return sectionCount;
  }

  void setShapeName(QString n);
  void setShapeCord(double a, double b);
  void setShapeApprox(double a, double b);
  void addShape();
  void display();
  void addSection();
  GShape findShape(QString name);
};



#endif //geometry_componets.h
