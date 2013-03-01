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

#include "krunnerdialog.h"

#include <QDeclarativeContext>
#include <QDeclarativeView>
#include <QDesktopWidget>
#include <QPainter>
#include <QResizeEvent>
#include <QMouseEvent>
#ifdef Q_WS_X11
#include <QX11Info>
#endif
#include <QBitmap>
#include <QTimer>
#include <QVBoxLayout>

#include <KDebug>
#include <KWindowSystem>
#include <KPluginInfo>
#include <KStandardDirs>
#ifdef Q_WS_X11
#include <NETRootInfo>
#endif

#include <QtCore/QStringBuilder> // % operator for QString

#include <KIcon>

#include <Plasma/AbstractRunner>
#include <Plasma/FrameSvg>
#include <Plasma/Package>
#include <Plasma/Theme>
#include <Plasma/WindowEffects>

#include "kworkspace/kdisplaymanager.h"

#include "interfaceApi.h"
#include "krunnerapp.h"
#include "krunnersettings.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

KRunnerDialog::KRunnerDialog(QWidget *parent, Qt::WindowFlags f)
    : Plasma::Dialog(parent, f),
      m_shownOnScreen(-1),
      m_offset(.5),
      m_floating(!KRunnerSettings::freeFloating()),
      m_resizing(false),
      m_rightResize(false),
      m_vertResize(false),
      m_runningTimer(false),
      m_desktopWidget(qApp->desktop()),
      m_interfacePackageStructure(Plasma::PackageStructure::load("Plasma/Generic")),
      m_interfaceApi(new InterfaceApi(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    setWindowTitle(i18nc("@title:window", "Run Command"));
    setWindowIcon(KIcon(QLatin1String("system-run")));

    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);

    connect(m_desktopWidget, SIGNAL(resized(int)), this, SLOT(screenGeometryChanged()));
    connect(m_desktopWidget, SIGNAL(screenCountChanged(int)), this, SLOT(screenGeometryChanged()));

    connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SLOT(resetScreenPos()));

    KConfigGroup interfaceConfig(KGlobal::config(), "Interface");
    if (interfaceConfig.hasKey("Size")) {
        resize(interfaceConfig.readEntry("Size", size()));
    } else {
        //FIXME: sensible default size, based on the QML?
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_view = new QDeclarativeView(this);
    m_view->setAttribute(Qt::WA_OpaquePaintEvent);
    m_view->setAttribute(Qt::WA_NoSystemBackground);
    m_view->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    m_view->viewport()->setAttribute(Qt::WA_NoSystemBackground);
    m_view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    m_kdeclarative.setDeclarativeEngine(m_view->engine());
    m_kdeclarative.initialize();
    m_kdeclarative.setupBindings();
    m_view->rootContext()->setContextProperty("App", m_interfaceApi);

    layout->addWidget(m_view);
    setFreeFloating(KRunnerSettings::freeFloating());
}

KRunnerDialog::~KRunnerDialog()
{
    //kDebug( )<< "!!!!!!!!!! deleting" << m_floating << m_screenPos.count();
    if (!m_floating) {
        KConfigGroup cg(KGlobal::config(), "EdgePositions");
        cg.writeEntry(QLatin1String("Offset"), m_offset);
    }

    KConfigGroup interfaceConfig(KGlobal::config(), "Interface");
    interfaceConfig.writeEntry("Size", size());
}

void KRunnerDialog::screenResized(int screen)
{
    if (isVisible() && screen == m_shownOnScreen) {
        positionOnScreen();
    }
}

void KRunnerDialog::screenGeometryChanged()
{
    if (isVisible()) {
        positionOnScreen();
    }
}

void KRunnerDialog::resetScreenPos()
{
    if (isVisible() && !m_floating) {
        positionOnScreen();
    }
}

void KRunnerDialog::positionOnScreen()
{
    if (m_desktopWidget->screenCount() < 2) {
        m_shownOnScreen = m_desktopWidget->primaryScreen();
    } else if (isVisible()) {
        m_shownOnScreen = m_desktopWidget->screenNumber(geometry().center());
    } else {
        m_shownOnScreen = m_desktopWidget->screenNumber(QCursor::pos());
    }

    const QRect r = m_desktopWidget->screenGeometry(m_shownOnScreen);

    if (m_floating && !m_customPos.isNull()) {
        int x = qBound(r.left(), m_customPos.x(), r.right() - width());
        int y = qBound(r.top(), m_customPos.y(), r.bottom() - height());
        move(x, y);
        show();
        return;
    }

    const int w = width();
    int x = r.left() + (r.width() * m_offset) - (w / 2);

    int y = r.top();
    if (m_floating) {
        y += r.height() / 3;
    }

    x = qBound(r.left(), x, r.right() - width());
    y = qBound(r.top(), y, r.bottom() - height());

    move(x, y);
    show();

    if (m_floating) {
        KWindowSystem::setOnDesktop(winId(), KWindowSystem::currentDesktop());
        //Turn the sliding effect off
        Plasma::WindowEffects::slideWindow(this, Plasma::Floating);
    } else {
        KWindowSystem::setOnAllDesktops(winId(), true);
        Plasma::WindowEffects::slideWindow(this, Plasma::TopEdge);
    }

    KWindowSystem::forceActiveWindow(winId());
    //kDebug() << "moving to" << m_screenPos[screen];
}

