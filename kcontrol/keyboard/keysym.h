#ifndef KEYSYM_H
#define KEYSYM_H
#include<QtGui>
#include<QChar>
#include<QString>
class Keysym
{
public:
    Keysym();
    int nill;
    static QString keystr[];
    static QChar keyuni[];
    QString getkeyuni(QString opton);
};

#endif // KEYSYM_H
