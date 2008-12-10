/***************************************************************************
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                        *
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "clock.h"

#include <math.h>

#include <QApplication>
#include <QBitmap>
#include <QGraphicsScene>
#include <QMatrix>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>

#include <KConfigDialog>
#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KSharedConfig>
#include <KTimeZoneWidget>
#include <KDialog>

#include <Plasma/Dialog>
#include <Plasma/PaintUtils>
#include <Plasma/Svg>
#include <Plasma/Theme>

Clock::Clock(QObject *parent, const QVariantList &args)
    : ClockApplet(parent, args),
      m_showSecondHand(false),
      m_showTimezoneString(false),
      m_repaintCache(RepaintAll),
      m_secondHandUpdateTimer(0)
{
    KGlobal::locale()->insertCatalog("libplasmaclock");

    setHasConfigurationInterface(true);
    resize(125, 125);
    setAspectRatioMode(Plasma::Square);

    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/clock");
    m_theme->setContainsMultipleImages(true);
    m_theme->resize(size());

    connect(m_theme, SIGNAL(repaintNeeded()), this, SLOT(repaintNeeded()));
}

Clock::~Clock()
{
}

void Clock::init()
{
    ClockApplet::init();

    KConfigGroup cg = config();
    m_showSecondHand = cg.readEntry("showSecondHand", false);
    m_showTimezoneString = cg.readEntry("showTimezoneString", false);
    m_fancyHands = cg.readEntry("fancyHands", false);
    setCurrentTimezone(cg.readEntry("timezone", localTimezone()));

    connectToEngine();
}

void Clock::connectToEngine()
{
    Plasma::DataEngine* timeEngine = dataEngine("time");
    if (m_showSecondHand) {
        timeEngine->connectSource(currentTimezone(), this, 500);
    } else {
        timeEngine->connectSource(currentTimezone(), this, 6000, Plasma::AlignToMinute);
    }
}

void Clock::constraintsEvent(Plasma::Constraints constraints)
{
    ClockApplet::constraintsEvent(constraints);

    if (constraints & Plasma::FormFactorConstraint) {
        setBackgroundHints(NoBackground);
    }

    if (constraints & Plasma::SizeConstraint) {
        m_theme->resize(size());
    }

    m_repaintCache = RepaintAll;
}

QPainterPath Clock::shape() const
{
    if (m_theme->hasElement("hint-square-clock")) {
        return Applet::shape();
    }

    QPainterPath path;
    // we adjust by 2px all around to allow for smoothing the jaggies
    // if the ellipse is too small, we'll get a nastily jagged edge around the clock
    path.addEllipse(boundingRect().adjusted(-2, -2, 2, 2));
    return path;
}

void Clock::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    m_time = data["Time"].toTime();

    if (m_time.minute() == m_lastTimeSeen.minute() &&
        m_time.second() == m_lastTimeSeen.second()) {
        // avoid unnecessary repaints
        return;
    }

    if (m_time.minute() != m_lastTimeSeen.minute()) {
        m_repaintCache = RepaintHands;
    }

    if (Plasma::ToolTipManager::self()->isVisible(this)) {
        updateTipContent();
    }

    if (m_secondHandUpdateTimer) {
        m_secondHandUpdateTimer->stop();
    }

    m_lastTimeSeen = m_time;
    update();
}

void Clock::createClockConfigurationInterface(KConfigDialog *parent)
{
    //TODO: Make the size settable
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("General"), icon());

    ui.showSecondHandCheckBox->setChecked(m_showSecondHand);
    ui.showTimezoneStringCheckBox->setChecked(m_showTimezoneString);
}

void Clock::clockConfigAccepted()
{
    KConfigGroup cg = config();
    m_showSecondHand = ui.showSecondHandCheckBox->isChecked();
    m_showTimezoneString = ui.showTimezoneStringCheckBox->isChecked();

    cg.writeEntry("showSecondHand", m_showSecondHand);
    cg.writeEntry("showTimezoneString", m_showTimezoneString);
    update();

    dataEngine("time")->disconnectSource(currentTimezone(), this);
    connectToEngine();

    //TODO: why we don't call updateConstraints()?
    constraintsEvent(Plasma::AllConstraints);
    emit configNeedsSaving();
}

void Clock::changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone)
{
    dataEngine("time")->disconnectSource(oldTimezone, this);
    Plasma::DataEngine* timeEngine = dataEngine("time");

    if (m_showSecondHand) {
        timeEngine->connectSource(newTimezone, this, 500);
    } else {
        timeEngine->connectSource(newTimezone, this, 6000, Plasma::AlignToMinute);
    }

    m_repaintCache = RepaintAll;
}

void Clock::repaintNeeded()
{
    m_repaintCache = RepaintAll;
}

void Clock::moveSecondHand()
{
    //kDebug() << "moving second hand";
    update();
}

void Clock::drawHand(QPainter *p, const QRect &rect, const qreal verticalTranslation, const qreal rotation, const QString &handName)
{
    // this code assumes that the _vertical_ placment of the hands in the svg file is
    // properly chosen to obtain the desired hand offset with respect to the center

    p->save();

    p->translate(rect.width() / 2, rect.height() / 2);
    p->rotate(rotation);

    QRectF elementRect;
    QString name = handName + "HandShadow";
    if (m_theme->hasElement(name)) {
        p->save();

        elementRect = m_theme->elementRect(name);
        static const QSizeF offset = QSizeF(1, 3);
        p->translate(-elementRect.width() / 2 + offset.width(), elementRect.y() - verticalTranslation + offset.height());
        m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

        p->restore();
    }

    name = handName + "Hand";
    elementRect = m_theme->elementRect(name);
    p->translate(-elementRect.width() / 2, elementRect.y() - verticalTranslation);
    m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

    p->restore();
}

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{
    Q_UNUSED(option)

    // compute hand angles
    const qreal minutes = 6.0 * m_time.minute() - 180;
    const qreal hours = 30.0 * m_time.hour() - 180 +
                        ((m_time.minute() / 59.0) * 30.0);
    qreal seconds = 0;
    if (m_showSecondHand) {
        static const double anglePerSec = 6;
        seconds = anglePerSec * m_time.second() - 180;

        if (m_fancyHands) {
            if (!m_secondHandUpdateTimer) {
                m_secondHandUpdateTimer = new QTimer(this);
                connect(m_secondHandUpdateTimer, SIGNAL(timeout()), this, SLOT(moveSecondHand()));
            }

            if (!m_secondHandUpdateTimer->isActive()) {
                //kDebug() << "starting second hand movement";
                m_secondHandUpdateTimer->start(50);
                m_animationStart = QTime::currentTime().msec();
            } else {
                static const int runTime = 500;
                static const double m = 1; // Mass
                static const double b = 1; // Drag coefficient
                static const double k = 1.5; // Spring constant
                static const double PI = 3.141592653589793; // the universe is irrational
                static const double gamma = b / (2 * m); // Dampening constant
                static const double omega0 = sqrt(k / m);
                static const double omega1 = sqrt(omega0 * omega0 - gamma * gamma);
                const double elapsed = QTime::currentTime().msec() - m_animationStart;
                const double t = (4 * PI) * (elapsed / runTime);
                const double val = 1 + exp(-gamma * t) * -cos(omega1 * t);

                if (elapsed > runTime) {
                    m_secondHandUpdateTimer->stop();
                } else {
                    seconds += -anglePerSec + (anglePerSec * val);
                }
            }
        } else {
            if (!m_secondHandUpdateTimer) {
                m_secondHandUpdateTimer = new QTimer(this);
                connect(m_secondHandUpdateTimer, SIGNAL(timeout()), this, SLOT(moveSecondHand()));
            }

            if (!m_secondHandUpdateTimer->isActive()) {
                m_secondHandUpdateTimer->start(50);
                seconds += 1;
            } else {
                m_secondHandUpdateTimer->stop();
            }
        }
    }

    // paint face and glass cache
    if (m_repaintCache == RepaintAll) {
        m_faceCache = QPixmap(rect.size());
        m_glassCache = QPixmap(rect.size());
        m_faceCache.fill(Qt::transparent);
        m_glassCache.fill(Qt::transparent);

        QPainter facePainter(&m_faceCache);
        QPainter glassPainter(&m_glassCache);
        facePainter.setRenderHint(QPainter::SmoothPixmapTransform);
        glassPainter.setRenderHint(QPainter::SmoothPixmapTransform);

        m_theme->paint(&facePainter, rect, "ClockFace");

        // optionally paint the time string
        if (m_showTimezoneString || shouldDisplayTimezone()) {
            QString time = prettyTimezone();
            QFontMetrics fm(QApplication::font());
            const int margin = 4;

            if (!time.isEmpty()) {
                const qreal labelHeight = fm.height() + 2 * margin;
                // for small clocks, compute a minimum offset
                qreal labelOffset = m_theme->elementRect("HandCenterScrew").height() / 2 + labelHeight;
                // for larger clocks, add a relative component to the offset
                if ((rect.height() / 2) / 3 > labelHeight) {
                    labelOffset += rect.height() / 2 * 0.05;
                }
                QRect textRect(rect.width() / 2 - fm.width(time) / 2, rect.height() / 2 - labelOffset,
                      fm.width(time), fm.height());

                facePainter.setPen(Qt::NoPen);
                QColor background = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
                background.setAlphaF(0.5);
                facePainter.setBrush(background);

                facePainter.setRenderHint(QPainter::Antialiasing, true);
                facePainter.drawPath(Plasma::PaintUtils::roundedRectangle(textRect.adjusted(-margin, -margin, margin, margin), margin));
                facePainter.setRenderHint(QPainter::Antialiasing, false);

                facePainter.setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));

                facePainter.drawText(textRect, Qt::AlignCenter, time);
            }
        }

        glassPainter.save();
        QRectF elementRect = QRectF(QPointF(0, 0), m_theme->elementSize("HandCenterScrew"));
        glassPainter.translate(rect.width() / 2 - elementRect.width() / 2, rect.height() / 2 - elementRect.height() / 2);
        m_theme->paint(&glassPainter, elementRect, "HandCenterScrew");
        glassPainter.restore();

        m_theme->paint(&glassPainter, rect, "Glass");

        // get vertical translation, see drawHand() for more details
        m_verticalTranslation = m_theme->elementRect("ClockFace").center().y();
    }

    // paint hour and minute hands cache
    if (m_repaintCache == RepaintHands || m_repaintCache == RepaintAll) {
        m_handsCache = QPixmap(rect.size());
        m_handsCache.fill(Qt::transparent);

        QPainter handsPainter(&m_handsCache);
        handsPainter.setRenderHint(QPainter::SmoothPixmapTransform);

        drawHand(&handsPainter, rect, m_verticalTranslation, hours, "Hour");
        drawHand(&handsPainter, rect, m_verticalTranslation, minutes, "Minute");
    }

    // reset repaint cache flag
    m_repaintCache = RepaintNone;

    // paint caches and second hand
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->drawPixmap(rect, m_faceCache, rect);
    p->drawPixmap(rect, m_handsCache, rect);
    if (m_showSecondHand) {
        drawHand(p, rect, m_verticalTranslation, seconds, "Second");
    }
    p->drawPixmap(rect, m_glassCache, rect);

}

#include "clock.moc"
