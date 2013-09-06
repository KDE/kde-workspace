/*
 *  Copyright (C) 2012 Shivam Makkar (amourphious1992@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KEYBOARDLAYOUT_NEW_H
#define KEYBOARDLAYOUT_NEW_H

#include "keyaliases.h"
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>


class KbKey{
    QList<QString> symbols;
    int symbolCount;
public :
    QString keyName;

    KbKey();

    void setKeyName(QString n);
    void addSymbol(QString n, int i);
    QString getSymbol(int i);

    int getSymbolCount(){
        return symbolCount;
    }

    void display();
};


class KbLayout{

    QList<QString> include;
    QString name;
    int keyCount, includeCount, level;

public:

    QList <KbKey> keyList;
    QString country;

    KbLayout();

    void setName(QString n);
    void addInclude(QString n);
    void addKey();
    QString getInclude(int i);
    int findKey(QString n);

    void setLevel(int lvl){
        level = lvl;
    }

    int getLevel(){
        return level;
    }

    int getKeyCount(){
        return keyCount;
    }

    int getIncludeCount(){
        return includeCount;
    }

    QString getLayoutName() const {
        return name;
    }

    void display();
};

#endif //KEYBOARDLAYOUT_NEW_H
