#include "geometry_parser.h"
#include "geometry_components.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <QFileDialog>
#include <QFile>


#include <fixx11h.h>
#include <config-workspace.h>

namespace grammar{
keywords::keywords(){
    add
       ("shape",1)
       ("height",2)
       ("width",3)
       ("description",4)
       ("keys",5)
       ("row",6)
       ("section",7)
       ("key",8)
       ("//",9)
       ("/*",10)
       ;

}

template<typename Iterator>
Geometry_parser<Iterator>::Geometry_parser():Geometry_parser::base_type(start){
    using qi::lexeme;
    using qi::char_;
    using qi::lit;
    using qi::_1;
    using qi::_val;
    using qi::int_;
    using qi::double_;
    using qi::eol;


    name = '"'>>+(char_-'"')>>'"';

    ignore =(lit("outline")||lit("overlay")||lit("text"))>>*(char_-lit("};"))>>lit("};")
            ||lit("solid")>>*(char_-lit("};"))>>lit("};")
            ||lit("indicator")>>*(char_-';'-'{')>>';'||'{'>>*(char_-lit("};"))>>lit("};")
            ||lit("indicator")>>'.'>>lit("shape")>>'='>>name>>';';

    comments =lexeme[ lit("//")>>*(char_-eol||kw-eol)>>eol||lit("/*")>>*(char_-lit("*/")||kw-lit("*/"))>>lit("*/") ];

    cordinates = ('['
            >>double_[phx::ref(x)=_1]
            >>','
            >>double_[phx::ref(y)=_1]
            >>']')
            ||'['>>double_>>",">>double_>>']'
            ;

    cordinatea = '['>>double_[phx::ref(ax)=_1]>>",">>double_[phx::ref(ay)=_1]>>']';

    set = '{'>>cordinates>>*(','>>cordinates)>>'}';

    setap = '{'>>cordinatea>>*(','>>cordinatea)>>'}';

    seta = '{'
            >>cordinates[phx::bind(&Geometry_parser::setCord,this)]
            >>*(','>>cordinates[phx::bind(&Geometry_parser::setCord,this)])
            >>'}'
            ;

    description = lit("description")>>'='>>name[phx::bind(&Geometry_parser::getDescription,this,_1)]>>';';

    cornerRadius = (lit("cornerRadius")||lit("corner"))>>'='>>double_;

    shapeDef = lit("shape")
            >>name[phx::bind(&Geometry_parser::getShapeName,this,_1)]
            >>'{'
            >>*(lit("approx")>>'='>>setap[phx::bind(&Geometry_parser::setApprox,this)]>>','||cornerRadius>>','||comments)
            >>seta
            >>*((','>>(set||lit("approx")>>'='>>setap[phx::bind(&Geometry_parser::setApprox,this)]||cornerRadius)||comments))
            >>lit("};")
            ;

    keyName = '<'>>+(char_-'>')>>'>';

    keyShape = *(lit("key."))>>lit("shape")>>'='>>name[phx::bind(&Geometry_parser::setKeyShape,this,_1)]
            ||name[phx::bind(&Geometry_parser::setKeyShape,this,_1)];

    keyColor = lit("color")>>'='>>name;

    keygap = lit("gap")>>'='>>double_[phx::ref(off)=_1]||double_[phx::ref(off)=_1];

    keyDesc = keyName[phx::bind(&Geometry_parser::setKeyNameandShape,this,_1)]
            ||'{'>>(keyName[phx::bind(&Geometry_parser::setKeyNameandShape,this,_1)]||keyShape
                   ||keygap[phx::bind(&Geometry_parser::setKeyOffset,this)]
                   ||keyColor)
            >>*((','
            >>(keyName
            ||keyShape
            ||keygap[phx::bind(&Geometry_parser::setKeyOffset,this)]
            ||keyColor))
            ||comments)
            >>'}';

    keys = lit("keys")
            >>'{'
            >>keyDesc[phx::bind(&Geometry_parser::setKeyCordi,this)]
            >>*((*lit(',')>>keyDesc[phx::bind(&Geometry_parser::setKeyCordi,this)]>>*lit(','))||comments)
            >>lit("};");

    geomShape = ((lit("key.shape")>>'='>>name[phx::bind(&Geometry_parser::setGeomShape,this,_1)])||(lit("key.color")>>'='>>name))>>';';
    geomLeft = lit("section.left")>>'='>>double_[phx::ref(geom.sectionLeft)=_1]>>';';
    geomTop = lit("section.top")>>'='>>double_[phx::ref(geom.sectionTop)=_1]>>';';
    geomRowTop = lit("row.top")>>'='>>double_[phx::ref(geom.rowTop)=_1]>>';';
    geomRowLeft = lit("row.left")>>'='>>double_[phx::ref(geom.rowLeft)=_1]>>';';
    geomGap = lit("key.gap")>>'='>>double_[phx::ref(geom.keyGap)=_1]>>';';
    geomVertical = *lit("row.")>>lit("vertical")>>'='>>(lit("True")||lit("true"))>>';';
    geomAtt = geomLeft||geomTop||geomRowTop||geomRowLeft||geomGap;

    top = lit("top")>>'='>>double_>>';';
    left = lit("left")>>'='>>double_>>';';

    row = lit("row")[phx::bind(&Geometry_parser::rowinit,this)]
            >>'{'
            >>*(top[phx::bind(&Geometry_parser::setRowTop,this,_1)]
            ||left[phx::bind(&Geometry_parser::setRowLeft,this,_1)]
            ||localShape[phx::bind(&Geometry_parser::setRowShape,this,_1)]
            ||localColor
            ||comments
            ||geomVertical[phx::bind(&Geometry_parser::setVerticalRow,this)]
            ||keys
            )
            >>lit("};")||ignore||geomVertical[phx::bind(&Geometry_parser::setVerticalSection,this)];

    angle = lit("angle")>>'='>>double_>>';';

    localShape = lit("key.shape")>>'='>>name[_val=_1]>>';';
    localColor = lit("key.color")>>'='>>name>>';';
    localDimension = (lit("height")||lit("width"))>>'='>>double_>>';';
    priority = lit("priority")>>'='>>double_>>';';

    section = lit("section")[phx::bind(&Geometry_parser::sectioninit,this)]
            >>name[phx::bind(&Geometry_parser::sectionName,this,_1)]
            >>'{'
            >>*(top[phx::bind(&Geometry_parser::setSectionTop,this,_1)]
            ||left[phx::bind(&Geometry_parser::setSectionLeft,this,_1)]
            ||angle[phx::bind(&Geometry_parser::setSectionAngle,this,_1)]
            ||row[phx::bind(&Geometry_parser::addRow,this)]
            ||localShape[phx::bind(&Geometry_parser::setSectionShape,this,_1)]
            ||geomAtt
            ||localColor
            ||localDimension
            ||priority
            ||comments)
            >>lit("};")||geomVertical[phx::bind(&Geometry_parser::setVerticalGeometry,this)];

    shapeC = lit("shape")>>'.'>>cornerRadius>>';';

    shape = shapeDef||shapeC;


    in = '{'
          >>+(width
          ||height
          ||comments
          ||ignore
          ||description
          ||(char_-kw-'}'
          ||shape[phx::bind(&Geometry::addShape,&geom)]
          ||section[phx::bind(&Geometry::addSection,&geom)]
          ||geomAtt
          ||geomShape
          ))
          >>'}';

          width = lit("width")>>'='>>double_[phx::bind(&Geometry::setWidth,&geom,_1)]>>";";
          height = lit("height")>>'='>>double_[phx::bind(&Geometry::setHeight,&geom,_1)]>>";";


          info = in;


          start %= *(lit("default"))
                 >>lit("xkb_geometry")
                 >>name[phx::bind(&Geometry_parser::getName,this,_1)]
                 >>info
                 >>';'>>*(comments||char_-lit("xkb_geometry"));


}

template<typename Iterator>
    void Geometry_parser<Iterator>::setCord(){
        geom.setShapeCord(x,y);
    }

template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionShape(std::string n){
    geom.sectionList[geom.getSectionCount()].setShapeName( QString::fromUtf8(n.data(), n.size()));
}

template<typename Iterator>
    void Geometry_parser<Iterator>::getName(std::string n){
        geom.setName(QString::fromUtf8(n.data(), n.size()));
}
template<typename Iterator>
    void Geometry_parser<Iterator>::getDescription(std::string n){
        geom.setDescription( QString::fromUtf8(n.data(), n.size()));
}

template<typename Iterator>
    void Geometry_parser<Iterator>::getShapeName(std::string n){
        geom.setShapeName( QString::fromUtf8(n.data(), n.size()));
}



template<typename Iterator>
    void Geometry_parser<Iterator>::setGeomShape(std::string n){
        geom.setKeyShape(QString::fromUtf8(n.data(), n.size()));
}

template<typename Iterator>
void Geometry_parser<Iterator>::setRowShape(std::string n){
    geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].setShapeName(QString::fromUtf8(n.data(), n.size() ));
}

