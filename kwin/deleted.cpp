/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>

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

#include "deleted.h"

#include "workspace.h"
#include "client.h"
#include "netinfo.h"
#include "paintredirector.h"
#include "shadow.h"

namespace KWin
{

Deleted::Deleted()
    : Toplevel()
    , delete_refcount(1)
    , no_border(true)
    , padding_left(0)
    , padding_top(0)
    , padding_right(0)
    , padding_bottom(0)
    , m_layer(UnknownLayer)
    , m_minimized(false)
    , m_modal(false)
    , m_paintRedirector(NULL)
    , m_wasClient(false)
{
}

Deleted::~Deleted()
{
    if (delete_refcount != 0)
        kError(1212) << "Deleted client has non-zero reference count (" << delete_refcount << ")";
    assert(delete_refcount == 0);
    workspace()->removeDeleted(this);
    deleteEffectWindow();
}

Deleted* Deleted::create(Toplevel* c)
{
    Deleted* d = new Deleted();
    d->copyToDeleted(c);
    workspace()->addDeleted(d, c);
    return d;
}

// to be used only from Workspace::finishCompositing()
void Deleted::discard()
{
    delete_refcount = 0;
    delete this;
}

void Deleted::copyToDeleted(Toplevel* c)
{
    assert(dynamic_cast< Deleted* >(c) == NULL);
    Toplevel::copyToDeleted(c);
    desk = c->desktop();
    activityList = c->activities();
    contentsRect = QRect(c->clientPos(), c->clientSize());
    transparent_rect = c->transparentRect();
    m_layer = c->layer();
    if (WinInfo* cinfo = dynamic_cast< WinInfo* >(info))
        cinfo->disable();
    Client* client = dynamic_cast<Client*>(c);
    if (client) {
        m_wasClient = true;
        no_border = client->noBorder();
        padding_left = client->paddingLeft();
        padding_right = client->paddingRight();
        padding_bottom = client->paddingBottom();
        padding_top = client->paddingTop();
        if (!no_border) {
            client->layoutDecorationRects(decoration_left,
                                          decoration_top,
                                          decoration_right,
                                          decoration_bottom,
                                          Client::WindowRelative);
            if (PaintRedirector *redirector = client->decorationPaintRedirector()) {
                redirector->ensurePixmapsPainted();
                redirector->reparent(this);
                m_paintRedirector = redirector;
            }
        }
        m_minimized = client->isMinimized();
        m_modal = client->isModal();
        m_mainClients = client->mainClients();
        foreach (Client *c, m_mainClients) {
            connect(c, SIGNAL(windowClosed(KWin::Toplevel*,KWin::Deleted*)), SLOT(mainClientClosed(KWin::Toplevel*)));
        }
    }
}

void Deleted::unrefWindow()
{
    if (--delete_refcount > 0)
        return;
    // needs to be delayed
    // a) when calling from effects, otherwise it'd be rather complicated to handle the case of the
    // window going away during a painting pass
    // b) to prevent dangeling pointers in the stacking order, see bug #317765
    deleteLater();
}

int Deleted::desktop() const
{
    return desk;
}

QStringList Deleted::activities() const
{
    return activityList;
}

QPoint Deleted::clientPos() const
{
    return contentsRect.topLeft();
}

QSize Deleted::clientSize() const
{
    return contentsRect.size();
}

void Deleted::debug(QDebug& stream) const
{
    stream << "\'ID:" << window() << "\' (deleted)";
}

void Deleted::layoutDecorationRects(QRect& left, QRect& top, QRect& right, QRect& bottom, int) const
{
    left = decoration_left;
    top = decoration_top;
    right = decoration_right;
    bottom = decoration_bottom;
}

QRect Deleted::decorationRect() const
{
    return rect().adjusted(-padding_left, -padding_top, padding_top, padding_bottom);
}

QRect Deleted::transparentRect() const
{
    return transparent_rect;
}

bool Deleted::isDeleted() const
{
    return true;
}

NET::WindowType Deleted::windowType(bool direct, int supportedTypes) const
{
    Q_UNUSED(direct)
    // TODO: maybe retrieve the actual window type when copying to deleted?
    if (supportedTypes == 0) {
        supportedTypes = SUPPORTED_UNMANAGED_WINDOW_TYPES_MASK;
    }
    return info->windowType(supportedTypes);
}

void Deleted::mainClientClosed(Toplevel *client)
{
    m_mainClients.removeAll(static_cast<Client*>(client));
}

} // namespace

#include "deleted.moc"
