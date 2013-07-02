#include "geometry_parser.h"
#include "geometry_components.h"

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QFileDialog>
#include <QFile>

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

    ignore =lit("outline")>>*(char_-lit("};"))>>lit("};")
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
            >>*(lit("approx")>>'='>>setap[phx::bind(&Geometry_parser::setApprox,this)]>>','||cornerRadius>>',')
            >>seta
            >>*((','>>(set||lit("approx")>>'='>>setap[phx::bind(&Geometry_parser::setApprox,this)]||cornerRadius)||comments))
            >>lit("};")
            ;

    keyName = '<'>>+(char_-'>')>>'>';

    keyShape = lit("shape")>>'='>>name[phx::bind(&Geometry_parser::setKeyShape,this,_1)]
            ||name[phx::bind(&Geometry_parser::setKeyShape,this,_1)];

    keyColor = lit("color")>>'='>>name;

    keygap = lit("gap")>>'='>>double_[phx::ref(off)=_1]||double_[phx::ref(off)=_1];

    keyDesc = keyName[phx::bind(&Geometry_parser::setKeyNameandShape,this,_1)]
            ||'{'>>keyName[phx::bind(&Geometry_parser::setKeyNameandShape,this,_1)]
            >>*((','
            >>(keyShape
            ||keygap[phx::bind(&Geometry_parser::setKeyOffset,this)]
            ||keyColor))
            ||comments)
            >>'}';

    keys = lit("keys")
            >>'{'
            >>keyDesc[phx::bind(&Geometry_parser::setKeyCordi,this)]
            >>*((*lit(',')>>keyDesc[phx::bind(&Geometry_parser::setKeyCordi,this)]>>*lit(','))||comments)
            >>lit("};");

    geomShape = lit("key.shape")>>'='>>name[phx::bind(&Geometry_parser::setGeomShape,this,_1)]>>';';
    geomLeft = lit("section.left")>>'='>>double_[phx::ref(geom.sectionLeft)=_1]>>';';
    geomTop = lit("section.top")>>'='>>double_[phx::ref(geom.sectionTop)=_1]>>';';
    geomRowTop = lit("row.top")>>'='>>double_[phx::ref(geom.rowTop)=_1]>>';';
    geomRowLeft = lit("row.left")>>'='>>double_[phx::ref(geom.rowLeft)=_1]>>';';
    geomGap = lit("key.gap")>>'='>>double_[phx::ref(geom.keyGap)=_1]>>';';

    geomAtt = geomLeft||geomTop||geomRowTop||geomRowLeft||geomGap;

    top = lit("top")>>'='>>double_>>';';
    left = lit("left")>>'='>>double_>>';';

    row = lit("row")[phx::bind(&Geometry_parser::rowinit,this)]
            >>'{'
            >>*(top[phx::bind(&Geometry_parser::setRowTop,this,_1)]
            ||left[phx::bind(&Geometry_parser::setRowLeft,this,_1)]
            ||localShape[phx::bind(&Geometry_parser::setRowShape,this,_1)]
            ||localColor
            ||comments)
            >>keys
            >>lit("};");

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
            >>lit("};");

    shapeC = lit("shape")>>'.'>>cornerRadius>>';';

    shape = shapeDef||shapeC;


    in = '{'
          >>+(width
          ||height
          ||comments
          ||ignore
          ||description
          ||(char_-kw-'}'
          ||shape[phx::bind(&Geometry::getShape,&geom)]
          ||section[phx::bind(&Geometry::addSection,&geom)]
          ||geomAtt
          ||geomShape
          ))
          >>'}';

          width = lit("width")>>'='>>double_[phx::bind(&Geometry::getWidth,&geom,_1)]>>";";
          height = lit("height")>>'='>>double_[phx::bind(&Geometry::getHeight,&geom,_1)]>>";";


          info = in;


          start %= *(lit("default"))
                 >>lit("xkb_geometry")
                 >>name[phx::bind(&Geometry_parser::getName,this,_1)]
                 >>info
                 >>';';


}

template<typename Iterator>
    void Geometry_parser<Iterator>::setCord(){
        geom.getShapeCord(x,y);
    }

template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionShape(std::string n){
        geom.sectionList[geom.sectionCount].shapeName = QString::fromUtf8(n.data(), n.size());
}

template<typename Iterator>
    void Geometry_parser<Iterator>::getName(std::string n){
        geom.getName(QString::fromUtf8(n.data(), n.size()));
}
template<typename Iterator>
    void Geometry_parser<Iterator>::getDescription(std::string n){
        geom.getDescription( QString::fromUtf8(n.data(), n.size()));
}

template<typename Iterator>
    void Geometry_parser<Iterator>::getShapeName(std::string n){
        geom.getShapeName( QString::fromUtf8(n.data(), n.size()));
}



template<typename Iterator>
    void Geometry_parser<Iterator>::setGeomShape(std::string n){
        geom.keyShape = QString::fromUtf8(n.data(), n.size());
}

template<typename Iterator>
void Geometry_parser<Iterator>::setRowShape(std::string n){
    geom.sectionList[geom.sectionCount].rowList[geom.sectionList[geom.sectionCount].rowCount].shapeName =  QString::fromUtf8(n.data(), n.size() );
}

template<typename Iterator>
    void Geometry_parser<Iterator>::setApprox(){
        geom.getShapeApprox(ax,ay);
}


template<typename Iterator>
    void Geometry_parser<Iterator>::addRow(){
        geom.sectionList[geom.sectionCount].addRow();
}


