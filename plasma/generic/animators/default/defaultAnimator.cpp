/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "defaultAnimator.h"

#include <QGraphicsItem>
#include <QPainter>

#include <QDebug>

DefaultAnimator::DefaultAnimator(QObject *parent, const QVariantList& list)
    : Plasma::AnimationDriver(parent)
{
    Q_UNUSED(list)
}

int DefaultAnimator::animationFps(Plasma::Animator::Animation animation) const
{
    switch (animation) {
        case Plasma::Animator::AppearAnimation:
            return 20;
        case Plasma::Animator::DisappearAnimation:
            return 20;

        default:
            return 0;
    }
}

int DefaultAnimator::elementAnimationFps(Plasma::Animator::Animation animation) const
{
    switch (animation) {
        case Plasma::Animator::AppearAnimation:
            return 20;
        case Plasma::Animator::DisappearAnimation:
            return 20;

        default:
            return 0;
    }
}

void DefaultAnimator::itemAppear(qreal progress, QGraphicsItem* item)
{
    //qDebug() << "DefaultAnimator::appear(" << progress << ", " << item << ")";
    if (progress >= 1) {
        item->resetTransform();
        return;
    }
    item->resetTransform();
    QRectF r = item->boundingRect();
    item->translate(r.width() / 2 * progress, r.height() / 2 * progress);
    item->scale(progress, progress);
}

void DefaultAnimator::itemDisappear(qreal progress, QGraphicsItem* item)
{
    if (progress >= 1) {
        //item->resetTransform();
        return;
    }
    item->resetTransform();
    QRectF r = item->boundingRect();
    item->translate(r.width() / 2 * progress, r.height() / 2 * progress);
    item->scale(1-progress,1-progress);
}

QPixmap DefaultAnimator::elementAppear(qreal progress, const QPixmap& pixmap)
{
    //qDebug() << progress;
    QPixmap pix = pixmap;

    if (progress < 1) {
        QColor alpha;
        alpha.setAlphaF(progress);

        QPainter painter(&pix);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.fillRect(pix.rect(), alpha);
    }

    return pix;
}

QPixmap DefaultAnimator::elementDisappear(qreal progress, const QPixmap& pixmap)
{
    //qDebug() << progress;
    QPixmap pix = pixmap;

    if (progress > 0) {
        QColor alpha;
        alpha.setAlphaF(1 - progress);

        QPainter painter(&pix);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.fillRect(pix.rect(), alpha);
    }

    return pix;
}

#include "defaultAnimator.moc"
