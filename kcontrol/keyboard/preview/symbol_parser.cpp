#include "symbol_parser.h"
#include "keyboardlayout_new.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <QFileDialog>
#include <QFile>


#include <fixx11h.h>
#include <config-workspace.h>

namespace grammar{

symbol_keywords :: symbol_keywords(){
    add
            ("key",2)
            ("include",1)
            ("//",3)
            ("*/",4)
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
    type = lit("type")>>'['>>group>>lit(']')>>lit('=')>>name;
    symbol = +(char_-','-']');
    symbols = *(lit("symbols")
        >>'['>>group>>lit(']')>>lit('='))
        >>'['>>symbol[phx::bind(&Symbol_parser::getSymbol,this,_1)]
        >>*(','>>symbol[phx::bind(&Symbol_parser::getSymbol,this,_1)])>>']';

    keyName = '<'>>*(char_-'>')>>'>';

    key = lit("key")>>keyName[phx::bind(&Symbol_parser::addKeyName,this,_1)]
        >>'{'>>*(type>>',')
        >>symbols
        >>*(','>>type)
        >>lit("};");

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
        >>lit("};");
}

template<typename Iterator>
void Symbol_parser<Iterator>::getSymbol(std::string n){
    int index = layout.keyList[keyIndex].getSymbolCount();
    layout.keyList[keyIndex].addSymbol(QString::fromUtf8(n.data(), n.size()), index);
    qDebug()<<"adding symbol: "<<QString::fromUtf8(n.data(), n.size());
    qDebug()<<"added symbol: "<<layout.keyList[keyIndex].getSymbol(index)<<" in "<<keyIndex<<" at "<<index;
}
template<typename Iterator>
void Symbol_parser<Iterator>::addKeyName(std::string n){
    keyIndex = layout.findKey(QString::fromUtf8(n.data(), n.size()));
    qDebug()<<layout.getKeyCount();
    if (keyIndex == -1){
        layout.keyList[layout.getKeyCount()].keyName = QString::fromUtf8(n.data(), n.size());
        keyIndex = layout.getKeyCount();
        newKey = 1;
    }
    else
        qDebug()<<"key at"<<keyIndex;
}

template<typename Iterator>
void Symbol_parser<Iterator>::addKey(){
    if(newKey == 1){
        layout.addKey();
        newKey = 0;
        qDebug()<<"new key";
    }
}

template<typename Iterator>
void Symbol_parser<Iterator>::getInclude(std::string n){
    layout.addInclude(QString::fromUtf8(n.data(), n.size()));
}

template<typename Iterator>
void Symbol_parser<Iterator>::setName(std::string n){
    layout.setName(QString::fromUtf8(n.data(), n.size()));
}

KbLayout parseSymbols(){
    using boost::spirit::ascii::space;
    typedef std::string::const_iterator iterator_type;
    typedef grammar::Symbol_parser<iterator_type> Symbol_parser;
    Symbol_parser s;

    QFile sfile("/home/amourphious/Desktop/Boost/input");
     if (!sfile.open(QIODevice::ReadOnly | QIODevice::Text)){
         qDebug()<<"unable to open the file";
         return s.layout;
    }
    QString scontent = sfile.readAll();
    sfile.close();
    std::string xyz = scontent.toUtf8().constData();

    std::string::const_iterator iter = xyz.begin();
    std::string::const_iterator end = xyz.end();

    bool r = phrase_parse(iter, end, s, space);
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

    s.layout.display();
    return s.layout;
}

}
