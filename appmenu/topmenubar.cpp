/*
  This file is part of the KDE project.

  Copyright (c) 2011 Lionel Chauvin <megabigbug@yahoo.fr>
  Copyright (c) 2011,2012 Cédric Bellegarde <gnumdk@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#include "topmenubar.h"
#include "glowbar.h"

//KDE
#include <Plasma/Svg>
#include <KWindowSystem>

// Qt
#include <QMenu>
#include <QTimer>
#include <QDebug>

TopMenuBar::TopMenuBar(QMenu *menu)
    : MenuBar(menu),
    m_mouseTracker(new QTimer(this)),
    m_glowBar(0)
{
    connect(this, SIGNAL(aboutToHide()), this, SLOT(slotAboutToHide()));
    connect(m_mouseTracker, SIGNAL(timeout()), this, SLOT(slotMouseTracker()));
}

TopMenuBar::~TopMenuBar()
{
    disconnect(m_mouseTracker, SIGNAL(timeout()), this, SLOT(slotMouseTracker()));
    delete m_mouseTracker;
    if (m_glowBar) {
        m_glowBar->hide();
        delete m_glowBar;
        m_glowBar = 0;
    }
}

void TopMenuBar::enableMouseTracking(bool enable) {
    if (enable) {
        m_mouseTracker->start(250);
        slotDestroyGlowBar();
        m_glowBar = new GlowBar(triggerRect().topLeft(), triggerRect().width());
        connect(m_glowBar, SIGNAL(destroy()), this, SLOT(slotDestroyGlowBar()));
        m_glowBar->show();
    }
    else
        m_mouseTracker->stop();
}

bool TopMenuBar::cursorInMenuBar()
{
    if (m_mouseTracker->isActive()) {
        return triggerRect().contains(QCursor::pos());
    } else {
        return MenuBar::cursorInMenuBar();
    }
}

void TopMenuBar::slotAboutToHide()
{
    enableMouseTracking();
}

void TopMenuBar::slotMouseTracker()
{
    static QPoint prevCursorPos;

    if (cursorInMenuBar()) {
        m_mouseTracker->stop();
        slotDestroyGlowBar();
        show();
    } else  {
        QPoint cursorPos = QCursor::pos();
        // if cursor move, show glow bar
        if (!m_glowBar && cursorPos != prevCursorPos) {
            m_glowBar = new GlowBar(triggerRect().topLeft(), triggerRect().width());
            connect(m_glowBar, SIGNAL(destroy()), this, SLOT(slotDestroyGlowBar()));
            m_glowBar->show();
        }
        prevCursorPos = cursorPos;
    }
}

void TopMenuBar::slotDestroyGlowBar()
{
    if (m_glowBar) {
        m_glowBar->hide();
        delete m_glowBar;
        m_glowBar = 0;
    }
}
QRect TopMenuBar::triggerRect()
{
    QPoint triggerPoint = QPoint(x(), y());
    QSize triggerSize = QSize(sizeHint().width(), 5);
    return QRect(triggerPoint, triggerSize);
}
#include "topmenubar.moc"