void KRunnerDialog::moveEvent(QMoveEvent *e)
{
    Plasma::Dialog::moveEvent(e);

    if (m_floating) {
        m_customPos = pos();
    } else {
        const QRect screen = m_desktopWidget->screenGeometry(m_shownOnScreen);
        m_offset = qRound((geometry().center().x() - screen.x())  / qreal(screen.width()) * 100) / 100.0;
    }
}

void KRunnerDialog::setFreeFloating(bool floating)
{
    if (m_floating == floating) {
        return;
    }

    m_floating = floating;
    m_shownOnScreen = -1;
    unsetCursor();

    if (m_floating) {
        KWindowSystem::setType(winId(), NET::Normal);
    } else {
        // load the positions for each screen from our config
        KConfigGroup cg(KGlobal::config(), "EdgePositions");
        m_offset = cg.readEntry(QLatin1String("Offset"), m_offset);
        KWindowSystem::setType(winId(), NET::Dock);
    }

    if (isVisible()) {
        positionOnScreen();
    }
}

bool KRunnerDialog::freeFloating() const
{
    return m_floating;
}

KRunnerDialog::ResizeMode KRunnerDialog::manualResizing() const
{
    if (!m_resizing) {
        return NotResizing;
    } else if (m_vertResize) {
        return VerticalResizing;
    } else {
        return HorizontalResizing;
    }
}

void KRunnerDialog::clearHistory()
{
    m_interfaceApi->signalClearHistory();
}

void KRunnerDialog::display(const QString &query, const QString &singleRunnerId)
{
    m_interfaceApi->setSingleRunnerId(singleRunnerId);
    if (!query.isEmpty()) {
        m_interfaceApi->query(query);
    }

    positionOnScreen();
    KWindowSystem::forceActiveWindow(winId());
}

void KRunnerDialog::loadInterface()
{
    const QString interfaceName = KRunnerSettings::interfacePlugin();
    if (interfaceName == interfaceName) {
        return;
    }

    if (!interfaceName.isEmpty()) {
        // FIXME: remove the existing interface
        delete m_interfaceApi->package();
        m_interfaceApi->setPackage(0);
    }

    /*
       we have to check for the location ourselves anyways, so no point in using KServiceTypeTrader
       with libplasma2, this will make sense to do, however, as PackageStructure can take it
       from there

    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(interfaceName);
    KService::List offers = KServiceTypeTrader::self()->query("KRunner/Interface", constraint);

    if (offers.isEmpty()) {
        kDebug() << "Could not find requested interface:" << interfaceName;
        return;
    }
    */

    //TODO: with libplasma2, finding the package can be done by PackageStructure
    QString path = KStandardDirs::locate("appdata", "interfaces/" + interfaceName + "/metadata.desktop");
    if (path.isEmpty()) {
        kDebug() << "Could not find requested interface:" << interfaceName;
        return;
    }

    path.truncate(path.size() - QString("metadata.desktop").size());
    kDebug() << "going to try and find the package at" << path;

    Plasma::Package *package = new Plasma::Package(path, m_interfacePackageStructure);
    const QString mainScript = package->filePath("mainscript");

    if (mainScript.isEmpty()) {
        kDebug() << path << "package malformed: no mainscript";
        delete package;
        return;
    }

    kDebug() << "our interface will start with" << mainScript;
    m_interfaceApi->setPackage(package);
    m_interfaceName = interfaceName;
    m_view->setSource(mainScript);
}

void KRunnerDialog::toggleConfigInterface()
{
    //FIXME QML
}

void KRunnerDialog::configCompleted()
{
    //FIXME QML
}

void KRunnerDialog::showEvent(QShowEvent *)
{
    //FIXME QML: prep the manager
    unsigned long state = NET::SkipTaskbar | NET::KeepAbove | NET::StaysOnTop;
    if (m_floating) {
        KWindowSystem::clearState(winId(), state);
    } else {
        KWindowSystem::setState(winId(), state);
    }
    KWindowSystem::setOnAllDesktops(winId(), true);
}

void KRunnerDialog::hideEvent(QHideEvent *)
{
    //FIXME QML: un-prep the manager
}

