/*
 * X11CursorImageCombiner combines flag and cursor icons.
 * Copyright (C) 2013  Victor Polevoy <vityatheboss@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "x11cursorimagecombiner.h"

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QCursor>

#include <kdebug.h>


bool X11CursorImageCombiner::imagesBeenSet()
{
    QImage *imageBelow = getImageBelow();
    QImage *imageAbove = getImageAbove();
    
    if (imageAbove == NULL || imageBelow == NULL
        || imageAbove->isNull() || imageBelow->isNull()) {
        return false;
    }
    
    return true;        
}

/* Image below is cursor image, image above is country flag */
void X11CursorImageCombiner::combineImages()
{    
    if (imagesBeenSet()) {
        kDebug() << "Images been set, combining";
        
        QImage *imageBelow = getImageBelow();
        QImage *imageAbove = getImageAbove();
        
        /* Cursor ibeam-image needs to be in the middle of the result icon to avoid visual issues */
        /* 4 is the margins before and after (2 || image || 2) */
        int resultImageWidth = 2 * imageAbove->width() + 4 + imageBelow->width();
        int resultImageHeight = imageBelow->height();
        
        QImage cursorImage(resultImageWidth, resultImageHeight, QImage::Format_ARGB32);
        
        cursorImage.fill(Qt::transparent);
        
        QPainter painter;
        painter.begin(&cursorImage);
        painter.drawImage(imageAbove->width() + 2, 0, *imageBelow);
        painter.drawImage(resultImageWidth - imageAbove->width(), 0, *imageAbove);
        painter.end();
        
        setResultImage(&cursorImage);
    } else {
        kError() << "Error: one or two images have not been set, can't combine.";
    }
}