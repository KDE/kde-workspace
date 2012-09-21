#ifndef KEYSYM_H
#define KEYSYM_H
#include<QtGui>
#include<QChar>
#include<QString>
class KeySym
{
public:
    KeySym();
    int nill;
    static QString keystr[];
    static QChar keyuni[];
    QString getkeyuni(QString opton);
};

#endif // KEYSYM_H
