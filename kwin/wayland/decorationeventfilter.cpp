/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

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

#include "decorationeventfilter.h"
#include "wayland_client.h"
// Qt
#include <QtGui/QWidget>

namespace KWin
{
namespace Wayland
{

DecorationEventFilter::DecorationEventFilter(KDecoration *decoration, Client *client)
    : QObject(decoration)
    , m_decoration(decoration)
    , m_client(client)
{
}

DecorationEventFilter::~DecorationEventFilter()
{
}

bool DecorationEventFilter::eventFilter(QObject *object, QEvent *event)
{
    if (!m_decoration->widget() || object != m_decoration->widget()) {
        return false;
    }
    if (QMouseEvent *me = dynamic_cast<QMouseEvent*>(event)) {
        const QRect clientRect = QRect(m_client->decorationPos(), m_client->clientSize());
        const QPoint pos = me->pos() - m_client->decorationPos();
        if (clientRect.contains(me->pos())) {
            switch (me->type()) {
            case QEvent::MouseMove:
                m_client->mouseMove(pos, me->globalPos());
                break;
            case QEvent::MouseButtonPress:
                m_client->mouseButtonPress(me->button());
                break;
            case QEvent::MouseButtonRelease:
                m_client->mouseButtonRelease(me->button());
                break;
            default:
                // ignore event
                break;
            }
            return true;
        }
    }
    return false;
}

} // namespace Wayland
} // namespace KWin