void KRunnerDialog::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_lastPressPos = e->globalPos();

        int leftMargin, topMargin, rightMargin, bottomMargin;
        getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

        const bool leftResize = e->x() < qMax(5, leftMargin);
        m_rightResize = e->x() > width() - qMax(5, rightMargin);
        m_vertResize = e->y() > height() - qMax(5, bottomMargin);
        kWarning() << "right:" << m_rightResize << "left:" << leftResize << "vert:" << m_vertResize;
        if (m_rightResize || m_vertResize || leftResize) {
            // let's do a resize! :)
            grabMouse();
            m_resizing = true;
        } else if (m_floating) {
#ifdef Q_WS_X11
            m_lastPressPos = QPoint();
            // We have to release the mouse grab before initiating the move operation.
            // Ideally we would call releaseMouse() to do this, but when we only have an
            // implicit passive grab, Qt is unaware of it, and will refuse to release it.
            XUngrabPointer(x11Info().display(), CurrentTime);

            // Ask the window manager to start an interactive move operation.
            NETRootInfo rootInfo(x11Info().display(), NET::WMMoveResize);
            rootInfo.moveResizeRequest(winId(), e->globalX(), e->globalY(), NET::Move);
#endif
        }
        e->accept();
    }
}

void KRunnerDialog::mouseReleaseEvent(QMouseEvent *)
{
    if (!m_lastPressPos.isNull()) {
        releaseMouse();
        unsetCursor();
        m_lastPressPos = QPoint();
        m_resizing = false;
    }
}

void KRunnerDialog::leaveEvent(QEvent *)
{
    unsetCursor();
}

void KRunnerDialog::mouseMoveEvent(QMouseEvent *e)
{
    //kDebug() << e->x() << m_leftBorderWidth << width() << m_rightBorderWidth;
    if (m_lastPressPos.isNull()) {
        // not press positiong, so we aren't going to resize or move.
        checkCursor(e->pos());
    } else if (m_resizing) {
        // resizing
        if (m_vertResize) {
            const int deltaY = e->globalY() - m_lastPressPos.y();
            resize(width(), qMax(80, height() + deltaY));
            m_lastPressPos = e->globalPos();
        } else {
            const QRect r = m_desktopWidget->availableGeometry(m_shownOnScreen);
            const int deltaX = (m_rightResize ? -1 : 1) * (m_lastPressPos.x() - e->globalX());
            int newWidth = width() + deltaX;

            int leftMargin, topMargin, rightMargin, bottomMargin;
            getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

            // don't let it grow beyond the opposite screen edge
            if (m_rightResize) {
                if (leftMargin > 0) {
                    newWidth += qMin(deltaX, x() - r.left());
                }
            } else if (rightMargin > 0) {
                newWidth += qMin(deltaX, r.right() - (x() + width() - 1));
            } else if (newWidth > minimumWidth() && newWidth < width()) {
                move(r.right() - newWidth + 1, y());
            }

            if (newWidth > minimumWidth()) {
                resize(newWidth, height());
                m_lastPressPos = e->globalPos();
            }
        }
    } else {
        // moving
        const QRect r = m_desktopWidget->availableGeometry(m_shownOnScreen);
        int newX = qBound(r.left(), x() - (m_lastPressPos.x() - e->globalX()), r.right() - width() + 1);
        if (abs(r.center().x() - (newX + (width() / 2))) < 20) {
            newX = r.center().x() - (width() / 2);
        } else {
            m_lastPressPos = e->globalPos();
        }

        move(newX, y());
    }
}

void KRunnerDialog::timerEvent(QTimerEvent *event)
{
    killTimer(event->timerId());
    if (checkCursor(mapFromGlobal(QCursor::pos()))) {
        m_runningTimer = true;
        startTimer(100);
    } else {
        m_runningTimer = false;
    }
}

bool KRunnerDialog::checkCursor(const QPoint &pos)
{
    int leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

    if ((leftMargin > 0 && pos.x() < qMax(5, leftMargin)) ||
        (rightMargin > 0 && pos.x() > width() - qMax(5, rightMargin))) {
        if (cursor().shape() != Qt::SizeHorCursor) {
            setCursor(Qt::SizeHorCursor);
            if (!m_runningTimer) {
                m_runningTimer = true;
                startTimer(100);
            }
            return false;
        }

        return true;
    } else if ((pos.y() > height() - qMax(5, bottomMargin)) && (pos.y() < height())) {
        if (cursor().shape() != Qt::SizeVerCursor) {
            setCursor(Qt::SizeVerCursor);
            if (!m_runningTimer) {
                m_runningTimer = true;
                startTimer(100);
            }
            return false;
        }

        return true;
    }

    unsetCursor();
    return false;
}

#include "krunnerdialog.moc"
