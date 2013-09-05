#ifndef SYMBOL_PARSER_H
#define SYMBOL_PARSER_H

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/home/support/char_encoding/iso8859_1.hpp>

#include <iostream>
#include <QtCore/QDebug>

#include "keyboardlayout.h"
#include "keyaliases.h"

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;
namespace iso = boost::spirit::iso8859_1;




namespace grammar {

    struct symbol_keywords : qi::symbols<char, int>{
        symbol_keywords();
    };

    struct levels : qi::symbols<char, int>{
        levels();
    };

    template<typename Iterator>
    struct Symbol_parser : qi::grammar<Iterator, iso::space_type>{

        Symbol_parser();
        qi::rule<Iterator,  iso::space_type>start;
        qi::rule<Iterator, std::string(), iso::space_type>name;
        qi::rule<Iterator, std::string(), iso::space_type>keyName;
        qi::rule<Iterator, std::string(), iso::space_type>symbols;
        qi::rule<Iterator, std::string(), iso::space_type>key;
        qi::rule<Iterator, std::string(), iso::space_type>type;
        qi::rule<Iterator, std::string(), iso::space_type>group;
        qi::rule<Iterator, std::string(), iso::space_type>symbol;
        qi::rule<Iterator, std::string(), iso::space_type>comments;
        qi::rule<Iterator, std::string(), iso::space_type>ee;
        qi::rule<Iterator, std::string(), iso::space_type>include;

        KbLayout layout;
        int keyIndex, newKey;
        symbol_keywords skw;
        levels lvl;
        Aliases alias;

        void getSymbol(std::string n);
        void addKeyName(std::string n);
        void getInclude(std::string n);
        void addKey();
        void setName(std::string n);
        void setLevel(int lvl);

    };

    KbLayout parseSymbols(const QString& layout, const QString& layoutVariant);
    QString findSymbolBaseDir();
}

#endif //SYMBOL_PARSER_H
