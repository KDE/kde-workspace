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

#ifndef MENUBAR__H
#define MENUBAR__H

#include "menuwidget.h"

#include <QGraphicsView>

class QMenu;

namespace Plasma
{
class FrameSvg;
class Label;
class Meter;
}

class MenuBar : public QGraphicsView
{
Q_OBJECT
public:
    MenuBar(QMenu *menu = 0);
    ~MenuBar();

    /**
     * Auto open menu items on mouse over
     */
    void autoOpen() { m_container->autoOpen(); }

    /**
     * set action as active menubar action
     */
    void setActiveAction(QAction *action) { m_container->setActiveAction(action); }

    virtual QSize sizeHint() const;
    virtual void show();
    virtual void hide();

private Q_SLOTS:
    void slotAboutToHide();
Q_SIGNALS:
    void aboutToHide();
protected:
    /**
     * Return true if cursor in menubar
     */
    virtual bool cursorInMenuBar();
    virtual void drawBackground(QPainter* painter, const QRectF &rectF);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void showEvent(QShowEvent* event);
private:
    QTimer* m_hideTimer;
    Plasma::FrameSvg* m_background;
    QGraphicsScene* m_scene;
    MenuWidget* m_container;
};

#endif
