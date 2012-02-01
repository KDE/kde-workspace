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

using namespace Plasma;

PlasmaView::PlasmaView(QWidget *parent)
    : QGraphicsView(parent),
      m_formfactor(Plasma::Planar),
      m_location(Plasma::Floating),
      m_containment(0),
      m_applet(0)
{
//    setFrameStyle(QFrame::NoFrame);

    Plasma::ContainmentActionsPluginsConfig containmentActionPlugins;
    containmentActionPlugins.addPlugin(Qt::NoModifier, Qt::RightButton, "contextmenu");

    m_corona.setContainmentActionsDefaults(Plasma::Containment::DesktopContainment, containmentActionPlugins);
    m_corona.setContainmentActionsDefaults(Plasma::Containment::CustomContainment, containmentActionPlugins);
    m_corona.setContainmentActionsDefaults(Plasma::Containment::PanelContainment, containmentActionPlugins);
    m_corona.setContainmentActionsDefaults(Plasma::Containment::CustomPanelContainment, containmentActionPlugins);

    setScene(&m_corona);
    connect(&m_corona, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(sceneRectChanged(QRectF)));
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  //  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    addApplet("pager", "desktop", QVariantList());
}

PlasmaView::~PlasmaView()
{
    storeCurrentApplet();
}

void PlasmaView::addApplet(const QString &name, const QString &containment,
                         const QVariantList &args)
{
    kDebug() << "adding applet" << name << "in" << containment;
    if (!m_containment || m_containment->pluginName() != containment) {
        delete m_containment;
        m_containment = m_corona.addContainment(containment);
        connect(m_containment, SIGNAL(appletRemoved(Plasma::Applet*)), this, SLOT(appletRemoved(Plasma::Applet*)));
    }

//    if (!wallpaper.isEmpty()) {
//        m_containment->setWallpaper(wallpaper);
//    }

    m_containment->setFormFactor(m_formfactor);
    m_containment->setLocation(m_location);
    m_containment->resize(size());
    setScene(m_containment->scene());

    if (name.startsWith("plasma:") || name.startsWith("zeroconf:")) {
        kDebug() << "accessing remote: " << name;
        AccessManager::self()->accessRemoteApplet(KUrl(name));
        connect(AccessManager::self(), SIGNAL(finished(Plasma::AccessAppletJob*)),
                this, SLOT(plasmoidAccessFinished(Plasma::AccessAppletJob*)));
        return;
    }

    if (m_applet) {
        // we already have an applet!
        storeCurrentApplet();
        disconnect(m_applet);
        m_applet->destroy();
    }

    QFileInfo info(name);
    if (!info.isAbsolute()) {
        info = QFileInfo(QDir::currentPath() + "/" + name);
    }

    if (info.exists()) {
        m_applet = Applet::loadPlasmoid(info.absoluteFilePath());
    }

    if (m_applet) {
        m_containment->addApplet(m_applet, QPointF(-1, -1), false);
    } else if (name.isEmpty()) {
        return;
    } else {
        m_applet = m_containment->addApplet(name, args, QRectF(0, 0, -1, -1));
    }

    if (!m_applet) {
        return;
    }

    if (hasStorageGroupFor(m_applet)) {
        KConfigGroup cg = m_applet->config();
        KConfigGroup storage = storageGroup(m_applet);
        cg.deleteGroup();
        storage.copyTo(&cg);
        m_applet->configChanged();
    }

    setSceneRect(m_applet->sceneBoundingRect());
    m_applet->setFlag(QGraphicsItem::ItemIsMovable, false);
    setWindowTitle(m_applet->name());
    setWindowIcon(SmallIcon(m_applet->icon()));
    resize(m_applet->size().toSize());
    connect(m_applet, SIGNAL(appletTransformedItself()), this, SLOT(appletTransformedItself()));
    kDebug() << "connecting ----------------";
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

void PlasmaView::sceneRectChanged(const QRectF &rect)
{
    Q_UNUSED(rect)
    if (m_applet) {
        setSceneRect(m_applet->sceneBoundingRect());
    }
}

bool PlasmaView::hasStorageGroupFor(Plasma::Applet *applet) const
{
    KConfigGroup stored = KConfigGroup(KGlobal::config(), "StoredApplets");
    return stored.groupList().contains(applet->pluginName());
}

KConfigGroup PlasmaView::storageGroup(Plasma::Applet *applet) const
{
    KConfigGroup stored = KConfigGroup(KGlobal::config(), "StoredApplets");
    return KConfigGroup(&stored, applet->pluginName());
}

void PlasmaView::storeCurrentApplet()
{
    if (m_applet) {
        KConfigGroup cg;
        m_applet->save(cg);
        cg = m_applet->config();
        KConfigGroup storage = storageGroup(m_applet);
        storage.deleteGroup();
        cg.copyTo(&storage);
        KGlobal::config()->sync();
    }
}

#include "plasmaview.moc"

