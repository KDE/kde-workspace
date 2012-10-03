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

#ifndef MENUWIDGET__H
#define MENUWIDGET__H

#include <QGraphicsWidget>
#include <QTimer>

class QGraphicsLinearLayout;
class QGraphicsView;
class MenuButton;

class MenuWidget : public QGraphicsWidget
{
Q_OBJECT
public:
    MenuWidget(QGraphicsView *view = 0, QMenu *menu = 0);
    ~MenuWidget();

    /**
     *  Update layout with root menu
     */
    void updateLayout();
    /**
     * Return true if layout is valid and populated
     */
    bool layoutValid();
    /**
     * True if a menu is visible in menubar
     */
    bool aMenuIsVisible() { return m_aMenuIsVisible; }

    /**
     * Activate action, or first action if null
     */
    void setActiveAction(QAction *action);

    /**
     * Auto open menu items on mouse over
     */
    void autoOpen() { m_mouseTimer->start(100); }

    void hide();
protected:
    /**
     * Use to get keyboard events
     */
    virtual bool eventFilter(QObject*, QEvent*);
private Q_SLOTS:
    /**
     * Check hovered item and active it
     */
    void slotCheckActiveItem();
    /**
     * a menu is hidding
     */
    void slotMenuAboutToHide();
    /**
     * menubar button clicked
     */
    void slotButtonClicked();
Q_SIGNALS:
    void aboutToHide();
private:
    /**
     * Show current button menu
     */
    void showMenu();
    /**
     * Show next menu if next, otherwise previous
     */
    void showLeftRightMenu(bool next);
    /**
     * Install event filter for menu and it submenus
     */
    void installEventFilterForAll(QMenu *menu, QObject *object);

    //Follow mouse position
    QTimer *m_mouseTimer;
    QGraphicsView *m_view;
    QGraphicsLinearLayout *m_layout;
    QList<MenuButton*> m_buttons;
    MenuButton *m_currentButton;
    bool m_aMenuIsVisible;
    QMenu *m_menu;
};

#endif //MENUWIDGET__H
