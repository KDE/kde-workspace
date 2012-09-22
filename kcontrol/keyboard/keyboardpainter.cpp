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

#include "keyboardpainter.h"
#include "ui_keyboardpainter.h"
#include<QHBoxLayout>
#include<QVBoxLayout>

KeyboardPainter::KeyboardPainter() :
    kbframe(new KbPreviewFrame(this)),
    exitButton(new QPushButton(tr("CLOSE"),this)),
    ui(new Ui::keyboardpainter)
{
    ui->setupUi(this);
    kbframe->setFixedSize( 1030, 490 );
    exitButton->setFixedSize(120,30);
    connect(exitButton,SIGNAL(clicked()),this,SLOT(close()));
    QVBoxLayout*const vLayout = new QVBoxLayout( this );
    vLayout->addWidget(kbframe);
    vLayout->addWidget(exitButton);
    setWindowTitle(kbframe->kblayout.Layoutname);
}

KeyboardPainter::~KeyboardPainter()
{
    delete ui;
    delete kbframe;
    delete exitButton;
}
