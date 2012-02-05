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

#include <Plasma/Package>
#include <Plasma/Wallpaper>
#include <plasma/theme.h>

using namespace Plasma;

PlasmaView::PlasmaView(Plasma::Containment *containment, QWidget *parent)
    : Plasma::View(containment, parent)
    , m_formfactor(Plasma::Planar)
    , m_location(Plasma::Desktop)
    , m_containment(containment)
    , m_pagerApplet(0)
    , m_tasksApplet(0)
    , m_clockApplet(0)
    , m_smApplet(0)
    , m_systrayApplet(0)
    , m_kickoffApplet(0)
    , m_calendarApplet(0)
    , m_panelApplet(0)
{
    Q_ASSERT(m_containment);

//    setFrameStyle(QFrame::NoFrame);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    connect(qApp, SIGNAL(aboutToQuit()), SLOT(cleanup()));

    connect(m_containment, SIGNAL(appletRemoved(Plasma::Applet*)), this, SLOT(appletRemoved(Plasma::Applet*)));

    m_containment->setFormFactor(m_formfactor);
    m_containment->setLocation(m_location);

    //load some specific crap into the panel and size differently
    if (m_containment->pluginName() == "panel") {
        m_containment->resize(1000, 120);
        m_systrayApplet = containment->addApplet("systemtray");
        m_systrayApplet = containment->addApplet("tasks");
   //     containment->addApplet("notifier");
  //      containment->addApplet("notifications");
        containment->addApplet("showdesktop");

    } else {
        m_containment->setMaximumSize(1300,1000);

        //HACK fucking sizes..
        m_containment->resize(1300, 1000);

        resize(1300, 1000);
//        setSceneRect(0, 0, 1300, 1000);

        m_pagerApplet = m_containment->addApplet("pager");
        m_clockApplet = m_containment->addApplet("clock");
    //    m_panelApplet = m_containment->addApplet("panel");
        //m_systrayApplet = m_panelContainment->addApplet("systemtray");
        m_kickoffApplet = m_containment->addApplet("launcher");

        //FIXME: unused, the calendar applet is broken...
        // it resizes itself after containment already fixes it
        // m_calendarApplet = m_containment->addApplet("calendar");

        m_containment->addApplet("weather");
        m_containment->addApplet("sm_hdd");
        m_containment->addApplet("sm_net");
    }

    //TODO: enable widgetexplorer. i made us link to plasmagenericshell.
    // look at code from netbook shell to see how it's done, in plasmaapp.cpp
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
