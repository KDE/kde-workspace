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

#include <iostream>
#include <QtCore/QDebug>

#include "keyboardlayout_new.h"
#include "keyaliases.h"

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;

namespace grammar {

    struct symbol_keywords : qi::symbols<char, int>{
        symbol_keywords();
    };

    template<typename Iterator>
    struct Symbol_parser : qi::grammar<Iterator, ascii::space_type>{

        Symbol_parser();
        qi::rule<Iterator, std::string(), ascii::space_type>name;
        qi::rule<Iterator, std::string(), ascii::space_type>keyName;
        qi::rule<Iterator, std::string(), ascii::space_type>symbols;
        qi::rule<Iterator, std::string(), ascii::space_type>key;
        qi::rule<Iterator, std::string(), ascii::space_type>type;
        qi::rule<Iterator, std::string(), ascii::space_type>group;
        qi::rule<Iterator, std::string(), ascii::space_type>symbol;
        qi::rule<Iterator,  ascii::space_type>start;
        qi::rule<Iterator, std::string(), ascii::space_type>comments;
        qi::rule<Iterator, std::string(), ascii::space_type>ee;
        qi::rule<Iterator, std::string(), ascii::space_type>include;

        KbLayout layout;
        int keyIndex, newKey;
        symbol_keywords skw;
        Aliases alias;

        void getSymbol(std::string n);
        void addKeyName(std::string n);
        void getInclude(std::string n);
        void addKey();
        void setName(std::string n);

    };

    KbLayout parseSymbols(const QString& layout, const QString& layoutVariant);
    QString findSymbolBaseDir();
}

#endif //SYMBOL_PARSER_H
