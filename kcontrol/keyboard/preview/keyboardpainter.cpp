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
#include "geometry_components.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>


#include <KLocale>


KeyboardPainter::KeyboardPainter():
    kbDialog(new QDialog(this)),
    kbframe(new KbPreviewFrame(this)),
    exitButton(new QPushButton(i18n("Close"), this)),
    levelBox(new QComboBox(this))
{
    kbDialog->setFixedSize( 1100, 490 );
    kbframe->setFixedSize( 1100, 490 );
    exitButton->setFixedSize( 120, 30 );
    levelBox->setFixedSize( 360, 30 );

    QVBoxLayout* vLayout = new QVBoxLayout( this );
    QHBoxLayout* hLayout = new QHBoxLayout( this );

    hLayout->addWidget(exitButton, 0, Qt::AlignLeft);
    hLayout->addWidget(levelBox, 0, Qt::AlignRight);
    hLayout->addSpacing(30);

    vLayout->addWidget(kbframe);
    vLayout->addLayout(hLayout);

    connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(levelBox, SIGNAL(activated(int)), this, SLOT(levelChanged(int)));

    setWindowTitle(kbframe->getLayoutName());
}


void KeyboardPainter::generateKeyboardLayout(const QString& layout, const QString& variant, const QString& model)
{
    kbframe->generateKeyboardLayout(layout, variant, model);
    kbDialog->setFixedSize(getWidth(), getHeight());
    kbframe->setFixedSize(getWidth(), getHeight());

    setWindowTitle(layout + ":" + variant);

    int level = kbframe->getLevel();

    if(level > 4){
<<<<<<< HEAD
        levelBox->addItem( "Level 3, 4" );
        levelBox->addItem( "Level 5, 6" );
        if(level > 6)
            levelBox->addItem( "Level 7,8" );
=======
        levelBox->addItem( i18n("Level 3, 4") );
        levelBox->addItem( i18n("Level 5, 6") );
        if(level > 6) {
            levelBox->addItem( i18n("Level 7, 8") );
        }
>>>>>>> fc995d16487f587ad00aa548badbab77af0493c7
    }
    else {
        levelBox->setVisible(false);
    }
}

void KeyboardPainter :: levelChanged(int l_id){
    kbframe->setL_id(l_id);
}

int KeyboardPainter::getHeight(){
   int height = kbframe->getHeight();
   height = 2.5 * height + 50;
   return height;
}

int KeyboardPainter::getWidth(){
   int width = kbframe->getWidth();
   width = 2.5 * width + 20;
   return width;
}

KeyboardPainter::~KeyboardPainter()
{
    delete kbframe;
    delete exitButton;
    delete levelBox;
}
