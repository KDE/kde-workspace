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

#include "pager.h"

#include <math.h>

#include <QApplication>
#include <QFont>
#include <QTimer>
#include <QX11Info>
#include <QDBusInterface>
#include <QTextDocument>
#include <QDesktopWidget>

#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KWindowSystem>
#include <NETRootInfo>

#include <Plasma/FrameSvg>
#include <Plasma/Package>

#include <KActivities/Consumer>

#include <taskmanager/task.h>

const int FAST_UPDATE_DELAY = 100;
const int UPDATE_DELAY = 500;
const int MAXDESKTOPS = 20;
// random(), find a less magic one if you can. -sreich
const qreal MAX_TEXT_WIDTH = 800;

Pager::Pager(QObject *parent)
    : QObject(parent),
      m_displayedText(None),
      m_currentDesktopSelected(DoNothing),
      m_rows(2),
      m_columns(0),
      m_currentDesktop(0),
      m_orientation(Qt::Horizontal),
      m_addDesktopAction(0),
      m_removeDesktopAction(0),
      m_showWindowIcons(false),
      m_desktopDown(false),
      m_desktopWidget(QApplication::desktop())
{
    // initialize with a decent default
    m_desktopCount = KWindowSystem::numberOfDesktops();
    
    m_pagerModel = new PagerModel(this);

    createMenu();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(recalculateWindowRects()));

    connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(currentDesktopChanged(int)));
    connect(KWindowSystem::self(), SIGNAL(windowAdded(WId)), this, SLOT(startTimerFast()));
    connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(startTimerFast()));
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(startTimerFast()));
    connect(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)), this, SLOT(numberOfDesktopsChanged(int)));
    connect(KWindowSystem::self(), SIGNAL(desktopNamesChanged()), this, SLOT(desktopNamesChanged()));
    connect(KWindowSystem::self(), SIGNAL(stackingOrderChanged()), this, SLOT(startTimerFast()));
    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId,const ulong*)), this, SLOT(windowChanged(WId,const ulong*)));
    connect(KWindowSystem::self(), SIGNAL(showingDesktopChanged(bool)), this, SLOT(startTimer()));
    connect(m_desktopWidget, SIGNAL(screenCountChanged(int)), SLOT(desktopsSizeChanged()));
    connect(m_desktopWidget, SIGNAL(resized(int)), SLOT(desktopsSizeChanged()));

    // connect to KWin's reloadConfig signal to get updates on the desktop layout
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect(QString(), "/KWin", "org.kde.KWin", "reloadConfig",
                 this, SLOT(configChanged()));

    recalculateGridSizes(m_rows);

    setCurrentDesktop(KWindowSystem::currentDesktop());

    KActivities::Consumer *act = new KActivities::Consumer(this);
    connect(act, SIGNAL(currentActivityChanged(QString)), this, SLOT(currentActivityChanged(QString)));
    m_currentActivity = act->currentActivity();
}

Pager::~Pager()
{
}



void Pager::setCurrentDesktop(int desktop)
{
    if (m_currentDesktop != desktop) {
        m_currentDesktop = desktop;
        emit currentDesktopChanged();
    }
}

void Pager::setShowWindowIcons(bool show)
{
    if (m_showWindowIcons != show) {
        m_showWindowIcons = show;
        emit showWindowIconsChanged();
    }
}

Qt::Orientation Pager::orientation() const
{
    return m_orientation;
}

void Pager::setOrientation(Qt::Orientation orientation)
{
    if (m_orientation == orientation) {
        return;
    }

    m_orientation = orientation;
    emit orientationChanged();
 
    // whenever we switch to/from vertical form factor, swap the rows and columns around
    if (m_columns != m_rows) {
        // pass in columns as the new rows
        recalculateGridSizes(m_columns);
        recalculateWindowRects();
    }
}

QSizeF Pager::size() const
{
    return m_size;
}

void Pager::setSize(const QSizeF &size)
{
    if (m_size == size) {
        return;
    }

    m_size = size;
    emit sizeChanged();
    
    // no need to update everything twice (if we are going to flip rows and columns later)
    if (m_columns == m_rows) {
        updateSizes();

        recalculateWindowRects();
    }
}

Pager::CurrentDesktopSelected Pager::currentDesktopSelected() const
{
    return m_currentDesktopSelected;
}

void Pager::setCurrentDesktopSelected(CurrentDesktopSelected cur)
{
    if (m_currentDesktopSelected == cur) {
        return;
    }

    m_currentDesktopSelected = cur;
    emit currentDesktopSelectedChanged();
}