template<typename Iterator>
    void Geometry_parser<Iterator>::setApprox(){
        geom.setShapeApprox(ax,ay);
}


template<typename Iterator>
    void Geometry_parser<Iterator>::addRow(){
        geom.sectionList[geom.getSectionCount()].addRow();
}


template<typename Iterator>
    void Geometry_parser<Iterator>::sectionName(std::string n){
        geom.sectionList[geom.getSectionCount()].setName(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
    void Geometry_parser<Iterator>::rowinit(){
        geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].setTop(geom.sectionList[geom.getSectionCount()].getTop());
        geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].setLeft(geom.sectionList[geom.getSectionCount()].getLeft());
        geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].setShapeName(geom.sectionList[geom.getSectionCount()].getShapeName());
        cx = geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].getLeft();
        cy = geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].getTop();
        geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].setVertical(geom.sectionList[geom.getSectionCount()].getVertical());
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::sectioninit(){
        geom.sectionList[geom.getSectionCount()].setTop(geom.sectionTop);
        geom.sectionList[geom.getSectionCount()].setLeft(geom.sectionLeft);
        cx = geom.sectionList[geom.getSectionCount()].getLeft();
        cy = geom.sectionList[geom.getSectionCount()].getTop();
        geom.sectionList[geom.getSectionCount()].setShapeName(geom.getKeyShape());
        geom.sectionList[geom.getSectionCount()].setVertical(geom.getVertical());
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setRowTop(double a){
        geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].setTop(a + geom.sectionList[geom.getSectionCount()].getTop());
        cy = geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].getTop();
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setRowLeft(double a){
        geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].setLeft(a + geom.sectionList[geom.getSectionCount()].getLeft());
        cx = geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].getLeft();
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionTop(double a){
        //qDebug()<<"\nsectionCount"<<geom.sectionCount;
        geom.sectionList[geom.getSectionCount()].setTop(a + geom.sectionTop);
        cy = geom.sectionList[geom.getSectionCount()].getTop();
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionLeft(double a){
        //qDebug()<<"\nsectionCount"<<geom.sectionCount;
        geom.sectionList[geom.getSectionCount()].setLeft(a + geom.sectionLeft);
        cx = geom.sectionList[geom.getSectionCount()].getLeft();

    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionAngle(double a){
        //qDebug()<<"\nsectionCount"<<geom.sectionCount;
        geom.sectionList[geom.getSectionCount()].setAngle(a);
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setVerticalRow(){
        geom.sectionList[geom.getSectionCount()].rowList[geom.sectionList[geom.getSectionCount()].getRowCount()].setVertical(1);
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setVerticalSection(){
        geom.sectionList[geom.getSectionCount()].setVertical(1);
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setVerticalGeometry(){
        geom.setVertical(1);
    }

template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyName(std::string n){
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
        //qDebug()<<"\nsC: "<<secn<<"\trC: "<<rown<<"\tkn: "<<keyn;
        geom.sectionList[secn].rowList[rown].keyList[keyn].setKeyName(QString::fromUtf8(n.data(), n.size()));
     }
template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyShape(std::string n){
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
        //qDebug()<<"\nsC: "<<secn<<"\trC: "<<rown<<"\tkn: "<<keyn;
        geom.sectionList[secn].rowList[rown].keyList[keyn].setShapeName(QString::fromUtf8(n.data(), n.size()));
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyNameandShape(std::string n){
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        setKeyName(n);
        setKeyShape(geom.sectionList[secn].rowList[rown].getShapeName().toUtf8().constData());
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyOffset(){
        //qDebug()<<"\nhere\n";
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
        //qDebug()<<"\nsC: "<<secn<<"\trC: "<<rown<<"\tkn: "<<keyn;
        geom.sectionList[secn].rowList[rown].keyList[keyn].setOffset(off);
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyCordi(){
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
        int vertical = geom.sectionList[secn].rowList[rown].getVertical();
        if(vertical == 0)
            cx+=geom.sectionList[secn].rowList[rown].keyList[keyn].getOffset();
        else
            cy+=geom.sectionList[secn].rowList[rown].keyList[keyn].getOffset();
        geom.sectionList[secn].rowList[rown].keyList[keyn].setKeyPosition(cx,cy);
        QString s = geom.sectionList[secn].rowList[rown].keyList[keyn].getShapeName();
        if (s=="")
            s = geom.getKeyShape();
        GShape t = geom.findShape(s);
        int a = t.size(vertical);
        if(vertical == 0)
            cx+=a+geom.keyGap;
        else
            cy+=a+geom.keyGap;
        geom.sectionList[secn].rowList[rown].addKey();
    }

    QString mapModelToGeometry(QString model){
        QStringList pcmodels;
        QStringList msmodels;
        QStringList nokiamodels;
        QStringList pcgeometries;
        QStringList macbooks;
        QStringList applealu;
        QStringList macs;
        pcmodels << "pc101" << "pc102" << "pc104" << "pc105";
        msmodels << "microsoft" << "microsoft4000" << "microsoft7000" << "microsoftpro" << "microsoftprousb" << "microsoftprose";
        nokiamodels << "nokiasu8w" << "nokiarx44" << "nokiarx51";
        pcgeometries << "latitude";
        macbooks << "macbook78" << "macbook79";
        applealu << "applealu_ansi" << "applealu_iso" << "applealu_jis";
        macs << "macintosh" << "macintosh_old" << "ibook" << "powerbook" << "macbook78" << "macbook79";

        if (model == "thinkpad     us"){
            return("thinkpad|us");
        }
        if (model == "microsoftelite"){
            return("microsoft|elite");
        }
        if (msmodels.contains(model)){
            return("microsoft|natural");
        }
        if (model == "dell101"){
            return("dell|dell101");
        }
        if (model == "dellm65"){
            return("dell|dellm65");
        }
        if (model == "latitude"){
            return("dell|latitude");
        }
        if (model == "flexpro"){
            return("keytronic|FlexPro");
        }
        if(model == "hp6000"|| model == "hpmini110"){
            return("hp|mini110");
        }
        if(model == "hpdv5"){
            return("hp|dv5");
        }
        if(model == "omnikey101"){
            return("northgate|omnikey101");
        }
        if(model == "sanwaskbkg3"){
            return("sanwa|sanwaskbkg3");
        }
        if(pcmodels.contains(model)||pcgeometries.contains(model)){
            return("pc|"+model);
        }
        if(model == "everex"){
            return("everex|STEPnote");
        }
        if(model.contains("thinkpad")){
            return("thinkpad|60");
        }
        if(model == "winbook"){
            return("winbook|XP5");
        }
        if(model == "pc98"){
            return("nec|pc98");
        }
        if(model == "hhk"){
            return("hhk|basic");
        }
        if(model == "kinesis"){
            return("kinesis|model100");
        }
        if(nokiamodels.contains(model)){
            return("nokia|"+model);
        }
        if(macs.contains(model)||macbooks.contains(model)||applealu.contains(model)){
            return("macintosh|"+model);
        }

        return("pc|pc104");
    }



    Geometry parseGeometry(QString model){
        using boost::spirit::ascii::space;
        typedef std::string::const_iterator iterator_type;
        typedef grammar::Geometry_parser<iterator_type> Geometry_parser;
        Geometry_parser g;

        QString geometry = mapModelToGeometry(model);
        qDebug()<<geometry;
        QStringList mapedModel = geometry.split('|');
        QString geometryfile = mapedModel.at(0);
        QString geometryName = mapedModel.at(1);
        QString xkbParentDir = findGeometryBaseDir();
        geometryfile.prepend(xkbParentDir);
        QFile gfile(geometryfile);
         if (!gfile.open(QIODevice::ReadOnly | QIODevice::Text)){
             qDebug()<<"unable to open the file";
             return g.geom;
        }
        QString gcontent = gfile.readAll();
        gfile.close();
        QStringList gcontentList = gcontent.split("xkb_geometry");
        int i = 1;
        while(g.geom.getName()!=geometryName){
            g.geom = Geometry();
            QString input = gcontentList.at(i);
            input.prepend("xkb_geometry");
            //qDebug()<<input;
            std::string xyz = input.toUtf8().constData();

            std::string::const_iterator iter = xyz.begin();
            std::string::const_iterator end = xyz.end();

            bool r = phrase_parse(iter, end, g, space);
            if (r && iter == end){
                std::cout << "-------------------------\n";
                std::cout << "Parsing succeeded\n";
                std::cout << "\n-------------------------\n";
            }
            else{
                std::cout << "-------------------------\n";
                std::cout << "Parsing failed\n";
                std::cout << "-------------------------\n";
            }
            i++;

        }
        g.geom.display();
        return g.geom;

    }



    QString findGeometryBaseDir()
    {
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

        return QString("%1/xkb/geometry/").arg(xkbParentDir);
    }

}
