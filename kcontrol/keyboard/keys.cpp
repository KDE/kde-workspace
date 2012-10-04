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


#include "keys.h"
#include<QApplication>
#include<QtCore/QStringList>
Keys::Keys()
{
}
void Keys::setKey(QString a){
    int i=a.indexOf("<");
    i++;
    keyname=a.mid(i,4);
    keyname.remove(" ");
    i=a.indexOf("[");
    i++;
    QString str=a.mid(i);
    i=str.indexOf("]");
    QString st=str.left(i);
    st=st.remove(" ");
    //QStringList klst;
    klst=st.split(",");
    for(int k=0;k<klst.size();k++){
        QString du=klst.at(k);
        du.remove(" ");
        du.remove("\t");
        du.remove("\n");
        klst[k]=du;
    }
}