Pager::DisplayedText Pager::displayedText() const
{
    return m_displayedText;
}

void Pager::setDisplayedText(Pager::DisplayedText disp)
{
    if (m_displayedText == disp) {
        return;
    }

    m_displayedText = disp;
    emit displayedTextChanged();
}

void Pager::createMenu()
{
#ifdef Q_WS_X11
    m_addDesktopAction = new QAction(SmallIcon("list-add"),i18n("&Add Virtual Desktop"), this);
    m_actions.append(m_addDesktopAction);
    connect(m_addDesktopAction, SIGNAL(triggered(bool)), this , SLOT(slotAddDesktop()));
    m_removeDesktopAction = new QAction(SmallIcon("list-remove"),i18n("&Remove Last Virtual Desktop"), this);
    m_actions.append(m_removeDesktopAction);
    connect(m_removeDesktopAction, SIGNAL(triggered(bool)), this , SLOT(slotRemoveDesktop()));

    if (m_desktopCount <= 1) {
        m_removeDesktopAction->setEnabled(false);
    } else if (m_desktopCount >= MAXDESKTOPS) {
        m_addDesktopAction->setEnabled(false);
    }
#endif
}

QList<QAction*> Pager::contextualActions()
{
  return m_actions;
}

#ifdef Q_WS_X11
void Pager::slotAddDesktop()
{
    NETRootInfo info(QX11Info::display(), NET::NumberOfDesktops);
    info.setNumberOfDesktops(info.numberOfDesktops() + 1);
}

void Pager::slotRemoveDesktop()
{
    NETRootInfo info(QX11Info::display(), NET::NumberOfDesktops);
    int desktops = info.numberOfDesktops();
    if (desktops > 1) {
        info.setNumberOfDesktops(info.numberOfDesktops() - 1);
    }
}
#endif

void Pager::recalculateGridSizes(int rows)
{
    // recalculate the number of rows and columns in the grid
    rows = qBound(1, rows, m_desktopCount);
    // avoid weird cases like having 3 rows for 4 desktops, where the last row is unused
    int columns = m_desktopCount / rows;
    if (m_desktopCount % rows > 0) {
        columns++;
    }

    rows = m_desktopCount / columns;
    if (m_desktopCount % columns > 0) {
        rows++;
    }

    // update the grid size
    m_rows = rows;
    m_columns = columns;

    updateSizes();
}

void Pager::updateSizes()
{
    int padding = 2; // Space between miniatures of desktops
    int textMargin = 3; // Space between name of desktop and border

    qreal leftMargin = 0;
    qreal topMargin = 0;
    qreal rightMargin = 0;
    qreal bottomMargin = 0;

    QRect totalRect;
    for (int x = 0; x < m_desktopWidget->screenCount(); x++) {
        totalRect |= m_desktopWidget->screenGeometry(x);
    }

    qreal ratio = (qreal) totalRect.width() /
                  (qreal) totalRect.height();


    qreal itemHeight;
    qreal itemWidth;
    qreal preferredItemHeight;
    qreal preferredItemWidth;

    if (orientation() == Qt::Vertical) {
        // work out the preferred size based on the width of the geometry
        preferredItemWidth = (m_size.width() - leftMargin - rightMargin -
                              padding * (m_columns - 1)) / m_columns;
        preferredItemHeight = preferredItemWidth / ratio;
        // make sure items of the new size actually fit in the current geometry
        itemHeight = (m_size.height() - topMargin - bottomMargin -
                      padding * (m_rows - 1)) / m_rows;
        if (itemHeight > preferredItemHeight) {
            itemHeight = preferredItemHeight;
        }
        itemWidth = itemHeight * ratio;
    } else {
        // work out the preferred size based on the height of the geometry
        preferredItemHeight = (m_size.height() - topMargin - bottomMargin -
                               padding * (m_rows - 1)) / m_rows;
        preferredItemWidth = preferredItemHeight * ratio;

        if (m_displayedText == Name) {
            // When containment is in this position we are not limited by low width and we can
            // afford increasing width of applet to be able to display every name of desktops
            for (int i = 0; i < m_desktopCount; i++) {
                QFontMetricsF metrics(KGlobalSettings::taskbarFont());
                QSizeF textSize = metrics.size(Qt::TextSingleLine, KWindowSystem::desktopName(i+1));
                if (textSize.width() + textMargin * 2 > preferredItemWidth) {
                     preferredItemWidth = textSize.width() + textMargin * 2;
                }
            }
        }

        itemWidth = (m_size.width() - leftMargin - rightMargin -
                     padding * (m_columns - 1)) / m_columns;
        if (itemWidth > preferredItemWidth) {
            itemWidth = preferredItemWidth;
        }
        itemHeight = preferredItemHeight;
        if (itemWidth < itemHeight * ratio) {
            itemHeight = itemWidth / ratio;
        }
    }

    m_widthScaleFactor = itemWidth / totalRect.width();
    m_heightScaleFactor = itemHeight / totalRect.height();

    m_pagerModel->clearDesktopRects();

    QRectF itemRect(QPointF(leftMargin, topMargin) , QSizeF(itemWidth, itemHeight));
    for (int i = 0; i < m_desktopCount; i++) {
        itemRect.moveLeft(leftMargin + (i % m_columns)  * (itemWidth + padding));
        itemRect.moveTop(topMargin + (i / m_columns) * (itemHeight + padding));

        QString name = KWindowSystem::desktopName(i + 1);
        m_pagerModel->appendDesktopRect(itemRect, name);
    }
}

