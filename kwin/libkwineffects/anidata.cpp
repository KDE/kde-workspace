/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2011 Thomas Lübking <thomas.luebking@web.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "anidata_p.h"

#include <KDebug>

QDebug operator<<(QDebug dbg, const KWin::AniData &a)
{
    dbg.nospace() << a.debugInfo();
    return dbg.space();
}

using namespace KWin;
static int Gaussian = 46;

AniData::AniData()
{
    attribute = AnimationEffect::Opacity;
    windowType = (NET::WindowTypeMask)0;
    duration = time = meta = startTime = 0;
    waitAtSource = keepAtTarget = false;
}

AniData::AniData(AnimationEffect::Attribute a, int meta, int ms, const FPx2 &to,
                 QEasingCurve curve, int delay, const FPx2 &from, bool waitAtSource, bool keepAtTarget )
{
    attribute = a;
    this->from = from;
    this->to = to;
    this->curve = curve;
    duration = ms;
    time = 0;
    this->meta = meta;
    this->waitAtSource = waitAtSource;
    this->keepAtTarget = keepAtTarget;
    startTime = AnimationEffect::clock() + delay;
}

AniData::AniData(const AniData &other)
{
    attribute = other.attribute;
    from = other.from;
    to = other.to;
    time = other.time;
    duration = other.duration;
    curve = other.curve;
    customCurve = other.customCurve;
    windowType = other.windowType;
    meta = other.meta;
    waitAtSource = other.waitAtSource;
    keepAtTarget = other.keepAtTarget;
    startTime = other.startTime;
}

static FPx2 fpx2(const QString &s, AnimationEffect::Attribute a)
{
    bool ok; float f1, f2;
    QStringList floats = s.split(QStringLiteral(","));
    f1 = floats.at(0).toFloat(&ok);
    if (!ok || (f1 < 0.0 && !(  a == AnimationEffect::Position ||
                                a == AnimationEffect::Translation ||
                                a == AnimationEffect::Size ||
                                a == AnimationEffect::Rotation)) ) {
        if (ok)
            kDebug(1212) << "Invalid value (must not be negative)" << s;
        return FPx2();
    }

    bool forced_align = (floats.count() < 2);
    if (forced_align)
        f2 = f1;
    else {
        f2 = floats.at(1).toFloat(&ok);
        if ( (forced_align = !ok || (f2 < 0.0 && !( a == AnimationEffect::Position ||
                                                    a == AnimationEffect::Translation ||
                                                    a == AnimationEffect::Size ||
                                                    a == AnimationEffect::Rotation))) )
            f2 = f1;
    }
    if ( forced_align && a >= AnimationEffect::NonFloatBase )
        kDebug(1212) << "Generic Animations, WARNING: had to align second dimension of non-onedimensional attribute" << a;
    return FPx2(f1, f2);
}

