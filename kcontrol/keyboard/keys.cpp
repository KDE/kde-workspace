#include "keys.h"
#include<QApplication>
#include<QStringList>
Keys::Keys()
{
}
void Keys::getKey(QString a){
    int i=a.indexOf("<");
    i++;
    keyname=a.mid(i,4);
    keyname.simplified();
    i=a.indexOf("[");
    i++;
    QString str=a.mid(i);
    i=str.indexOf("]");
    QString st=str.left(i);
    st=st.simplified();
    //QStringList klst;
    klst=st.split(",");
    for(int k=0;k<klst.size();k++){
        QString du=klst.at(k);
        du.remove(" ");
        klst[k]=du;
    }
}
