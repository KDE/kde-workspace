/*
 * X Window System cursor layout indicator realization.
 * Copyright 2013  Victor Polevoy <vityatheboss@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <kdebug.h>

#include "x11cursorlayoutindicator.h"
#include "x11cursorimagecombiner.h"

#include "x11_helper.h"

#include <QtGui/QPixmap>
#include <QtGui/QCursor>
#include <QtGui/QImage>
#include <QtGui/QIcon>
#include <QtCore/QDir>
#include <QtGui/QX11Info>
#include <QtCore/QFile>

#include <config-X11.h>
#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

#ifdef HAVE_XFIXES
#  include <X11/extensions/Xfixes.h>
#endif

#include <keyboard_config.h>
#include <KGlobalSettings>
#include "flags.h"
#include <xcursortheme.h>
#include <thememodel.h>

#include "../../krdb/krdb.h"

// 24 is maximum for all flag icons; found experimentally
const int X11CursorLayoutIndicator::flagImageSize = 24;


X11CursorLayoutIndicator::X11CursorLayoutIndicator()
{
    this->cursorThemeModel = new CursorThemeModel();
}

X11CursorLayoutIndicator::~X11CursorLayoutIndicator()
{
    delete this->cursorThemeModel;
}

QImage X11CursorLayoutIndicator::getFlagImage(LayoutUnit& currentLayout, KeyboardConfig& keyboardConfig) const
{
    Flags flags;
    QImage flagImage = flags.getIconWithText(currentLayout, keyboardConfig).pixmap(flagImageSize).toImage();

    if (flagImage.isNull()) {
        kError() << "Error: flag image is null";
    }

    return flagImage;
}

const CursorTheme* X11CursorLayoutIndicator::getCurrentCursorTheme() const
{
    QString currentTheme = XcursorGetTheme(QX11Info().display());

    currentTheme = this->mouseConfigGroup->readEntry("cursorTheme", currentTheme);

    return this->cursorThemeModel->theme(this->cursorThemeModel->findIndex(currentTheme));
}

QImage X11CursorLayoutIndicator::getCursorImage(CursorInfo::CursorType cursorType) const
{
    QString cursorName      	= this->getCursorName(cursorType);
    QImage cursorDefaultImage   = this->getCurrentCursorTheme()->loadImage(cursorName, 0);

    if (cursorDefaultImage.isNull()) {
        kError() << "Error: cursor image is null";
    }

    return cursorDefaultImage;
}

void X11CursorLayoutIndicator::setCursorImage(CursorInfo::CursorType cursorType, QImage cursorImage) const
{
    //TODO Make new cursor appear in new windows and change it in firefox/chromium/other-non-kde applications.
    
    kDebug() << "Changing cursor icon";

    KGlobalSettings::self()->emitChange(KGlobalSettings::CursorChanged);
    runRdb(0);

    QString cursorName = this->getCursorName(cursorType);

    QCursor cursor(QPixmap::fromImage(cursorImage));

    // Tagging cursor with "cursorName"
    XFixesSetCursorName(QX11Info::display(), cursor.handle(), QFile::encodeName(cursorName));

    // Changing "cursorName"-cursors on the current display in all running applications
    XFixesChangeCursorByName(QX11Info::display(), cursor.handle(), QFile::encodeName(cursorName));
}

void X11CursorLayoutIndicator::setLayoutIndicator(CursorInfo::CursorType cursorType, LayoutUnit& layoutUnit, KeyboardConfig& keyboardConfig) const
{            
    if (!layoutUnit.layout.isEmpty()) {    
        if (!this->cachedImageExists(cursorType, layoutUnit.layout)) {
            QImage mouseCursorImage     	= this->getCursorImage(cursorType);
            QImage flagImage        	= this->getFlagImage(layoutUnit, keyboardConfig);

            if (!mouseCursorImage.isNull() && !flagImage.isNull()) {
                this->imageCombiner->setSourceImages(mouseCursorImage, flagImage);
                
                QImage combinedImage = this->imageCombiner->getCombinedImage();

                this->setCursorImage(cursorType, combinedImage);

                this->addCachedImage(cursorType, layoutUnit.layout, combinedImage);
            }
        } else {
            this->setCursorImage(cursorType, this->getCachedImage(cursorType, layoutUnit.layout));
        }
    }
}

