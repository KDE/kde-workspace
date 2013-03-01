/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef KRUNNERDIALOG_H
#define KRUNNERDIALOG_H

#include <kdeclarative.h>

#include <Plasma/Dialog>
#include <Plasma/PackageStructure>

namespace Plasma
{
    class FrameSvg;
    class Svg;
}

class InterfaceApi;
class PanelShadows;
class QDesktopWidget;
class QDeclarativeView;

class KRunnerDialog : public Plasma::Dialog
{
    Q_OBJECT

    public:
        explicit KRunnerDialog(QWidget *parent = 0, Qt::WindowFlags f =  Qt::Dialog | Qt::FramelessWindowHint);
        virtual ~KRunnerDialog();

        void setFreeFloating(bool floating);
        bool freeFloating() const;

        enum ResizeMode { NotResizing = 0, VerticalResizing, HorizontalResizing };
        ResizeMode manualResizing() const;

        void loadInterface();

    public Q_SLOTS:
        void display(const QString &term = QString(), const QString &singleRunnerId = QString());
        void clearHistory();
        void toggleConfigInterface();

    protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void leaveEvent(QEvent *e);
        void showEvent(QShowEvent *);
        void hideEvent(QHideEvent *);
        void moveEvent(QMoveEvent *);

        void positionOnScreen();

    protected Q_SLOTS:
        void timerEvent(QTimerEvent *event);

        /**
         * React to configuration being done
         */
        void configCompleted();

    private:
        bool checkCursor(const QPoint &pos);

    private Q_SLOTS:
        /**
         * React to screen changes
         */
        void screenResized(int screen);
        void screenGeometryChanged();
        void resetScreenPos();

    private:
        QPixmap *m_cachedBackground;
        QPoint m_lastPressPos;
        QPoint m_customPos;
        int m_shownOnScreen;
        qreal m_offset;
        bool m_floating : 1;
        bool m_resizing : 1;
        bool m_rightResize : 1;
        bool m_vertResize : 1;
        bool m_runningTimer : 1;
        QDesktopWidget *m_desktopWidget;
        Plasma::PackageStructure::Ptr m_interfacePackageStructure;
        InterfaceApi *m_interfaceApi;
        QDeclarativeView *m_view;
        KDeclarative m_kdeclarative;
};

#endif
