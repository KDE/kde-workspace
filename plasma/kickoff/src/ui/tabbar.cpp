/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

// Own
#include "ui/tabbar.h"

// Qt
#include <QIcon>
#include <QPainter>
#include <QtDebug>

using namespace Kickoff;

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent)
{
}
QSize TabBar::tabSizeHint(int index) const
{
    QSize hint;
    const QFontMetrics metrics(font());
    const QSize textSize = metrics.size(Qt::TextHideMnemonic,tabText(index));
    
    hint.rwidth() = qMax(iconSize().width(),textSize.width());
    hint.rheight() = iconSize().height() + textSize.height();

    hint.rwidth() += 2*TAB_CONTENTS_MARGIN;
    hint.rheight() += 2*TAB_CONTENTS_MARGIN;

    return hint;
}
void TabBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    for(int i=0 ; i<count() ; i++) {
        QRect rect = tabRect(i).adjusted(TAB_CONTENTS_MARGIN,TAB_CONTENTS_MARGIN,
                                        -TAB_CONTENTS_MARGIN,-TAB_CONTENTS_MARGIN);

        // draw background for selected tab
        if (i==currentIndex()) {
            painter.fillRect(tabRect(i),palette().button());
        }

        // draw tab icon and text
        QFontMetrics metrics(painter.font());
        int textHeight = metrics.height();
        QRect iconRect = rect;
        iconRect.setBottom(iconRect.bottom()-textHeight);
        tabIcon(i).paint(&painter,iconRect);

        QRect textRect = rect;
        textRect.setTop(textRect.bottom()-textHeight);
        painter.drawText(textRect,Qt::AlignCenter|Qt::TextHideMnemonic,tabText(i));
    }
}

#include "tabbar.moc"