void Pager::recalculateWindowRects()
{
    QList<WId> windows = KWindowSystem::stackingOrder();
    m_pagerModel->clearWindowRects();

    foreach (WId window, windows) {
        KWindowInfo info = KWindowSystem::windowInfo(window, NET::WMGeometry | NET::WMFrameExtents |
                                                             NET::WMWindowType | NET::WMDesktop |
                                                             NET::WMState | NET::XAWMState | NET::WMVisibleName);
        NET::WindowType type = info.windowType(NET::NormalMask | NET::DialogMask | NET::OverrideMask |
                                               NET::UtilityMask | NET::DesktopMask | NET::DockMask |
                                               NET::TopMenuMask | NET::SplashMask | NET::ToolbarMask |
                                               NET::MenuMask);

        // the reason we don't check for -1 or Net::Unknown here is that legitimate windows, such
        // as some java application windows, may not have a type set for them.
        // apparently sane defaults on properties is beyond the wisdom of x11.
        if (type == NET::Desktop || type == NET::Dock || type == NET::TopMenu ||
            type == NET::Splash || type == NET::Menu || type == NET::Toolbar ||
            info.hasState(NET::SkipPager) || info.isMinimized()) {
            continue;
        }

        //check activity
        unsigned long properties[] = { 0, NET::WM2Activities };
        NETWinInfo netInfo(QX11Info::display(), window, QX11Info::appRootWindow(), properties, 2);
        QString result(netInfo.activities());
        if (!result.isEmpty() && result != "00000000-0000-0000-0000-000000000000") {
            QStringList activities = result.split(',');
            if (!activities.contains(m_currentActivity)) {
                continue;
            }
        }

        for (int i = 0; i < m_desktopCount; i++) {
            if (!info.isOnDesktop(i+1)) {
                continue;
            }

            QRectF windowRect = info.frameGeometry();

            if (KWindowSystem::mapViewport()) {
                windowRect = fixViewportPosition(windowRect.toRect());
            }

            windowRect = QRectF(windowRect.x() * m_widthScaleFactor,
                                windowRect.y() * m_heightScaleFactor,
                                windowRect.width() * m_widthScaleFactor,
                                windowRect.height() * m_heightScaleFactor).toRect();

            bool active = (window == KWindowSystem::activeWindow());
            int windowIconSize = KIconLoader::global()->currentSize(KIconLoader::Small);
            int windowRectSize = qMin(windowRect.width(), windowRect.height());
            windowIconSize = qMax(windowIconSize, windowRectSize / 2);
            QPixmap icon = KWindowSystem::icon(info.win(), windowIconSize, windowIconSize, true);
            m_pagerModel->appendWindowRect(i, window, windowRect, active, icon, info.visibleName());
        }
    }
}

void Pager::currentDesktopChanged(int desktop)
{
    if (desktop < 1) {
        return; // bogus value, don't accept it
    }

    setCurrentDesktop(desktop);
    m_desktopDown = false;
    startTimerFast();
}

void Pager::currentActivityChanged(const QString &activity)
{
    m_currentActivity = activity;
    startTimerFast();
}

void Pager::numberOfDesktopsChanged(int num)
{
    if (num < 1) {
        return; // refuse to update to zero desktops
    }

#ifdef Q_WS_X11
    m_removeDesktopAction->setEnabled(num > 1);
    m_addDesktopAction->setEnabled(num < MAXDESKTOPS);
#endif

    m_desktopCount = num;

    m_pagerModel->clearDesktopRects();
    recalculateGridSizes(m_rows);
    recalculateWindowRects();
}

