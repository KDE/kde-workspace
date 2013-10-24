/***************************************************************************
 *   Copyright (C) 2007 by Daniel Laidig <d.laidig@gmx.de>                 *
 *   Copyright (C) 2012 by Luís Gabriel Lima <lampih@gmail.com>            *
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

#ifndef PAGER_H
#define PAGER_H

#include <QGraphicsSceneHoverEvent>
#include <QList>

#include <Plasma/Applet>

#include "model.h"

class QDesktopWidget;

class KColorScheme;
class KWindowInfo;
class KCModuleProxy;

namespace Plasma
{
    class FrameSvg;
}

class Pager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* model READ model CONSTANT)
    Q_PROPERTY(int currentDesktop READ currentDesktop NOTIFY currentDesktopChanged)
    Q_PROPERTY(bool showWindowIcons READ showWindowIcons NOTIFY showWindowIconsChanged)
    Q_PROPERTY(bool showDesktopName READ showDesktopName NOTIFY showDesktopTextChanged)
    Q_PROPERTY(bool showDesktopNumber READ showDesktopNumber NOTIFY showDesktopTextChanged)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QRectF geometry READ geometry WRITE setGeometry NOTIFY geometryChanged)

    public:
        Pager(QObject *parent = 0);
        ~Pager();

        void init();
        //void constraintsEvent(Plasma::Constraints);
        virtual QList<QAction*> contextualActions();

        QObject *model() const { return m_pagerModel; }

        int currentDesktop() const { return m_currentDesktop; }
        void setCurrentDesktop(int desktop);

        bool showWindowIcons() const { return m_showWindowIcons; }
        void setShowWindowIcons(bool show);

        bool showDesktopName() const { return m_displayedText == Name; }
        bool showDesktopNumber() const { return m_displayedText == Number; }

        Qt::Orientation orientation() const;
        void setOrientation(Qt::Orientation orientation);

        QRectF geometry() const;
        void setGeometry(const QRectF &geom);

        Q_INVOKABLE void moveWindow(int, double, double, int, int);
        Q_INVOKABLE void changeDesktop(int desktopId);

    Q_SIGNALS:
        void currentDesktopChanged();
        void showWindowIconsChanged();
        void showDesktopTextChanged();
        void orientationChanged();
        void geometryChanged();

    public Q_SLOTS:
        void recalculateGridSizes(int rows);
        void updateSizes(bool allowResize = true);
        void recalculateWindowRects();

    protected Q_SLOTS:
        void currentDesktopChanged(int desktop);
        void currentActivityChanged(const QString &activity);
        void desktopsSizeChanged();
        void numberOfDesktopsChanged(int num);
        void desktopNamesChanged();
        void windowChanged(WId id, const unsigned long *dirty);
        void startTimer();
        void startTimerFast();
#ifdef Q_WS_X11
        void slotAddDesktop();
        void slotRemoveDesktop();
#endif

    protected:
        void createMenu();
        QRect fixViewportPosition( const QRect& r );

    private:

        PagerModel *m_pagerModel;

        QTimer* m_timer;
        enum DisplayedText {
            Number,
            Name,
            None
        };

        enum CurrentDesktopSelected {
            DoNothing,
            ShowDesktop,
            ShowDashboard
        };

        DisplayedText m_displayedText;
        CurrentDesktopSelected m_currentDesktopSelected;
        int m_rows;
        int m_columns;
        int m_desktopCount;
        int m_currentDesktop;
        QString m_currentActivity;
        qreal m_widthScaleFactor;
        qreal m_heightScaleFactor;
        QSizeF m_size;
        Qt::Orientation m_orientation;
        QRectF m_geometry;

        //list of info about animations for each desktop
        QList<QAction*> m_actions;
        QAction *m_addDesktopAction;
        QAction *m_removeDesktopAction;

        bool m_showWindowIcons;
        bool m_desktopDown;
        bool m_ignoreNextSizeConstraint;
        bool m_hideWhenSingleDesktop;

        QDesktopWidget *m_desktopWidget;
    };

#endif
