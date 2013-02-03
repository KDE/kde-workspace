#include "kbgeometry.h"
#include <QtGui>

#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>
#include <fixx11h.h>
#include <config-workspace.h>
#include <KApplication>
#include <KLocale>
#include <QtCore/QFile>


KbGeometry::KbGeometry()
{
    defSectionLeft=0;
    defSectionTop=0;

    pcModel="pc101 pc102 pc104 pc105";
    pcgeometries = "latitude";
    macbooks = "macbook78 macbook79";
    applealu = "applealu_ansi applealu_iso applealu_jis";
    macs = "macintosh macintosh_old ibook powerbook macbook78 macbook79";
    dell = "dell101 dellm65";

}
int KbGeometry::setIntProp(QString a,QString b){
    int val;
    float f;
    if(b.contains(a)){
        QString dvl=b.mid(b.indexOf(a));
        dvl=dvl.left(dvl.indexOf(";"));
        dvl=dvl.mid(dvl.indexOf("=")+1);
        val=dvl.toFloat();
    }
    else
        val=0;
    return val;
}

QString KbGeometry::setStrProp(QString a,QString b){
    QString val;
    if(b.contains(a)){
        QString dvl=b.mid(b.indexOf(a));
        dvl=dvl.left(dvl.indexOf(";"));
        dvl=dvl.mid(dvl.indexOf("=")+1);
        val=dvl;
    }
    else
        val="";
    return val;
}

void KbGeometry::extractShape(QString desc){
    desc.remove(" ");
    desc.remove("\t");
    desc.remove("\n");
    QList<QString>shapes;
    shapes=desc.split("shape\"");
    for(int i=1;i<shapes.size();i++){
        Shape temp;
        QString sh=shapes.at(i);
        QString th=shapes.at(i);
        QMessageBox q;

        int idx=sh.indexOf("\"",1);
        QString shname=sh.left(idx);
        temp.name=shname;
        idx=sh.indexOf("[");
        sh=sh.mid(idx+1);


        idx=sh.indexOf("]");
        sh=sh.left(idx);

        QList<QString>sz=sh.split(",");
        temp.sizex=sz.at(0).toFloat();
        temp.sizey=sz.at(1).toFloat();
        if(temp.sizex==0||temp.sizey==0){

            idx=th.indexOf("[");
            int tdx=th.indexOf("[",idx+1);
            th=th.mid(tdx+1);
            idx=th.indexOf("]");
            th=th.left(idx);
            QList<QString>sd=th.split(",");
            temp.sizex=sd.at(0).toInt();
            temp.sizey=sd.at(1).toInt();
        }
        shapeList<<temp;
        sz.clear();
        QString f;
        sh=temp.name;
        sh.append(" : ");
        sh.append(f.setNum(temp.sizex));
        sh.append(" : ");
        sh.append(f.setNum(temp.sizey));
        //q.setText(sh);
        //q.exec();
    }
}

void KbGeometry::setDefaultValues(QString desc){
    desc.remove(" ");
    desc.remove("\t");
    QString g,l;
    defSectionLeft=setIntProp("section.left=",desc);
    l.setNum(defSectionLeft);
    g.append(l);
    g.append(" : ");
    defSectionTop=setIntProp("section.top=",desc);
    l.setNum(defSectionTop);
    g.append(l);
    g.append(" : ");

    defShape=setStrProp("key.shape=",desc);

    g.append(defShape);
    g.append(" : ");

    kbWidth=setIntProp("width=",desc);
    l.setNum(kbWidth);
    g.append(l);
    g.append(" : ");
    kbHieght=setIntProp("height=",desc);
    l.setNum(kbHieght);
    g.append(l);
    g.append(" : ");
    keygap=setIntProp("key.gap=",desc);
    l.setNum(keygap);
    g.append(l);
    g.append(" : ");
    //QMessageBox a;
    //a.setText(g);
    //a.exec();
}