void Pager::desktopNamesChanged()
{
    m_pagerModel->clearDesktopRects();
    updateSizes();
    startTimer();
}

void Pager::windowChanged(WId id, const unsigned long* dirty)
{
    Q_UNUSED(id)

    if (dirty[NETWinInfo::PROTOCOLS] & (NET::WMGeometry | NET::WMDesktop) ||
        dirty[NETWinInfo::PROTOCOLS2] & NET::WM2Activities) {
        startTimer();
    }
}

void Pager::desktopsSizeChanged()
{
    m_pagerModel->clearDesktopRects();
    updateSizes();
    startTimer();
}

void Pager::startTimer()
{
    if (!m_timer->isActive()) {
        m_timer->start(UPDATE_DELAY);
    }
}

void Pager::startTimerFast()
{
    if (!m_timer->isActive()) {
        m_timer->start(FAST_UPDATE_DELAY);
    }
}

void Pager::moveWindow(int window, double x, double y, int targetDesktop, int sourceDesktop)
{
    WId windowId = (WId) window;

    QPointF dest = QPointF(x, y) - m_pagerModel->desktopRectAt(targetDesktop).topLeft();

    dest = QPointF(dest.x()/m_widthScaleFactor, dest.y()/m_heightScaleFactor);

    // don't move windows to negative positions
    dest = QPointF(qMax(dest.x(), qreal(0.0)), qMax(dest.y(), qreal(0.0)));

    // use _NET_MOVERESIZE_WINDOW rather than plain move, so that the WM knows this is a pager request
    NETRootInfo info(QX11Info::display(), 0);
    int flags = (0x20 << 12) | (0x03 << 8) | 1; // from tool, x/y, northwest gravity

    if (!KWindowSystem::mapViewport()) {
        KWindowInfo windowInfo = KWindowSystem::windowInfo(windowId, NET::WMDesktop | NET::WMState);

        if (!windowInfo.onAllDesktops()) {
            KWindowSystem::setOnDesktop(windowId, targetDesktop+1);
        }

        // only move the window if it is not full screen and if it is kept within the same desktop
        // moving when dropping between desktop is too annoying due to the small drop area.
        if (!(windowInfo.state() & NET::FullScreen) &&
            (targetDesktop == sourceDesktop || windowInfo.onAllDesktops())) {
            QPoint d = dest.toPoint();
            info.moveResizeWindowRequest(windowId, flags, d.x(), d.y(), 0, 0);
        }
    } else {
        // setOnDesktop() with viewports is also moving a window, and since it takes a moment
        // for the WM to do the move, there's a race condition with figuring out how much to move,
        // so do it only as one move
        dest += KWindowSystem::desktopToViewport(targetDesktop+1, false);
        QPoint d = KWindowSystem::constrainViewportRelativePosition(dest.toPoint());
        info.moveResizeWindowRequest(windowId, flags, d.x(), d.y(), 0, 0);
    }
    m_timer->start();
}

void Pager::changeDesktop(int newDesktop)
{
    if (m_currentDesktop == newDesktop+1) {
        // toogle the desktop or the dashboard
        if (m_currentDesktopSelected == ShowDesktop) {
            NETRootInfo info(QX11Info::display(), 0);
            m_desktopDown = !m_desktopDown;
            info.setShowingDesktop(m_desktopDown);
        } else if (m_currentDesktopSelected == ShowDashboard) {
            QDBusInterface plasmaApp("org.kde.plasma-desktop", "/App");
            plasmaApp.call("toggleDashboard");
        }
    } else {
        KWindowSystem::setCurrentDesktop(newDesktop + 1);
        setCurrentDesktop(newDesktop + 1);
    }
}

// KWindowSystem does not translate position when mapping viewports
// to virtual desktops (it'd probably break more things than fix),
// so the offscreen coordinates need to be fixed
QRect Pager::fixViewportPosition( const QRect& r )
{
    QRect desktopGeom = m_desktopWidget->geometry();
    int x = r.center().x() % desktopGeom.width();
    int y = r.center().y() % desktopGeom.height();
    if( x < 0 ) {
        x = x + desktopGeom.width();
    }
    if( y < 0 ) {
        y = y + desktopGeom.height();
    }
    return QRect( x - r.width() / 2, y - r.height() / 2, r.width(), r.height());
}



#include "moc_pager.cpp"
