/*****************************************************************************
 *   Copyright (C) 2012 by Shaun Reich <shaun.reich@kdemail.net>              *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU General Public License as           *
 *   published by the Free Software Foundation; either version 2 of           *
 *   the License, or (at your option) any later version.                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/

#include "plasmaview.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QResizeEvent>
#include <QTimer>

#include <KCmdLineArgs>
#include <KIconLoader>

#include <Plasma/AccessManager>
#include <Plasma/AccessAppletJob>
#include <Plasma/Containment>
#include <Plasma/ContainmentActions>
#include <Plasma/Package>
#include <Plasma/Wallpaper>
#include <plasma/theme.h>

using namespace Plasma;

PlasmaView::PlasmaView(QWidget *parent)
    : QGraphicsView(parent)
    , m_formfactor(Plasma::Planar)
    , m_location(Plasma::Desktop)
    , m_containment(0)
    , m_pagerApplet(0)
    , m_tasksApplet(0)
{
//    setFrameStyle(QFrame::NoFrame);
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(cleanup()));

    Plasma::ContainmentActionsPluginsConfig containmentActionPlugins;
    containmentActionPlugins.addPlugin(Qt::NoModifier, Qt::RightButton, "contextmenu");

    m_corona.setContainmentActionsDefaults(Plasma::Containment::DesktopContainment, containmentActionPlugins);
    m_corona.setContainmentActionsDefaults(Plasma::Containment::CustomContainment, containmentActionPlugins);
    m_corona.setContainmentActionsDefaults(Plasma::Containment::PanelContainment, containmentActionPlugins);
    m_corona.setContainmentActionsDefaults(Plasma::Containment::CustomPanelContainment, containmentActionPlugins);

    setScene(&m_corona);
    connect(&m_corona, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(sceneRectChanged(QRectF)));
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_containment = m_corona.addContainment("desktop");
    connect(m_containment, SIGNAL(appletRemoved(Plasma::Applet*)), this, SLOT(appletRemoved(Plasma::Applet*)));

    m_containment->setFormFactor(m_formfactor);
    m_containment->setLocation(m_location);
    setScene(m_containment->scene());

    m_containment->addApplet("pager");
    m_containment->addApplet("tasks");

    m_containment->setMaximumSize(1300,1000);
//    m_containment->resize(size());
    //HACK fucking sizes..
    m_containment->resize(1300, 1000);
    resize(1300, 1000);
    setSceneRect(0, 0, 1300, 1000);
}

PlasmaView::~PlasmaView()
{
}

void PlasmaView::cleanup()
{
    delete m_containment;
    m_containment = 0;
}

//void PlasmaView::resizeEvent(QResizeEvent *event)
//{
//    QGraphicsView::resizeEvent(event);
//
//    if (!m_containment) {
//        return;
//    }
//
//    m_containment->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
//    m_containment->setMinimumSize(size());
//    m_containment->setMaximumSize(size());
//    m_containment->resize(size());
//    if (m_containment->layout()) {
//        return;
//    }
//
//    if (!m_applet) {
//        return;
//    }
//
//    //kDebug() << size();
//    qreal newWidth = 0;
//    qreal newHeight = 0;
//
//    if (false && m_applet->aspectRatioMode() == Plasma::KeepAspectRatio) {
//        // The applet always keeps its aspect ratio, so let's respect it.
//        qreal ratio = m_applet->size().width() / m_applet->size().height();
//        qreal widthForCurrentHeight = (qreal)size().height() * ratio;
//        if (widthForCurrentHeight > size().width()) {
//            newHeight = size().width() / ratio;
//            newWidth = newHeight * ratio;
//        } else {
//            newWidth = widthForCurrentHeight;
//            newHeight = newWidth / ratio;
//        }
//    } else {
//        newWidth = size().width();
//        newHeight = size().height();
//    }
//    QSizeF newSize(newWidth, newHeight);
//
//    // check if the rect is valid, or else it seems to try to allocate
//    // up to infinity memory in exponential increments
//    if (newSize.isValid()) {
//        m_applet->resize(QSizeF(newWidth, newHeight));
//        setSceneRect(m_applet->sceneBoundingRect());
//    }
//}
//
//void PlasmaView::appletTransformedItself()
//{
//    resize(m_applet->size().toSize());
//    setSceneRect(m_applet->sceneBoundingRect());
//}

#include "plasmaview.moc"
