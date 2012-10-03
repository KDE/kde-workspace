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

#include "menubar.h"

#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QMenu>
#include <QDesktopWidget>

#include <KWindowSystem>
#include <Plasma/FrameSvg>
#include <Plasma/Label>
#include <Plasma/Meter>
#include <Plasma/Theme>
#include <Plasma/WindowEffects>
#include <KApplication>

MenuBar::MenuBar(QMenu *menu)
    : QGraphicsView(),
    m_hideTimer(new QTimer(this)),
    m_background(new Plasma::FrameSvg(this)),
    m_scene(new QGraphicsScene(this)),
    m_container(new MenuWidget(this, menu))
{
    qreal left, top, right, bottom;

    //Setup the window properties
    setWindowFlags(Qt::Tool|Qt::X11BypassWindowManagerHint|Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    KWindowSystem::setType(winId(), NET::Dock);
    setFrameStyle(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //Setup the widgets
    m_background->setImagePath("widgets/tooltip");
    m_background->setEnabledBorders(Plasma::FrameSvg::BottomBorder|Plasma::FrameSvg::LeftBorder|Plasma::FrameSvg::RightBorder);
    m_container->updateLayout();

    m_scene->addItem(m_container);

    setScene(m_scene);

    m_background->getMargins(left, top, right, bottom);
    m_container->layout()->setContentsMargins(left, top, right, bottom);
    resize(sizeHint());

    connect(m_container, SIGNAL(aboutToHide()), this, SLOT(slotAboutToHide()));
    connect(m_hideTimer, SIGNAL(timeout()), this, SLOT(slotAboutToHide()));
}

MenuBar::~MenuBar()
{
}

QSize MenuBar::sizeHint() const
{
    QSizeF size = m_container->minimumSize();
    return QSize(size.width(), size.height());
}

void MenuBar::show()
{
    m_hideTimer->start(1000);
    QGraphicsView::show();

}

void MenuBar::hide()
{
    emit aboutToHide();
    m_hideTimer->stop();
    QGraphicsView::hide();
}

void MenuBar::slotAboutToHide()
{
    if (m_container->aMenuIsVisible()) { // MenuBar::m_hideTimer
        m_hideTimer->stop(); // menu is visible, menubar will be hidden by another aboutToHide() signal
    }
    else if (!cursorInMenuBar()) { //MenuWidget::AboutToHide signal
        hide();
        emit aboutToHide();
    } else if (!m_hideTimer->isActive()){ //use click on menubar button while a popup was shown
        m_hideTimer->start(1000);
    }
}

bool MenuBar::cursorInMenuBar()
{
    return QRect(pos(), size()).contains(QCursor::pos());
}

void MenuBar::drawBackground(QPainter *painter, const QRectF &/*rectF*/)
{
    painter->save();
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    m_background->paintFrame(painter);
    painter->restore();
}

void MenuBar::resizeEvent(QResizeEvent*)
{
    m_background->resizeFrame(size());
    m_scene->setSceneRect(0, 0, width(), height());
    if (!KWindowSystem::compositingActive()) {
        setMask(m_background->mask());
    }
}

void MenuBar::showEvent(QShowEvent *)
{
    Plasma::WindowEffects::overrideShadow(winId(), true);
    Plasma::WindowEffects::enableBlurBehind(winId(), true, m_background->mask());
}