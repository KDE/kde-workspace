#include "keyaliases.h"
#include<QString>
#include<QMap>
#include<QMessageBox>
#include<QFile>

Aliases::Aliases()
{
    QString filename="/usr/share/X11/xkb/keycodes/aliases";
    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = file.readAll();
    file.close();
    QList<QString>als;
    als=content.split("xkb_keycodes");
    for(int i=1;i<als.size();i++){
        QString temp=als.at(i);
        temp=temp.remove(" ");
        temp=temp.remove("\n");
        temp=temp.remove("\"");
        temp=temp.remove(">");
        temp=temp.remove("<");
        temp=temp.remove(";");
        temp=temp.remove("}");
        temp=temp.remove("{");
        QList<QString>alskeys;
        alskeys=temp.split("alias");
        if(temp.startsWith("qwerty")){
            for(int k=1;k<alskeys.size();k++){
                QString tmp=alskeys.at(k);
                int inofeq=tmp.indexOf("=");
                QString lat=tmp.left(inofeq);
                QString key=tmp.mid(inofeq+1);
                qwerty[lat]=key;
            }
        }
        if(temp.startsWith("azerty")){
            for(int k=1;k<alskeys.size();k++){
                QString tmp=alskeys.at(k);
                int inofeq=tmp.indexOf("=");
                QString lat=tmp.left(inofeq);
                QString key=tmp.mid(inofeq+1);
                azerty[lat]=key;
            }
      }
       if(temp.startsWith("qwertz")){
            for(int k=1;k<alskeys.size();k++){
                 QString tmp=alskeys.at(k);
                 int inofeq=tmp.indexOf("=");
                 QString lat=tmp.left(inofeq);
                 QString key=tmp.mid(inofeq+1);
                 qwertz[lat]=key;
            }
       }
   }

}

QString Aliases::getAlias(QString cname, QString name){
    QMessageBox q;
    QString a=name;
    if(cname=="ma"){
        a=azerty.value(name);
    }
    else{
        a=qwerty.value(name);
    }
    return a;
}