void KbGeometry::extractSections(QString desc){
    QMessageBox q;
    desc.append("\n");
    QString sec=desc;
    QString ma;
    while(sec.contains("//")){
        int k=sec.indexOf("//");
        QString first=sec.left(k);
        ma.append(first);
        //q.setText(ma);
        //q.exec();
        int l=sec.indexOf("\n",k+1);
        sec=sec.mid(l);
        //q.setText(sec);
        //q.exec();
        //q.setText(first.setNum(k));
        //q.exec();

    }
    ma.append(sec);
    desc=ma;
    desc.remove(" ");
    desc.remove("\t");
    QString dm,m;
    QList<QString>seclist=desc.split("section\"");
    setDefaultValues(seclist.at(0));
    extractShape(desc);
    for(int i=1;i<seclist.size();i++){
        Section temp;
        QList<QString>rlst=seclist.at(i).split("row{");
        QString inp=rlst.at(0);
        int top=setIntProp("top=",inp);
        temp.ycordi=top+defSectionTop;
        dm.setNum(temp.ycordi);
        dm.append("=");
        m.setNum(top);
        dm.append(m);
        dm.append("+");
        m.setNum(defSectionTop);
        dm.append(m);
        dm.append("\n");
        temp.secShape=setStrProp("key.shape=",inp);
        if(temp.secShape=="")
            temp.secShape=defShape;
        temp.gap=setIntProp("key.gap=",inp);
        if(temp.gap==0)
            temp.gap=keygap;
        int left=setIntProp("left=",inp);
        temp.xcordi=left+defSectionLeft;
        m.setNum(temp.xcordi);
        dm.append(m);
        dm.append("=");
        m.setNum(left);
        dm.append(m);
        dm.append("+");
        m.setNum(defSectionLeft);
        dm.append(m);
        dm.append("\n");
        for(int j=1;j<rlst.size();j++){
            temp.rowlist<<rlst.at(j);
            dm.append(rlst.at(j));
            dm.append("\n");
        }
        sections<<temp;
        //q.setText(dm);
        //q.exec();
    }
}

QStringList KbGeometry :: splitKeys(QString desc){
    desc.remove(" ");
    desc.remove("\t");
    desc.remove("\n");
    QList <QString> lst=desc.split(",");
    QList <QString> klst;
    while(!lst.empty()){
        QString temp=lst.first();
        if(temp=="};")
            break;
        lst.pop_front();
        if(temp.contains("{")){
            temp.remove("{");
            while(!lst.at(0).contains("}")){
                temp.append(";");
                temp.append(lst.first());
                lst.pop_front();
            }
            temp.append(";");
            temp.append(lst.first());
            lst.pop_front();
            if(temp.contains("};"))
                temp=temp.left(temp.indexOf("};"));
            temp.remove("}");
        }
        else{
            if(temp.contains("};"))
                temp=temp.left(temp.indexOf("};"));
        }
        klst << temp;


    }
    return klst;
}

void KbGeometry::extractRows(QString des){
    extractSections(des);
    for(int j=0;j<sections.size();j++){
        Section current=sections.at(j);
        for(int k=0;k<current.rowlist.size();k++){

            QString desc=current.rowlist.at(k);
            desc=desc.left(desc.indexOf("};"));
            desc.remove(" ");
            desc.remove("\t");
            Row temp;
            temp.cx=setIntProp("left=",desc)+current.xcordi;
            temp.cy=setIntProp("top=",desc)+current.ycordi;
            QString tshape=setStrProp("key.shape=",desc);
            if (tshape==""){
                tshape=current.secShape.remove("\"");
            }
            temp.shapeRow=tshape.remove("\"");
            desc=desc.mid(desc.indexOf("keys{")+5);
            temp.gap=setIntProp("key.gap=",desc);
            if(temp.gap==0)
                temp.gap=current.gap;
            QMessageBox q;
            //QString tst;
            //tst.setNum(setIntProp("left=",desc));
            //q.setText(tst);
            //q.exec();
            temp.keylist=splitKeys(desc);
            rows<<temp;

       }

    }
    QMessageBox q;
    for(int i=0;i<rows.size();i++){
        QString a,b;
        a.append(rows.at(i).shapeRow);
        a.append(" : ");
        a.append(b.setNum(rows.at(i).cx));
        a.append(" : ");
        a.append(b.setNum(rows.at(i).cy));
        for(int n=0;n<rows.at(i).keylist.size();n++){
            a.append("\n");
            a.append(rows.at(i).keylist.at(n));
        }
      //q.setText(a);
       //q.exec();
    }
}

Shape KbGeometry :: getShape(QString shape){
    for(int i=0;i<shapeList.size();i++){
        Shape tmp=shapeList.at(i);
        if(tmp.name==shape)
            return tmp;
    }
    Shape k=getShape("NORM");
    return k;
}

