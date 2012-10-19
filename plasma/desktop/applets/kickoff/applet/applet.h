/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef APPLET_H
#define APPLET_H

// KDE

// Plasma
#include <Plasma/PopupApplet>

namespace Kickoff
{
class Launcher;
}
namespace Plasma
{
}

class LauncherApplet : public Plasma::PopupApplet
{
    Q_OBJECT
    Q_PROPERTY(bool switchTabsOnHover READ switchTabsOnHover WRITE setSwitchTabsOnHover NOTIFY switchTabsOnHoverChanged)
    Q_PROPERTY(bool showAppsByName READ showAppsByName WRITE setShowAppsByName NOTIFY showAppsByNameChanged)
    Q_PROPERTY(Location location READ plasmoidLocation NOTIFY locationChanged)
    Q_ENUMS(Location)

public:
    enum Location {
        Floating = 0, /**< Free floating. Neither geometry or z-ordering
                        is described precisely by this value. */
        Desktop,      /**< On the planar desktop layer, extending across
                        the full screen from edge to edge */
        FullScreen,   /**< Full screen */
        TopEdge,      /**< Along the top of the screen*/
        BottomEdge,   /**< Along the bottom of the screen*/
        LeftEdge,     /**< Along the left side of the screen */
        RightEdge     /**< Along the right side of the screen */
    };


    LauncherApplet(QObject *parent, const QVariantList &args);
    virtual ~LauncherApplet();

    void init();

    void constraintsEvent(Plasma::Constraints constraints);

    virtual QList<QAction*> contextualActions();

    QGraphicsWidget *graphicsWidget();

    bool switchTabsOnHover() const;
    void setSwitchTabsOnHover(bool on);

    bool showAppsByName() const;
    void setShowAppsByName(bool on);

    Location plasmoidLocation() const;

Q_SIGNALS:
    void switchTabsOnHoverChanged(bool switchTabsOnHover);
    void showAppsByNameChanged(bool showAppsByName);
    void locationChanged(Location location);

protected Q_SLOTS:
    void switchMenuStyle();
    void startMenuEditor();
    void toolTipAboutToShow();
    void configChanged();

    /**
     * Save config values stored on SimpleLauncher after a menu switch
     */
    void saveConfigurationFromSimpleLauncher(const KConfigGroup & configGroup,
                                             const KConfigGroup & globalConfigGroup);

    void configAccepted();
    //void toggleMenu();
    //void toggleMenu(bool pressed);

protected:

    void createConfigurationInterface(KConfigDialog *parent);
    void popupEvent(bool show);

private:
    friend class Kickoff::Launcher;
    class Private;
    Private * const d;
};

K_EXPORT_PLASMA_APPLET(launcher, LauncherApplet)

#endif
