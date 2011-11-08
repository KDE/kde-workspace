/* This file is part of the KDE libraries

   Copyright (C) 2009 Marco Martin <notmart@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef SCREENPREVIEWWIDGET_H
#define SCREENPREVIEWWIDGET_H

#include <QWidget>

#include "kworkspace_export.h"

class Wallpaper;

class ScreenPreviewWidgetPrivate;

namespace Plasma
{
    class Wallpaper;
}

class KWORKSPACE_EXPORT ScreenPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    ScreenPreviewWidget(QWidget *parent);
    ~ScreenPreviewWidget();

    void setPreview(const QPixmap &preview);
    void setPreview(Plasma::Wallpaper* wallpaper);
    const QPixmap preview() const;
    void setRatio(const qreal ratio);
    qreal ratio() const;

    QRect previewRect() const;

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    virtual void dropEvent(QDropEvent *event);

Q_SIGNALS:
    void imageDropped(const QString &);

private:
    ScreenPreviewWidgetPrivate *const d;

    Q_PRIVATE_SLOT(d, void updateRect(const QRectF& rect))
    Q_PRIVATE_SLOT(d, void wallpaperDeleted())
};


#endif