template<typename Iterator>
    void Geometry_parser<Iterator>::sectionName(std::string n){
        geom.sectionList[geom.sectionCount].getName(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
    void Geometry_parser<Iterator>::rowinit(){
        geom.sectionList[geom.sectionCount].rowList[geom.sectionList[geom.sectionCount].rowCount].top = geom.sectionList[geom.sectionCount].top;
        geom.sectionList[geom.sectionCount].rowList[geom.sectionList[geom.sectionCount].rowCount].left =  geom.sectionList[geom.sectionCount].left;
        geom.sectionList[geom.sectionCount].rowList[geom.sectionList[geom.sectionCount].rowCount].shapeName =  geom.sectionList[geom.sectionCount].shapeName;
        cx = geom.sectionList[geom.sectionCount].rowList[geom.sectionList[geom.sectionCount].rowCount].left;
        cy = geom.sectionList[geom.sectionCount].rowList[geom.sectionList[geom.sectionCount].rowCount].top;
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::sectioninit(){
        geom.sectionList[geom.sectionCount].top = geom.sectionTop;
        geom.sectionList[geom.sectionCount].left = geom.sectionLeft;
        cx = geom.sectionList[geom.sectionCount].left;
        cy = geom.sectionList[geom.sectionCount].top;
        geom.sectionList[geom.sectionCount].shapeName = geom.keyShape;
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setRowTop(double a){
        geom.sectionList[geom.sectionCount].rowList[geom.sectionList[geom.sectionCount].rowCount].top = a + geom.sectionList[geom.sectionCount].top;
        cy = geom.sectionList[geom.sectionCount].rowList[geom.sectionList[geom.sectionCount].rowCount].top;
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setRowLeft(double a){
        geom.sectionList[geom.sectionCount].rowList[geom.sectionList[geom.sectionCount].rowCount].left = a + geom.sectionList[geom.sectionCount].left;
        cx = geom.sectionList[geom.sectionCount].rowList[geom.sectionList[geom.sectionCount].rowCount].left;
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionTop(double a){
        qDebug()<<"\nsectionCount"<<geom.sectionCount;
        geom.sectionList[geom.sectionCount].top = a + geom.sectionTop;
        cy = geom.sectionList[geom.sectionCount].top;
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionLeft(double a){
        qDebug()<<"\nsectionCount"<<geom.sectionCount;
        geom.sectionList[geom.sectionCount].left = a + geom.sectionLeft;
        cx = geom.sectionList[geom.sectionCount].left;

    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionAngle(double a){
        qDebug()<<"\nsectionCount"<<geom.sectionCount;
        geom.sectionList[geom.sectionCount].angle = a;
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyName(std::string n){
        int secn = geom.sectionCount;
        int rown = geom.sectionList[secn].rowCount;
        int keyn = geom.sectionList[secn].rowList[rown].keyCount;
        qDebug()<<"\nsC: "<<secn<<"\trC: "<<rown<<"\tkn: "<<keyn;
        geom.sectionList[secn].rowList[rown].keyList[keyn].name = QString::fromUtf8(n.data(), n.size());
     }
template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyShape(std::string n){
        int secn = geom.sectionCount;
        int rown = geom.sectionList[secn].rowCount;
        int keyn = geom.sectionList[secn].rowList[rown].keyCount;
        geom.sectionList[secn].rowList[rown].keyList[keyn].shapeName = QString::fromUtf8(n.data(), n.size());
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyNameandShape(std::string n){
        int secn = geom.sectionCount;
        int rown = geom.sectionList[secn].rowCount;
        setKeyName(n);
        setKeyShape(geom.sectionList[secn].rowList[rown].shapeName.toUtf8().constData());
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyOffset(){
        qDebug()<<"\nhere\n";
        int secn = geom.sectionCount;
        int rown = geom.sectionList[secn].rowCount;
        int keyn = geom.sectionList[secn].rowList[rown].keyCount;
        geom.sectionList[secn].rowList[rown].keyList[keyn].getKey(off);
    }
template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyCordi(){
        int secn = geom.sectionCount;
        int rown = geom.sectionList[secn].rowCount;
        int keyn = geom.sectionList[secn].rowList[rown].keyCount;
        cx+=geom.sectionList[secn].rowList[rown].keyList[keyn].offset;
        geom.sectionList[secn].rowList[rown].keyList[keyn].setKeyPosition(cx,cy);
        QString s = geom.sectionList[secn].rowList[rown].keyList[keyn].shapeName;
        if (s=="")
                s = geom.keyShape;
        GShape t = geom.findShape(s);
        int a = t.size();
        cx+=a+geom.keyGap;
        geom.sectionList[secn].rowList[rown].addKey();
    }

    Geometry parseGeometry(){
        using boost::spirit::ascii::space;
        typedef std::string::const_iterator iterator_type;
        typedef grammar::Geometry_parser<iterator_type> Geometry_parser;
        Geometry_parser g;

        QString geometryfile = QFileDialog::getOpenFileName();

        QFile gfile(geometryfile);
         if (!gfile.open(QIODevice::ReadOnly | QIODevice::Text)){
             qDebug()<<"unable to open the file";
             return g.geom;
        }
        QString gcontent = gfile.readAll();
        //qDebug()<<gcontent;
        std::string xyz = gcontent.toUtf8().constData();

        gfile.close();

        std::string::const_iterator iter = xyz.begin();
        std::string::const_iterator end = xyz.end();

        bool r = phrase_parse(iter, end, g, space);
        if (r && iter == end){
              std::cout << "-------------------------\n";
               std::cout << "Parsing succeeded\n";
               g.geom.display();
               std::cout << "\n-------------------------\n";
               return g.geom;
       }
       else{
               std::cout << "-------------------------\n";
               std::cout << "Parsing failed\n";
               std::cout << "-------------------------\n";
               g.geom.display();
               return g.geom;
       }

    }


}


