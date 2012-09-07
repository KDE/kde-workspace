#include "keyboardlayout.h"
#include<QList>
#include"keys.h"
#include<QMessageBox>
#include<QFile>

#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>
#include <fixx11h.h>
#include <config-workspace.h>
#include<QDir>


Keyboardlayout::Keyboardlayout()
{
}
void Keyboardlayout::getLayout(QString a,QString cname){
    includeSymbol(a,cname);
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
            dum.keyname=alias.getAlias(cname,dum.keyname);
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
void Keyboardlayout::includeSymbol(QString a,QString cname){
    int k=a.indexOf("include");
    a=a.mid(k);
    /*QMessageBox x;
    x.setText(a);
    x.exec();*/
    QList<QString>tobeinclude;
    tobeinclude=a.split("include");
    //QMessageBox s;
    QString r;
    for(int o=1;o<tobeinclude.size();o++){
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
        //QMessageBox q;
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
        QString filename=findSymbolbasedir();
        filename.append(incfile.at(0));
    //q.setText(filename);
    //q.exec();
        QFile file(filename);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString content = file.readAll();
        QList<QString> symstrlist;
        symstrlist=content.split("xkb_symbols ");
        for(int u=1;u<symstrlist.size();u++){
            QString cur=symstrlist.at(u);
            int pos = cur.indexOf("{");
            cur=cur.left(pos);
            if(cur.contains(incfile.at(1))){
                getLayout(symstrlist.at(u),cname);
                break;
            }
        }
    }
}

QString Keyboardlayout::findSymbolbasedir(){

    QString symBasedir;
    QString xkbParentDir;

    QString base(XLIBDIR);
    if( base.count('/') >= 3 ) {
        // .../usr/lib/X11 -> /usr/share/X11/xkb vs .../usr/X11/lib -> /usr/X11/share/X11/xkb
        QString delta = base.endsWith("X11") ? "/../../share/X11" : "/../share/X11";
        QDir baseDir(base + delta);
        if( baseDir.exists() ) {
            xkbParentDir = baseDir.absolutePath();
        }
        else {
            QDir baseDir(base + "/X11");	// .../usr/X11/lib/X11/xkb (old XFree)
            if( baseDir.exists() ) {
                xkbParentDir = baseDir.absolutePath();
            }
        }
    }

    if( xkbParentDir.isEmpty() ) {
        xkbParentDir = "/usr/share/X11";
    }
    symBasedir=QString("%1/xkb/symbols/").arg(xkbParentDir);
    return(symBasedir);
}
