/*
 * Abstract class which provides API to change cursor layout indicator.
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

#ifndef CURSORLAYOUTINDICATOR_H
#define CURSORLAYOUTINDICATOR_H

#include <icursorlayoutindicator.h>

#include <KConfig>
#include <KConfigGroup>

#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QString>

class CursorTheme;

template <class ImageCombinerType, class CursorImageType> class CursorLayoutIndicator
{
private:
    QHash<CursorInfo::CursorType, QString>*                             cursorNames;
    QHash<QPair<CursorInfo::CursorType, QString>, CursorImageType>*     cachedImages;

protected:
    KConfig*                    kcmInputKdeConfig;
    KConfigGroup*               mouseConfigGroup;

    ImageCombinerType*          imageCombiner;

    QString getCursorName(CursorInfo::CursorType cursorType) const 
    {
        return this->cursorNames->value(cursorType);
    }
    
    bool cachedImageExists(CursorInfo::CursorType cursorType, QString & layout) const 
    {
        return this->cachedImages->contains(QPair<CursorInfo::CursorType, QString>(cursorType, layout));
    }
    
    CursorImageType getCachedImage(CursorInfo::CursorType cursorType, QString & layout) const 
    {
        return this->cachedImages->value(QPair<CursorInfo::CursorType, QString>(cursorType, layout));
    }
    
    void addCachedImage(CursorInfo::CursorType cursorType, QString & layout, CursorImageType cursorImage) const
    {
        this->cachedImages->insert(QPair<CursorInfo::CursorType, QString>(cursorType, layout), cursorImage);
    }

    virtual const CursorTheme* getCurrentCursorTheme() const = 0;
    virtual CursorImageType getCursorImage(CursorInfo::CursorType) const = 0;
    virtual void setCursorImage(CursorInfo::CursorType, CursorImageType) const = 0;

public:
    CursorLayoutIndicator() 
    {
        this->kcmInputKdeConfig = new KConfig("kcminputrc");
        this->mouseConfigGroup  = new KConfigGroup(this->kcmInputKdeConfig, "Mouse");
        
        this->imageCombiner     = new ImageCombinerType();

        this->cachedImages      = new QHash<QPair<CursorInfo::CursorType, QString>, CursorImageType>();

        this->cursorNames       = new QHash<CursorInfo::CursorType, QString>();

        this->cursorNames->insert(CursorInfo::IBEAM, "ibeam");
    }

    virtual ~CursorLayoutIndicator() 
    {
        delete this->kcmInputKdeConfig;
        delete this->mouseConfigGroup;
        delete this->imageCombiner;
        delete this->cursorNames;
        delete this->cachedImages;
    }
};

#endif // CURSORLAYOUTINDICATOR_H


