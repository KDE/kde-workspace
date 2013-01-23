#ifndef KBGEOMETRY_H
#define KBGEOMETRY_H
#include<QtCore>

struct Shape{
    QString name;
    int sizex,sizey;
};

struct KeyGm {
    QString name;
    int cordx,cordy;
    Shape shape;
};

struct Row{
    int cx,cy,gap;
    QString shapeRow;
    QList<QString> keylist;
};

struct Section{
    int xcordi,ycordi;
    QList<QString> rowlist;
    int gap;
    QString secShape;
};

class KbGeometry
{
    QList <Shape> shapeList;
    QList <Section> sections;
    QList <Row> rows;
    QString defShape;
    int defSectionLeft,defSectionTop;
    int setIntProp(QString a, QString b);
    QString setStrProp(QString a,QString b);
    Shape getShape(QString shape);
    QString pcModel,pcgeometries,macbooks,applealu,macs,dell;
    void extractShape(QString desc);
    void setDefaultValues(QString desc);
    void extractSections(QString desc);
    void extractRows(QString desc);
    QString makeGeometryStr(QString file,QString model);
    QString getGeometryfromModel(QString model);
    QString getGeometryDescription(QString geometry);
    QString findGeometryBasedir();
public:
    QList <KeyGm> keys;
    KbGeometry();
    int kbWidth,kbHieght,keygap;
    void extractKeys(QString desc);
    QStringList splitKeys(QString desc);
};

#endif // KBGEOMETRY_H
