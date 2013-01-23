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
 

#ifndef KEYBOARDPAINTER_H
#define KEYBOARDPAINTER_H

#include <QtGui/QDialog>
#include <QtGui/QPushButton>

#include"kbpreviewframe.h"

namespace Ui {
class keyboardpainter;
}

class KeyboardPainter : public QDialog
{
    Q_OBJECT
    
public:
    explicit KeyboardPainter();
    ~KeyboardPainter();
    void generateKeyboardLayout(QString country, QString layoutvariant, QString model);
    
private:
	bool event(QEvent *event);
    Ui::keyboardpainter *ui;
    KbPreviewFrame *kbframe;
    QPushButton *exitButton;
};

#endif // KEYBOARDPAINTER_H
