/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#include "panel.h"

#include <QAction>

#include <Plasma/Corona>
#include <Plasma/Containment>

#include "panelview.h"
#include "plasmaapp.h"
#include <plasmagenericshell/scripting/scriptengine.h>
#include <plasmagenericshell/scripting/widget.h>

namespace WorkspaceScripting
{

Panel::Panel(Plasma::Containment *containment, QObject *parent)
    : Containment(containment, parent)
{
}

Panel::~Panel()
{
}

QString Panel::location() const
{
    Plasma::Containment *c = containment();
    if (!c) {
        return "floating";
    }

    switch (c->location()) {
        case Plasma::Floating:
            return "floating";
            break;
        case Plasma::Desktop:
            return "desktop";
            break;
        case Plasma::FullScreen:
            return "fullscreen";
            break;
        case Plasma::TopEdge:
            return "top";
            break;
        case Plasma::BottomEdge:
            return "bottom";
            break;
        case Plasma::LeftEdge:
            return "left";
            break;
        case Plasma::RightEdge:
            return "right";
            break;
    }

    return "floating";
}

void Panel::setLocation(const QString &locationString)
{
    Plasma::Containment *c = containment();
    if (!c) {
        return;
    }

    const QString lower = locationString.toLower();
    Plasma::Location loc = Plasma::Floating;
    if (lower == "desktop") {
        loc = Plasma::Desktop;
    } else if (lower == "fullscreen") {
        loc = Plasma::FullScreen;
    } else if (lower == "top") {
        loc = Plasma::TopEdge;
    } else if (lower == "bottom") {
        loc = Plasma::BottomEdge;
    } else if (lower == "left") {
        loc = Plasma::LeftEdge;
    } else if (lower == "right") {
        loc = Plasma::RightEdge;
    }

    c->setLocation(loc);

    c->flushPendingConstraintsEvents();
}

PanelView *Panel::panel() const
{
    Plasma::Containment *c = containment();
    if (!c) {
        return 0;
    }

    foreach (PanelView *v, PlasmaApp::self()->panelViews()) {
        if (v->containment() == c) {
            return v;
        }
    }

    return 0;
}

QString Panel::alignment() const
{
    PanelView *v = panel();
    if (!v) {
        return "left";
    }

    switch (v->alignment()) {
        case Qt::AlignRight:
            return "right";
            break;
        case Qt::AlignCenter:
            return "center";
            break;
        default:
            return "left";
            break;
    }

    return "left";
}

void Panel::setAlignment(const QString &alignment)
{
    PanelView *v = panel();
    if (v) {
        bool success = false;

        if (alignment.compare("left", Qt::CaseInsensitive) == 0) {
            if (v->alignment() != Qt::AlignLeft) {
                success = true;
                v->setAlignment(Qt::AlignLeft);
            }
        } else if (alignment.compare("right", Qt::CaseInsensitive) == 0) {
            if (v->alignment() != Qt::AlignRight) {
                success = true;
                v->setAlignment(Qt::AlignRight);
            }
        } else if (alignment.compare("center", Qt::CaseInsensitive) == 0) {
            if (v->alignment() != Qt::AlignCenter) {
                success = true;
                v->setAlignment(Qt::AlignCenter);
            }
        }

        if (success) {
            v->setOffset(0);
        }
    }
}

int Panel::offset() const
{
    PanelView *v = panel();
    if (v) {
        return v->offset();
    }

    return 0;
}

void Panel::setOffset(int pixels)
{
    Plasma::Containment *c = containment();
    if (pixels < 0 || !c) {
        return;
    }

    PanelView *v = panel();
    if (v) {
        QRectF screen = c->corona()->screenGeometry(v->screen());
        QSizeF size = c->size();

        if (c->formFactor() == Plasma::Vertical) {
            if (pixels > screen.height()) {
                return;
            }

            if (size.height() + pixels > screen.height()) {
                c->resize(size.width(), screen.height() - pixels);
            }
        } else if (pixels > screen.width()) {
            return;
        } else if (size.width() + pixels > screen.width()) {
            size.setWidth(screen.width() - pixels);
            c->resize(size);
            c->setMinimumSize(size);
            c->setMaximumSize(size);
        }

        v->setOffset(pixels);
    }
}

int Panel::length() const
{
    Plasma::Containment *c = containment();
    if (!c) {
        return 0;
    }

    if (c->formFactor() == Plasma::Vertical) {
        return c->size().height();
    } else {
        return c->size().width();
    }
}

void Panel::setLength(int minPixels, int maxPixels)
{
    Plasma::Containment *c = containment();
    if ((minPixels < 0 && maxPixels < 0) || !c) {
        return;
    }

    PanelView *v = panel();
    if (v) {
        if (minPixels < 0) {
            minPixels = minLength();
        } else if (minPixels > maxPixels) {
            maxPixels = minPixels;
        }

        if (maxPixels < 0) {
            maxPixels = maxLength();
        } else if (minPixels > maxPixels) {
            minPixels = maxPixels;
        }

        int pixels = 0;
        if (minPixels == maxPixels) {
            pixels = minPixels;
        } else {
            pixels = qBound(minPixels,
                            c->formFactor() == Plasma::Vertical ?
                                c->preferredSize().toSize().height() :
                                c->preferredSize().toSize().width(),
                            maxPixels);
        }

        QRectF screen = c->corona()->screenGeometry(v->screen());
        QSizeF size = c->size();
        QSizeF minSize = c->minimumSize();
        QSizeF maxSize = c->maximumSize();
        if (c->formFactor() == Plasma::Vertical) {
            if (minPixels > screen.height() - v->offset()) {
                return;
            }

            size.setHeight(pixels);
            minSize.setHeight(minPixels);
            maxSize.setHeight(maxPixels);
        } else if (minPixels > screen.width() - v->offset()) {
            return;
        } else {
            size.setWidth(pixels);
            minSize.setWidth(minPixels);
            maxSize.setWidth(maxPixels);
        }

        //kDebug() << "sizes:" << minSize << size << maxSize;
        c->setMinimumSize(0, 0);
        c->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        c->resize(size);
        c->setMinimumSize(minSize);
        c->setMaximumSize(maxSize);
        v->pinchContainmentToCurrentScreen();
    }
}

void Panel::setLength(int pixels)
{
    setLength(pixels, pixels);
}

int Panel::minLength() const
{
    Plasma::Containment *c = containment();
    if (!c) {
        return 0;
    }

    if (c->formFactor() == Plasma::Vertical) {
        return c->minimumHeight();
    } else {
        return c->minimumWidth();
    }
}

void Panel::setMinLength(int pixels)
{
    setLength(pixels, -1);
}

int Panel::maxLength() const
{
    Plasma::Containment *c = containment();
    if (!c) {
        return 0;
    }

    if (c->formFactor() == Plasma::Vertical) {
        return c->maximumHeight();
    } else {
        return c->maximumWidth();
    }
}

void Panel::setMaxLength(int pixels)
{
    setLength(-1, pixels);
}

int Panel::height() const
{
    Plasma::Containment *c = containment();
    if (!c) {
        return 0;
    }

    return c->formFactor() == Plasma::Vertical ? c->size().width()
                                               : c->size().height();
}

void Panel::setHeight(int height)
{
    Plasma::Containment *c = containment();
    if (height < 16 || !c) {
        return;
    }

    PanelView *v = panel();
    if (v) {
        QRect screen = c->corona()->screenGeometry(v->screen());
        QSizeF size = c->size();
        const int max = (c->formFactor() == Plasma::Vertical ? screen.width() : screen.height()) / 3;
        height = qBound(16, height, max);

        if (c->formFactor() == Plasma::Vertical) {
            size.setWidth(height);
        } else {
            size.setHeight(height);
        }

        c->resize(size);
        c->setMinimumSize(size);
        c->setMaximumSize(size);
    }
}

QString Panel::hiding() const
{
    PanelView *v = panel();
    if (v) {
        switch (v->visibilityMode()) {
            case PanelView::NormalPanel:
                return "none";
                break;
            case PanelView::AutoHide:
                return "autohide";
                break;
            case PanelView::LetWindowsCover:
                return "windowscover";
                break;
            case PanelView::WindowsGoBelow:
                return "windowsbelow";
                break;
        }
    }

    return "none";
}

void Panel::setHiding(const QString &mode)
{
    PanelView *v = panel();
    if (v) {
        if (mode.compare("autohide", Qt::CaseInsensitive) == 0) {
            v->setVisibilityMode(PanelView::AutoHide);
        } else if (mode.compare("windowscover", Qt::CaseInsensitive) == 0) {
            v->setVisibilityMode(PanelView::LetWindowsCover);
        } else if (mode.compare("windowsbelow", Qt::CaseInsensitive) == 0) {
            v->setVisibilityMode(PanelView::WindowsGoBelow);
        } else {
            v->setVisibilityMode(PanelView::NormalPanel);
        }
    }
}

}

#include "panel.moc"

