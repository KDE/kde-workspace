/*
 * ImageCombiner takes 2 images and combine them by setting first image above/near the second.
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

#ifndef IMAGECOMBINER_H
#define IMAGECOMBINER_H

#include <cstddef>

template <class BelowImageType, class AboveImageType, class CombinedImageType> class ImageCombiner
{
private:
    BelowImageType*     imageBelow;
    AboveImageType*     imageAbove;
    
    CombinedImageType*  combinedImage;
    
protected:
    virtual void combineImages() = 0;
    
    void setResultImage(CombinedImageType *combinedImage)
    {   
        this->combinedImage = new CombinedImageType();
        
        *this->combinedImage = *combinedImage;
    }
    
public:
    ImageCombiner()
    {
        this->imageBelow = new BelowImageType();
        this->imageAbove = new AboveImageType();
    }
    
    virtual ~ImageCombiner()
    {   
        delete this->imageBelow;
        delete this->imageAbove;
        
        delete this->combinedImage;
    }
    
    virtual bool imagesBeenSet() = 0;
    
    virtual CombinedImageType getCombinedImage()
    {   
        this->combineImages();
        
        if (this->combinedImage != NULL) {
            return *(this->combinedImage);
        }
        
        return CombinedImageType();
    }
    
    BelowImageType* getImageBelow()
    {
        return this->imageBelow;
    }
    
    AboveImageType* getImageAbove()
    {
        return this->imageAbove;
    }
    
    void setSourceImages(BelowImageType imageBelow, AboveImageType imageAbove)
    {
        this->imageBelow = &imageBelow;
        this->imageAbove = &imageAbove;
    }
};

#endif // IMAGECOMBINER_H

