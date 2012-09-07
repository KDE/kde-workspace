#ifndef KEYS_H
#define KEYS_H
#include<QApplication>
#include<QList>
class Keys
{
public:
    Keys();
    QString keyname;
    QList<QString>klst;
    void getKey(QString a);
};

#endif // KEYS_H
