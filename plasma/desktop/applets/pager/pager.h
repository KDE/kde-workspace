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
#include <Plasma/DataEngine>

#include "model.h"
#include "ui_pagerConfig.h"

class QDesktopWidget;

class KColorScheme;
class KWindowInfo;
class KCModuleProxy;

namespace Plasma
{
    class DeclarativeWidget;
    class Package;
}

class Pager : public Plasma::Applet
{
    Q_OBJECT
    Q_PROPERTY(QObject* model READ model CONSTANT)
    Q_PROPERTY(int currentDesktop READ currentDesktop WRITE setCurrentDesktop NOTIFY currentDesktopChanged)

    public:
        Pager(QObject *parent, const QVariantList &args);
        ~Pager();
        void init();
        void constraintsEvent(Plasma::Constraints);
        virtual QList<QAction*> contextualActions();

        QObject *model() const { return m_pagerModel; }
        int currentDesktop() const { return m_currentDesktop; }
        void setCurrentDesktop(int desktop);

    signals:
        void currentDesktopChanged();

    public slots:
        void recalculateGridSizes(int rows);
        void updateSizes(bool allowResize);
        void recalculateWindowRects();
        void themeRefresh();
        void configChanged();

    protected slots:
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event);
        virtual void wheelEvent(QGraphicsSceneWheelEvent *);

        void configAccepted();
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
        KColorScheme *colorScheme();
        QRect fixViewportPosition( const QRect& r );
        void createConfigurationInterface(KConfigDialog *parent);
        void updateToolTip();

    private:
        void initDeclarativeUI();
        QRectF mapToDeclarativeUI(const QRectF &rect) const;

        Plasma::Package *m_package;
        Plasma::DeclarativeWidget *m_declarativeWidget;
        VirtualDesktopModel *m_pagerModel;

        QTimer* m_timer;
        Ui::pagerConfig ui;
        enum DisplayedText
        {
            Number,
            Name,
            None
        };

        enum CurrentDesktopSelected
        {
            DoNothing,
            ShowDesktop,
            ShowDashboard
        };

        DisplayedText m_displayedText;
        CurrentDesktopSelected m_currentDesktopSelected;
        bool m_showWindowIcons;
        bool m_showOwnBackground;
        int m_rows;
        int m_columns;
        int m_desktopCount;
        int m_currentDesktop;
        QString m_currentActivity;
        bool m_desktopDown;
        qreal m_widthScaleFactor;
        qreal m_heightScaleFactor;
        QSizeF m_size;

        //list of info about animations for each desktop
        QRectF m_hoverRect;
        int m_hoverIndex;
        QList<QAction*> m_actions;
        QAction *m_addDesktopAction;
        QAction *m_removeDesktopAction;
        QList<KWindowInfo> m_windowInfo;
        KColorScheme *m_colorScheme;
        bool m_verticalFormFactor;

        // dragging of windows
        QRectF m_dragOriginal;
        QPointF m_dragOriginalPos;
        QPointF m_dragCurrentPos;
        WId m_dragId;
        int m_dragStartDesktop;
        int m_dragHighlightedDesktop;

        // desktop switching on drop event
        int m_dragSwitchDesktop;
        bool m_ignoreNextSizeConstraint;

        //embedded KCM module in the configuratoin dialog
        KCModuleProxy *m_configureDesktopsWidget;

        QDesktopWidget *m_desktopWidget;
    };

K_EXPORT_PLASMA_APPLET(pager, Pager)

#endif
