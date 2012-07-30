#ifndef KEYBOARDLAYOUT_H
#define KEYBOARDLAYOUT_H
#include"keys.h"
#include<QApplication>
class Keyboardlayout
{
public:
    Keyboardlayout();
    QString Layoutname;
    Keys AE[12];
    Keys AD[12];
    Keys AC[11];
    Keys AB[11];
    void getLayout(QString a);
    void includeSymbol(QString a);
    QString latkeys(QString a);
};
#endif // KEYBOARDLAYOUT_H
