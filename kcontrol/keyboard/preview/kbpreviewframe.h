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
#ifndef KBPREVIEWFRAME_H
#define KBPREVIEWFRAME_H

#include "keyboardlayout.h"
#include "keysymhelper.h"
#include "keyaliases.h"

#include <QtGui/QPainter>
#include <QtGui/QFrame>

#ifdef NEW_GEOMETRY
#include "geometry_parser.h"
#include "geometry_components.h"
#endif
class KbPreviewFrame : public QFrame
{
    Q_OBJECT
    
private:
    KeySymHelper symbol;
    Aliases alias;
    KeyboardLayout keyboardLayout;
#ifdef NEW_GEOMETRY
    void drawKeySymbols(QPainter &painter,QPoint temp[],GShape s,QString name);
	float scaleFactor;
    void drawShape(QPainter &painter, GShape s, int x, int y, int i, QString name);
#else
    void paintTLDE(QPainter &painter, int &x, int &y);
    void paintAERow(QPainter &painter, int &x, int &y);
    void paintADRow(QPainter &painter, int &x, int &y);
    void paintACRow(QPainter &painter, int &x, int &y);
    void paintABRow(QPainter &painter, int &x, int &y);
    void paintBottomRow(QPainter &painter, int &x, int &y);
    void paintFnKeys(QPainter &painter, int &x, int &y);
#endif
	
public:
#ifdef NEW_GEOMETRY
    Geometry geometry;
#endif
    explicit KbPreviewFrame(QWidget *parent = 0);
    void paintEvent(QPaintEvent * event);
#ifdef NEW_GEOMETRY
    void generateKeyboardLayout(const QString &country, const QString &layoutVariant, QString model);
#else
    void generateKeyboardLayout(const QString &country, const QString &layoutVariant);
#endif
    QString getLayoutName() const {
    	return keyboardLayout.getLayoutName();
    }
};

#endif // KBPREVIEWFRAME_H 
