/*
*   Copyright 2007 by Matt Broadstone <mbroadst@kde.org>
*   Copyright 2007 by Robert Knight <robertknight@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2, 
*   or (at your option) any later version.
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

#ifndef PLASMA_PANELVIEW_H
#define PLASMA_PANELVIEW_H

#include <QList>

#include <KConfigGroup>

#include <plasma/plasma.h>
#include <plasma/view.h>

#ifdef Q_WS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

class QWidget;

namespace Plasma
{
    class Containment;
    class Corona;
    class Svg;
}

class PanelController;
class PanelAppletOverlay;

class PanelView : public Plasma::View
{
    Q_OBJECT
public:

   /**
    * Constructs a new panelview.
    * @arg parent the QWidget this panel is parented to
    */
    explicit PanelView(Plasma::Containment *panel, int id = 0, QWidget *parent = 0);
    ~PanelView();

    /**
     * @return the location (screen edge) where this panel is positioned.
     */
    Plasma::Location location() const;

    /**
     * @return the Corona (scene) associated with this panel.
     */
    Plasma::Corona *corona() const;

    /**
     * @return the offset of the panel from the left screen edge
     */
    int offset() const;

    /**
     * @return the panel alignment
     */
    Qt::Alignment alignment() const;

    /**
     * Pinches the min/max sizes of the containment to the current screen resolution
     */
    void pinchContainment(const QRect &screenGeometry);

public Q_SLOTS:
    /**
     * Sets the offset the left border, the offset is the distance of the left
     * border of the panel from the left border of the screen when the alignment is
     * Qt::AlignLeft, right border and right edge if the alignment is Qt::alignRight
     * and the distance between the center of the panel and the center of the screen if
     * the alignment is Qt::AlignCenter.
     * Similar way for vertical panels.
     * @param newOffset the offset of the panel
     */
    void setOffset(int newOffset);

    /**
     * Sets the edge of the screen the panel will be aligned and will grow
     * @param align the direction (for instance Qt::AlignLeft) means the panel will start
     * from the left of the screen and grow to the right
     */
    void setAlignment(Qt::Alignment align);

    /**
     * Sets the location (screen edge) where this panel is positioned.
     * @param location the location to place the panel at
     */
    void setLocation(Plasma::Location location);

protected:
    void updateStruts();
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);

private Q_SLOTS:
    void showAppletBrowser();
    void togglePanelController();
    void edittingComplete();

    /**
     * Updates the panel's position according to the screen and containment
     * dimensions
     */
    void updatePanelGeometry();

private:
    Qt::Alignment alignmentFilter(Qt::Alignment align) const;
    bool isHorizontal() const;
#ifdef Q_WS_WIN
    bool registerAccessBar(HWND hwndAccessBar, bool fRegister);
    void appBarQuerySetPos(uint uEdge, LPRECT lprc, PAPPBARDATA pabd);
    void appBarCallback(MSG *message, long *result);
    void appBarPosChanged(PAPPBARDATA pabd);
    bool winEvent(MSG *message, long *result);
    bool m_barRegistered;
#endif

    Plasma::Svg *m_background;
    PanelController *m_panelController;
    QList<PanelAppletOverlay*> m_moveOverlays;

    int m_offset;
    Qt::Alignment m_alignment;
    QSizeF m_lastMin;
    QSizeF m_lastMax;
    int m_lastSeenSize;
    bool m_lastHorizontal;
    bool m_editting;
};

#endif