AniData::AniData(const QString &str) // format: WindowMask:Attribute:Meta:Duration:To:Shape:Delay:From
{
    time = 0;
    duration = 1; // invalidate
    customCurve = 0; // Linear

    QStringList animation = str.split(u':');
    if (animation.count() < 5)
        return; // at least window type, attribute, metadata, time and target is required

    windowType = (NET::WindowTypeMask)animation.at(0).toUInt();

    if (animation.at(1) == QStringLiteral("Opacity"))           attribute = AnimationEffect::Opacity;
    else if (animation.at(1) == QStringLiteral("Brightness"))   attribute = AnimationEffect::Brightness;
    else if (animation.at(1) == QStringLiteral("Saturation"))   attribute = AnimationEffect::Saturation;
    else if (animation.at(1) == QStringLiteral("Scale"))        attribute = AnimationEffect::Scale;
    else if (animation.at(1) == QStringLiteral("Translation"))  attribute = AnimationEffect::Translation;
    else if (animation.at(1) == QStringLiteral("Rotation"))     attribute = AnimationEffect::Rotation;
    else if (animation.at(1) == QStringLiteral("Position"))     attribute = AnimationEffect::Position;
    else if (animation.at(1) == QStringLiteral("Size"))         attribute = AnimationEffect::Size;
    else if (animation.at(1) == QStringLiteral("Clip"))         attribute = AnimationEffect::Clip;
    else {
        kDebug(1212) << "Invalid attribute" << animation.at(1);
        return;
    }

    meta = animation.at(2).toUInt();

    bool ok;
    duration = animation.at(3).toInt(&ok);
    if (!ok || duration < 0) {
        kDebug(1212) << "Invalid duration" << animation.at(3);
        duration = 0;
        return;
    }

    to = fpx2(animation.at(4), attribute);

    if (animation.count() > 5) {
        customCurve = animation.at(5).toInt();
        if (customCurve < QEasingCurve::Custom)
            curve.setType((QEasingCurve::Type)customCurve);
        else if (customCurve == Gaussian)
            curve.setCustomType(AnimationEffect::qecGaussian);
        else
            kDebug(1212) << "Unknown curve type" << customCurve; // remains default, ie. linear

        if (animation.count() > 6) {
            int t = animation.at(6).toInt();
            if (t < 0)
                kDebug(1212) << "Delay can not be negative" << animation.at(6);
            else
                time = t;

            if (animation.count() > 7)
                from = fpx2(animation.at(7), attribute);
        }
    }
    if (!(from.isValid() || to.isValid())) {
        duration = -1; // invalidate
        return;
    }
}

static QString attributeString(KWin::AnimationEffect::Attribute attribute)
{
    switch (attribute) {
    case KWin::AnimationEffect::Opacity:      return QStringLiteral("Opacity");
    case KWin::AnimationEffect::Brightness:   return QStringLiteral("Brightness");
    case KWin::AnimationEffect::Saturation:   return QStringLiteral("Saturation");
    case KWin::AnimationEffect::Scale:        return QStringLiteral("Scale");
    case KWin::AnimationEffect::Translation:  return QStringLiteral("Translation");
    case KWin::AnimationEffect::Rotation:     return QStringLiteral("Rotation");
    case KWin::AnimationEffect::Position:     return QStringLiteral("Position");
    case KWin::AnimationEffect::Size:         return QStringLiteral("Size");
    case KWin::AnimationEffect::Clip:         return QStringLiteral("Clip");
    default:                                  return QStringLiteral(" ");
    }
}

QList<AniData> AniData::list(const QString &str)
{
    QList<AniData> newList;
    QStringList list = str.split(u';', QString::SkipEmptyParts);
    foreach (const QString &astr, list) {
        newList << AniData(astr);
        if (newList.last().duration < 0)
            newList.removeLast();
    }
    return newList;
}

QString AniData::toString() const
{
    QString ret =   QString::number((uint)windowType) + QStringLiteral(":") + attributeString(attribute) + QStringLiteral(":") +
                    QString::number(meta) + QStringLiteral(":") + QString::number(duration) + QStringLiteral(":") +
                    to.toString() + QStringLiteral(":") + QString::number(customCurve) + QStringLiteral(":") +
                    QString::number(time) + QStringLiteral(":") + from.toString();
    return ret;
}

QString AniData::debugInfo() const
{
    return QStringLiteral("Animation: ") + attributeString(attribute) +
           QStringLiteral("\n     From: ") + from.toString() +
           QStringLiteral("\n       To: ") + to.toString() +
           QStringLiteral("\n  Started: ") + QString::number(AnimationEffect::clock() - startTime) + QStringLiteral("ms ago\n") +
           QStringLiteral(  " Duration: ") + QString::number(duration) + QStringLiteral("ms\n") +
           QStringLiteral(  "   Passed: ") + QString::number(time) + QStringLiteral("ms\n") +
           QStringLiteral(  " Applying: ") + QString::number(windowType) + QStringLiteral("\n");
}
