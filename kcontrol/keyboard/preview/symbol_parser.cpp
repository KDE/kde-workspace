#include "symbol_parser.h"
#include "keyboardlayout.h"
#include "keyaliases.h"
#include "x11_helper.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <QFileDialog>
#include <QFile>



namespace grammar{

symbol_keywords :: symbol_keywords(){
    add
            ("key", 2)
            ("include", 1)
            ("//", 3)
            ("*/", 4)
        ;
}

levels ::levels(){
    add
            ("ONE", 1)
            ("TWO", 2)
            ("THREE", 3)
            ("FOUR", 4)
            ("SIX", 6)
            ("EIGHT", 8)
        ;
}

template<typename Iterator>
Symbol_parser<Iterator>::Symbol_parser():Symbol_parser::base_type(start){
    using qi::lexeme;
    using qi::char_;
    using qi::lit;
    using qi::_1;
    using qi::_val;
    using qi::int_;
    using qi::double_;
    using qi::eol;

    newKey = 0;


    name %= '"'>>+(char_-'"')>>'"';


    group = lit("Group")>>int_;

    comments =lexeme[ lit("//")>>*(char_-eol||skw-eol)>>eol||lit("/*")>>*(char_-lit("*/")||skw-lit("*/"))>>lit("*/") ];

    include = lit("include")>>name[phx::bind(&Symbol_parser::getInclude,this,_1)];

    type = lit("type")>>'['>>group>>lit(']')>>lit('=')>>lit("\"")>>*(char_ - lvl)>>*lvl[phx::bind(&Symbol_parser::setLevel, this, _1)]>>*(char_ - lvl - '"')>>lit("\"");

    symbol = +(char_-','-']');

    symbols = *(lit("symbols")
        >>'['>>group>>lit(']')>>lit('='))
        >>'['>>symbol[phx::bind(&Symbol_parser::getSymbol,this,_1)]
        >>*(','>>symbol[phx::bind(&Symbol_parser::getSymbol,this,_1)])>>']';

    keyName = '<'>>*(char_-'>')>>'>';

    key = (lit("key")>>keyName[phx::bind(&Symbol_parser::addKeyName,this,_1)]
        >>'{'>>*(type>>',')
        >>symbols
        >>*(','>>type)
       >>lit("};"))
       ||lit("key")>>lit(".")>>type>>lit(";");

    ee = *(char_ - skw - '{')>>'{'>>*(char_-'}'-';')>>lit("};");


    start = *(char_ - lit("xkb_symbols")||comments)
        >>lit("xkb_symbols")
        >>name[phx::bind(&Symbol_parser::setName,this,_1)]
        >>'{'
        >>*(key[phx::bind(&Symbol_parser::addKey,this)]
        ||include
        ||ee
        ||char_-'}'-skw
        ||comments)
        >>lit("};")
        >>*(comments||char_);
}


template<typename Iterator>
void Symbol_parser<Iterator>::getSymbol(std::string n){
    int index = layout.keyList[keyIndex].getSymbolCount();
    layout.keyList[keyIndex].addSymbol(QString::fromUtf8(n.data(), n.size()), index);
    //qDebug()<<"adding symbol: "<<QString::fromUtf8(n.data(), n.size());
    //qDebug()<<"added symbol: "<<layout.keyList[keyIndex].getSymbol(index)<<" in "<<keyIndex<<" at "<<index;
}


template<typename Iterator>
void Symbol_parser<Iterator>::addKeyName(std::string n){
    QString kname = QString::fromUtf8(n.data(), n.size());
    if(kname.startsWith("Lat"))
        kname = alias.getAlias(layout.country, kname);
    keyIndex = layout.findKey(kname);
    //qDebug()<<layout.getKeyCount();
    if (keyIndex == -1){
        layout.keyList[layout.getKeyCount()].keyName = kname;
        keyIndex = layout.getKeyCount();
        newKey = 1;
    }
       // qDebug()<<"key at"<<keyIndex;
}


template<typename Iterator>
void Symbol_parser<Iterator>::addKey(){
    if(newKey == 1){
        layout.addKey();
        newKey = 0;
        //qDebug()<<"new key";
    }
}


template<typename Iterator>
void Symbol_parser<Iterator>::getInclude(std::string n){
    layout.addInclude(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
void Symbol_parser<Iterator>::setName(std::string n){
    layout.setName(QString::fromUtf8(n.data(), n.size()));
    //qDebug() << layout.getLayoutName();
}

template<typename Iterator>
void Symbol_parser<Iterator> :: setLevel(int lvl){
    if(lvl > layout.getLevel()){
        layout.setLevel(lvl);
        qDebug()<< lvl;
    }
}


QString findSymbolBaseDir()
{
    QString xkbDir = X11Helper::findXkbDir();
    return QString("%1/symbols/").arg(xkbDir);
}



QString findLayout(const QString& layout, const QString& layoutVariant){

    QString symbolBaseDir = findSymbolBaseDir();
    QString symbolFile = symbolBaseDir.append(layout);

    QFile sfile(symbolFile);
    if (!sfile.open(QIODevice::ReadOnly | QIODevice::Text)){
         //qDebug()<<"unable to open the file";
         return QString();
    }

    QString scontent = sfile.readAll();
    sfile.close();
    QStringList scontentList = scontent.split("xkb_symbols");

    QString variant;
    QString input;

    if(layoutVariant.isEmpty()){
        input = scontentList.at(1);
        input.prepend("xkb_symbols");
    }

    else{
        int i = 1;

        while (layoutVariant != variant && i < scontentList.size()) {
            input = scontentList.at(i);

            QString h = scontentList.at(i);

            int k = h.indexOf("\"");
            h = h.mid(k);
            k = h.indexOf("{");
            h = h.left(k);
            h = h.remove(" ");
            variant = h.remove("\"");

            input.prepend("xkb_symbols");
            i++;
        }
    }

    return input;
}

KbLayout parseSymbols(const QString& layout, const QString& layoutVariant){

    using boost::spirit::iso8859_1::space;
    typedef std::string::const_iterator iterator_type;
    typedef grammar::Symbol_parser<iterator_type> Symbol_parser;

    Symbol_parser s;

    s.layout.country = layout;
    QString input = findLayout(layout, layoutVariant);

    std::string xyz = input.toUtf8().constData();

    std::string::const_iterator iter = xyz.begin();
    std::string::const_iterator end = xyz.end();

    bool r = phrase_parse(iter, end, s, space);

    /*if (r && iter == end){
        std::cout << "-------------------------\n";
        std::cout << "Parsing succeeded\n";
        std::cout << "\n-------------------------\n";
    }
    else{
        std::cout << "-------------------------\n";
        std::cout << "Parsing failed\n";
        std::cout << "-------------------------\n";
        qDebug()<<input;
    }*/


    for(int j = 0; j < s.layout.getIncludeCount(); j++){
        QString include = s.layout.getInclude(j);
        QStringList includeFile = include.split("(");
        if(includeFile.size() == 2){
            QString l = includeFile.at(0);
            QString lv = includeFile.at(1);
            lv.remove(")");
            input = findLayout(l,lv);

        }

        else{
            QString a = QString();
            input = findLayout(includeFile.at(0),a);
        }

        xyz = input.toUtf8().constData();

        std::string::const_iterator iter = xyz.begin();
        std::string::const_iterator end = xyz.end();

        bool r = phrase_parse(iter, end, s, space);

        /*if (r && iter == end){
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
            std::cout << "\n-------------------------\n";
        }
        else{
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
            qDebug()<<input;
        }*/
    }

    s.layout.display();

    return s.layout;
}

}
