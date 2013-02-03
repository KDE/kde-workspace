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
#include "kbgeometry.h"
#include <QPainter>
#include <QtGui/QFrame>

struct ToolTipText{
	QPoint ttPoint;
	QString ttString;
};

class KbPreviewFrame : public QFrame
{
    Q_OBJECT
private:
	int scalex,scaley;
    void setScaleFactor(int width,int hieght);
    void writeKeySym(QPainter &painter,KeyGm key);
    void plotSym(QPainter &painter,KeyGm key,Keys sym);
    void plotSym(QPainter &painter,KeyGm key,QString sym);
    void getkeyboardlayout(QString country, QString layoutvariant);
	KeySymHelper symbol;
	Aliases alias;
public:
	QList<ToolTipText> toolTipList;
    explicit KbPreviewFrame(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);
    bool event(QEvent *event);
    int keyAt(QPoint pos);
    KbGeometry geometry;
    KeyboardLayout kblayout;
    void generateKeyboardLayout(const QString &country, const QString &layoutvariant, const QString &model);

    
};

#endif // KBPREVIEWFRAME_H 