void KbGeometry :: extractKeys(QString model){

    QString desc=getGeometryDescription(getGeometryfromModel(model));
    extractRows(desc);
    for(int i=0;i<rows.size();i++){
        int totksz=0;
        int gap=rows.at(i).gap;
        for(int j=0;j<rows.at(i).keylist.size();j++){
            KeyGm temp;
            int addx=0;
            QString dummy=rows.at(i).keylist.at(j);
            QMessageBox g;
            //g.setText(dummy);
            //g.exec();
            dummy.remove(" ");
            int check=0;
            if(dummy.contains(";")){
                QList <QString>dummylst=dummy.split(";");
                int z=0;
                for(int k=0;k<dummylst.size();k++){
                    QString d=dummylst.at(k);
                    if(d.startsWith("<")){
                        d.remove("<");
                        d.remove(">");
                        temp.name=d;
                    }
                    if(d.startsWith("\"")){
                        d.remove("\"");
                        check=1;
                        //g.setText(d);
                        //g.exec();
                        temp.shape=getShape(d);
                    }
                    if(d.startsWith("shape=")){
                        d=setStrProp("shape=",d);
                        d.remove("\"");
                        temp.shape=getShape(d);
                        check=1;
                    }
                    bool ok;
                    if(z==0){
                        addx=d.toInt(&ok,10);
                        if(ok)
                            z=1;
                    }
                }
            }
            else{
                dummy.remove("<");
                dummy.remove(">");
                temp.name=dummy;
                //g.setText(rows.at(i).shapeRow);
                //g.exec();

            }
            if(check==0)
                temp.shape=getShape(rows.at(i).shapeRow);
            if(temp.shape.name=="")
                temp.shape=getShape("NORM");
            temp.cordx=rows.at(i).cx+totksz+addx+gap;
            totksz+=temp.shape.sizex+addx+gap;
            temp.cordy=rows.at(i).cy;
            keys<<temp;
        }
    }
    //QMessageBox q;
    QString a,b;
    int ptop=0;
    for(int i=0;i<keys.size();i++){

        KeyGm s=keys.at(i);
        if(ptop!=s.cordy){
            ptop=s.cordy;
            a.append("\n");
        }
        a.append(s.name);
        a.append(" : ");
        a.append(b.setNum(s.cordx));
        a.append(" : ");
        a.append(b.setNum(s.cordy));
        a.append(" : ");
        a.append(s.shape.name);
        a.append("  ");


    }
    QMessageBox q;
    //q.setText(a);
    //q.exec();
}

QString KbGeometry::makeGeometryStr(QString file,QString model){
    QString geometry;
    geometry.append(file);
    geometry.append("/");
    geometry.append(model);
    return geometry;
}

QString KbGeometry::getGeometryfromModel(QString model){

    QString geometry;

    if(pcModel.contains(model))
        return geometry=makeGeometryStr("pc",model);

    if(pcgeometries.contains(model))
        return geometry=makeGeometryStr("pc",model);

    if(model=="apple")
        return geometry=makeGeometryStr("macintosh","macintosh");
    if(macbooks.contains(model)||macs.contains(model))
        return geometry=makeGeometryStr("macintosh","macintosh");

    if(applealu.contains(model))
        return geometry=makeGeometryStr("macintosh",model);

    if(dell.contains(model))
        return geometry=makeGeometryStr("dell",model);

    if(model=="hpmini110")
        return geometry=makeGeometryStr("hp","mini110");

    if(model=="hpdv5")
        return geometry=makeGeometryStr("hp","dv5");

    if(model=="pc98")
        return geometry=makeGeometryStr("nec",model);

    QMessageBox q;
    q.setText(i18n("Showing in Default Geometry"));
    q.exec();

    return geometry="pc/pc104";

}


QString KbGeometry::getGeometryDescription(QString geometry){

    QStringList temp=geometry.split("/");
    QString description;
    QString filename=findGeometryBasedir();
    filename.append(temp.at(0));

    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = file.readAll();
    file.close();

    QString geometryName=temp.at(1);
    temp=content.split("xkb_geometry");
    for(int i=1;i<temp.size();i++){
        QString tempstr=temp.at(i);
        int k=tempstr.indexOf("\"");
        tempstr=tempstr.mid(k);
        k=tempstr.indexOf("{");
        tempstr=tempstr.left(k);
        tempstr.remove(" ");
        tempstr.remove("\"");
        if(tempstr==geometryName){
            description=temp.at(i);
            break;
        }
    }
    return description;
}

QString KbGeometry::findGeometryBasedir()
{
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
    symBasedir=QString("%1/xkb/geometry/").arg(xkbParentDir);
    return(symBasedir);
}
