#include "keyboardlayout.h"
#include<QList>
#include"keys.h"
#include<QMessageBox>
#include<QFile>
Keyboardlayout::Keyboardlayout()
{
}
void Keyboardlayout::getLayout(QString a){
    includeSymbol(a);
    int i=a.indexOf("name[Group1]=");
    i+=13;
    QString n=a.mid(i);
    n=n.simplified();
    i=n.indexOf("\"",1);
    Layoutname=n.left(i);
    Layoutname.remove("\"");
    Layoutname.simplified();
    i=n.indexOf("key");
    n=n.mid(i);
    QList<QString> st;
    st=n.split("key");
    Keys dum;
    QString r,y;
    for(int k=0;k<st.size();k++){
        dum.getKey(st.at(k));
        if(dum.keyname.startsWith("Lat"))
            dum.keyname=latkeys(dum.keyname);
        if(dum.keyname.contains("AE")){
            QString ind=dum.keyname.right(2);
            int index=ind.toInt();
            r=st.at(k);
            AE[index-1].getKey(r);
        }
        if(dum.keyname.contains("AD")){
            QString ind=dum.keyname.right(2);
            int index=ind.toInt();
            r=st.at(k);
            AD[index-1].getKey(r);
        }
        if(dum.keyname.contains("AC")){
            QString ind=dum.keyname.right(2);
            int index=ind.toInt();
            r=st.at(k);
            AC[index-1].getKey(r);
        }
        if(dum.keyname.contains("AB")){
            QString ind=dum.keyname.right(2);
            int index=ind.toInt();
            r=st.at(k);
            AB[index-1].getKey(r);
        }
    }
    /*QString e;
    e=Layoutname;
    for(int l=0;l<12;l++){
        e.append(AE[l].keyname);
        for(int u=0;u<AE[l].klst.size();u++){
            e.append(AE[l].klst.at(u));
        }
        e.append("\n");
    }
    for(int l=0;l<12;l++){
        e.append(AD[l].keyname);
        for(int u=0;u<AD[l].klst.size();u++){
            e.append(AD[l].klst.at(u));
        }
        e.append("\n");
    }
    for(int l=0;l<11;l++){
        e.append(AC[l].keyname);
        for(int u=0;u<AC[l].klst.size();u++){
            e.append(AC[l].klst.at(u));
        }
        e.append("\n");
    }
    for(int l=0;l<10;l++){
        e.append(AB[l].keyname);
        for(int u=0;u<AB[l].klst.size();u++){
            e.append(AB[l].klst.at(u));
        }
        e.append("\n");
    }
    QMessageBox q;
    q.setText(e);
    q.exec();*/
}
void Keyboardlayout::includeSymbol(QString a){
    QList<QString>tobeinclude;
    tobeinclude=a.split("include");
    //int k=a.indexOf("include");
    QMessageBox s;
    QString r;
    for(int o=0;o<tobeinclude.size();o++){
        QString d=tobeinclude.at(o);
        d.simplified();
    //s.setText(d);
    //s.exec();
        int k=d.indexOf("\"",2);
    //s.setText(r.setNum(k));
    //s.exec();
        QString incsym=d.left(k);
        incsym.remove(" ");
        incsym.remove("\"");
        QMessageBox q;
        QList<QString> incfile;
        incfile=incsym.split("(");
        for(int i=0;i<incfile.size();i++){
                QString z=incfile.at(i);
                z.remove(" ");
            incfile[i]=z;
        }
        if(incfile.size()==1)
            incfile<<"basic";
        else{
            QString ns=incfile.at(1);
            ns.remove(")");
            incfile[1]=ns;
        }
        r=incfile.at(0);
        r.append(incfile.at(1));
    //q.setText(r);
    //q.exec();
        QString filename="/usr/share/X11/xkb/symbols/";
        filename.append(incfile.at(0));
    //q.setText(filename);
    //q.exec();
        QFile file(filename);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString content = file.readAll();
        QList<QString> symstrlist;
        symstrlist=content.split("xkb_symbols ");
        for(int u=0;u<symstrlist.size();u++){
            QString cur=symstrlist.at(u);
            int pos = cur.indexOf("{");
            cur=cur.left(pos);
            if(cur.contains(incfile.at(1))){
                getLayout(symstrlist.at(u));
                break;
            }
        }
    }
}
QString Keyboardlayout::latkeys(QString a){
    if(a.startsWith("Lat")){
    if(a=="LatQ")
        return ("AD01");
    if(a=="LatW")
        return ("AD02");
    if(a=="LatE")
        return ("AD03");
    if(a=="LatR")
        return ("AD04");
    if(a=="LatT")
        return ("AD05");
    if(a=="LatY")
        return ("AD06");
    if(a=="LatU")
        return ("AD07");
    if(a=="LatI")
        return ("AD08");
    if(a=="LatO")
        return ("AD09");
    if(a=="LatP")
        return ("AD10");
    if(a=="LatA")
        return ("AC01");
    if(a=="LatS")
        return ("AC02");
    if(a=="LatD")
        return ("AC03");
    if(a=="LatF")
        return ("AC04");
    if(a=="LatG")
        return ("AC05");
    if(a=="LatH")
        return ("AC06");
    if(a=="LatJ")
        return ("AC07");
    if(a=="LatK")
        return ("AC08");
    if(a=="LatL")
        return ("AC09");
    if(a=="LatZ")
        return ("AB01");
    if(a=="LatX")
        return ("AB02");
    if(a=="LatC")
        return ("AB03");
    if(a=="LatV")
        return ("AB04");
    if(a=="LatB")
        return ("AB05");
    if(a=="LatN")
        return ("AB06");
        return ("AB07");
    }
    else
        return ("a");
}
