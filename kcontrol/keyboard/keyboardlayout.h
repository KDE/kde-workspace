#ifndef KEYBOARDLAYOUT_H
#define KEYBOARDLAYOUT_H
#include"keys.h"
#include"keyaliases.h"
#include<QApplication>
class KeyboardLayout
{
public:
    KeyboardLayout();
    QString Layoutname;
    Aliases alias;
    Keys TLDE;
    Keys AE[12];
    Keys AD[12];
    Keys AC[11];
    Keys AB[11];
    void getLayout(QString a,QString cname);
    QString findSymbolbasedir();
    void includeSymbol(QString a,QString cname);
};
#endif // KEYBOARDLAYOUT_H
