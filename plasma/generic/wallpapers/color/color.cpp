/*
 *   Copyright 2008 by Petri Damsten <damu@iki.fi>
 *   Copyright 2012 by Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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

#include "color.h"

#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QStandardItemModel>
#include <QStandardItem>
#include <KDebug>
#include "backgroundlistmodel.h"
#include "backgrounddelegate.h"

K_EXPORT_PLASMA_WALLPAPER(color, Color)

enum BackgroundMode {
    SOLID,
    HORIZONTAL,
    VERTICAL,
    RECTANGULAR,
    RADIAL,
    TOP_LEFT_DIAGONAL,
    TOP_RIGHT_DIAGONAL
};

Color::Color(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args),
      m_model(0)
{
}

void Color::init(const KConfigGroup &config)
{
    m_color1 = config.readEntry("color1", QColor(Qt::white));
    m_color2 = config.readEntry("color2", QColor(Qt::black));
    m_backgroundMode = config.readEntry("backgroundMode", (int)SOLID);
    emit update(boundingRect());
}

void Color::paint(QPainter *painter, const QRectF& exposedRect)
{
    generatePainting(m_backgroundMode, painter, exposedRect, boundingRect());
}

QWidget* Color::createConfigurationInterface(QWidget* parent)
{
    QWidget *widget = new QWidget(parent);
    m_ui.setupUi(widget);

    m_ui.m_color1->setColor(m_color1);
    m_ui.m_color2->setColor(m_color2);

    m_model = new BackgroundListModel(this, widget);
    m_model->addColor(0,i18n("Solid"));
    m_model->addColor(1,i18n("Horizontal"));
    m_model->addColor(2,i18n("Vertical"));
    m_model->addColor(3,i18n("Rectangular"));
    m_model->addColor(4,i18n("Radial"));
    m_model->addColor(5,i18n("Top Left Diagonal"));
    m_model->addColor(6,i18n("Top Right Diagonal"));

    m_ui.m_view->setItemDelegate(new BackgroundDelegate(m_ui.m_view));
    m_ui.m_view->setMinimumWidth((BackgroundDelegate::SCREENSHOT_SIZE + BackgroundDelegate::MARGIN * 2 +
                                  BackgroundDelegate::BLUR_INCREMENT) * 3 +
                                  m_ui.m_view->spacing() * 4 +
                                  QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) +
                                  QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2 + 7);
    m_ui.m_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_ui.m_view->setModel(m_model);
    connect(m_ui.m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(backgroundModeChanged(QModelIndex)));

    QModelIndex index = m_model->indexOf(m_backgroundMode);
    if (index.isValid()) {
        m_ui.m_view->setCurrentIndex(index);
    }

    connect(m_ui.m_color1, SIGNAL(changed(QColor)), SLOT(widgetChanged()));
    connect(m_ui.m_color2, SIGNAL(changed(QColor)), SLOT(widgetChanged()));

    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));

    return widget;
}

void Color::save(KConfigGroup &config)
{
    config.writeEntry("color1", m_color1);
    config.writeEntry("color2", m_color2);
    config.writeEntry("backgroundMode", m_backgroundMode);
}

void Color::backgroundModeChanged(const QModelIndex &index)
{
    if (index.row() == -1 || !m_model) {
        return;
    }

    m_backgroundMode = m_model->backgroundMode(index.row());

    emit settingsChanged(true);
    emit update(boundingRect());
}

void Color::widgetChanged()
{
    const QColor newColor1 = m_ui.m_color1->color();
    const QColor newColor2 = m_ui.m_color2->color();
    const bool updateThumbs = (m_color1 != newColor1) || (m_color2 != newColor2);

    m_color1 = newColor1;
    m_color2 = newColor2;

    if (updateThumbs) {
        m_model->reload();
    }

    emit settingsChanged(true);
    emit update(boundingRect());
}

void Color::generatePainting(int mode, QPainter* painter, const QRectF& exposedRect, const QRectF& boundingRect) const
{
    switch (mode) {
    case SOLID: {
            painter->fillRect(exposedRect, m_color1);
            break;
        }

    case HORIZONTAL: {
            QLinearGradient gradient = QLinearGradient(boundingRect.topLeft(),
                                                       boundingRect.topRight());
            gradient.setColorAt(0, m_color1);
            gradient.setColorAt(1, m_color2);
            painter->fillRect(exposedRect, gradient);
            break;
        }

    case VERTICAL: {
            QLinearGradient gradient = QLinearGradient(boundingRect.topLeft(),
                                                       boundingRect.bottomLeft());
            gradient.setColorAt(0, m_color1);
            gradient.setColorAt(1, m_color2);
            painter->fillRect(exposedRect, gradient);
            break;
        }

    case RECTANGULAR: {
            // First draw a horizontal gradient covering the whole view/screen
            QLinearGradient horizontalGradient = QLinearGradient(boundingRect.topLeft(),
                                                                 boundingRect.topRight());
            horizontalGradient.setColorAt(0, m_color2);
            horizontalGradient.setColorAt(0.5, m_color1);
            horizontalGradient.setColorAt(1, m_color2);

            painter->fillRect(exposedRect, horizontalGradient);

            // Then draw two triangles with vertical gradient
            QLinearGradient verticalGradient = QLinearGradient(boundingRect.topLeft(),
                                                               boundingRect.bottomLeft());
            verticalGradient.setColorAt(0, m_color2);
            verticalGradient.setColorAt(0.5, m_color1);
            verticalGradient.setColorAt(1, m_color2);
            painter->setBrush(verticalGradient);
            painter->setPen(Qt::NoPen);

            QPolygon triangle = QPolygon(3);

            // Draw a triangle which starts from the top edge to the center
            triangle.append(boundingRect.topLeft().toPoint());
            triangle.append(boundingRect.topRight().toPoint());
            triangle.append(boundingRect.center().toPoint());
            painter->drawPolygon(triangle);

            triangle.clear();

            // Draw a triangle which starts from the bottom edge to the center
            triangle.append(boundingRect.bottomLeft().toPoint());
            triangle.append(boundingRect.bottomRight().toPoint());
            triangle.append(boundingRect.center().toPoint());
            painter->drawPolygon(triangle);

            break;
        }

    case RADIAL: {
            // The diameter of the gradient will be the max screen dimension
            int maxDimension = qMax(boundingRect.height(), boundingRect.width());

            QRadialGradient gradient = QRadialGradient(boundingRect.center(),
                                                       maxDimension / 2,
                                                       boundingRect.center());
            gradient.setColorAt(0, m_color1);
            gradient.setColorAt(1, m_color2);
            painter->fillRect(exposedRect, gradient);
            break;
        }

    case TOP_LEFT_DIAGONAL: {
            QLinearGradient gradient = QLinearGradient(boundingRect.topLeft(),
                                                       boundingRect.bottomRight());
            gradient.setColorAt(0, m_color1);
            gradient.setColorAt(1, m_color2);
            painter->fillRect(exposedRect, gradient);
            break;
        }

    case TOP_RIGHT_DIAGONAL: {
            QLinearGradient gradient = QLinearGradient(boundingRect.topRight(),
                                                       boundingRect.bottomLeft());
            gradient.setColorAt(0, m_color1);
            gradient.setColorAt(1, m_color2);
            painter->fillRect(exposedRect, gradient);
            break;
        }

    }
}
#include "color.moc"
