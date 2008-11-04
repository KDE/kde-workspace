/*
*   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
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

#include "panel.h"

#include <limits>

#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QBitmap>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QAction>
#include <QGraphicsLayout>
#include <QGraphicsSceneDragDropEvent>


#include <KDebug>
#include <KIcon>
#include <KDialog>
#include <KIntNumInput>
#include <KMessageBox>

#include <Plasma/Corona>
#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <Plasma/View>
#include <Plasma/PaintUtils>

using namespace Plasma;

class Spacer : public QGraphicsWidget
{
public:
    Spacer(QGraphicsWidget *parent)
         : QGraphicsWidget(parent)
    {
        setAcceptDrops(true);
    }

    ~Spacer()
    {}

    Panel *panel;

protected:
    void dropEvent(QGraphicsSceneDragDropEvent *event)
    {
        event->setPos(mapToParent(event->pos()));
        panel->dropEvent(event);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0)
    {
        Q_UNUSED(option)
        Q_UNUSED(widget)

        //TODO: make this a pretty gradient?
        painter->setRenderHint(QPainter::Antialiasing);
        QPainterPath p = Plasma::PaintUtils::roundedRectangle(contentsRect().adjusted(1, 1, -2, -2), 4);
        QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        c.setAlphaF(0.3);

        painter->fillPath(p, c);
    }
};

Panel::Panel(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_configureAction(0),
      m_addPanelAction(0),
      m_currentSize(QSize(QApplication::desktop()->screenGeometry(screen()).width(), 38)),
      m_lastViewGeom(),
      m_spacerIndex(-1),
      m_spacer(0)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/panel-background");
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(backgroundChanged()));
    setZValue(150);
    setContainmentType(Containment::PanelContainment);
    resize(m_currentSize);
    setMinimumSize(m_currentSize);
    setMaximumSize(m_currentSize);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));
}

Panel::~Panel()
{
}

void Panel::init()
{
    Containment::init();
    //FIXME: This should be enabled, but in that case proxywidgets won't get rendered
    //setFlag(ItemClipsChildrenToShape, true);

    KConfigGroup cg = config("Configuration");
    setMinimumSize(cg.readEntry("minimumSize", m_currentSize));
    setMaximumSize(cg.readEntry("maximumSize", m_currentSize));
    setDrawWallpaper(false);
}

QList<QAction*> Panel::contextualActions()
{
    if (!m_configureAction) {
        m_configureAction = new QAction(i18n("Panel Settings"), this);
        m_configureAction->setIcon(KIcon("configure"));
        connect(m_configureAction, SIGNAL(triggered()), this, SIGNAL(toolBoxToggled()));

        m_addPanelAction = new QAction(i18n("Add Panel"), this);
        connect(m_addPanelAction, SIGNAL(triggered(bool)), this, SLOT(addPanel()));
        m_addPanelAction->setIcon(KIcon("list-add"));
        constraintsEvent(Plasma::ImmutableConstraint);
    }

    QList<QAction*> actions;
    actions << action("add widgets") << m_addPanelAction << action("lock widgets") << m_configureAction << action("remove");
    return actions;
}

void Panel::backgroundChanged()
{
    constraintsEvent(Plasma::LocationConstraint);
}

void Panel::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    // this gets called whenever an applet is added, and we add it to our layout
    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout*>(layout());

    if (!lay) {
        return;
    }

    Plasma::FormFactor f = formFactor();
    int insertIndex = -1;

    //Enlarge the panel if possible
    if (f == Plasma::Horizontal) {
        resize(size().width() + applet->preferredWidth(), size().height());
    } else {
        resize(size().width(), size().height() + applet->preferredHeight());
    }
    layout()->setMaximumSize(size());

    //if pos is (-1,-1) insert at the end of the panel
    if (pos != QPoint(-1, -1)) {
        for (int i = 0; i < lay->count(); ++i) {
            QRectF siblingGeometry = lay->itemAt(i)->geometry();
            if (f == Plasma::Horizontal) {
                qreal middle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
                if (pos.x() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.x() <= siblingGeometry.right()) {
                    insertIndex = i + 1;
                    break;
                }
            } else { // Plasma::Vertical
                qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
                if (pos.y() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.y() <= siblingGeometry.bottom()) {
                    insertIndex = i + 1;
                    break;
                }
            }
        }
    }

    if (insertIndex == -1) {
        lay->addItem(applet);
    } else {
        lay->insertItem(insertIndex, applet);
    }

    connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
}

void Panel::appletRemoved(Plasma::Applet* applet)
{
    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout*>(layout());
    lay->removeItem(applet);

    //shrink the panel if possible
    if (formFactor() == Plasma::Horizontal) {
        resize(size().width() - applet->size().width(), size().height());
    } else {
        resize(size().width(), size().height() - applet->size().height());
    }
    layout()->setMaximumSize(size());
}

void Panel::updateSize()
{
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(sender());

    if (applet) {
        if (formFactor() == Plasma::Horizontal) {
            const int delta = applet->preferredWidth() - applet->size().width();
            //setting the preferred width when delta = 0 and preferredWidth() < minimumWidth()
            // leads to the same thing as setPreferredWidth(minimumWidth())
            if (delta != 0) {
                setPreferredWidth(preferredWidth() + delta);
            }
        } else if (formFactor() == Plasma::Vertical) {
            const int delta = applet->preferredHeight() - applet->size().height();
            if (delta != 0) {
                setPreferredHeight(preferredHeight() + delta);
            }
        }

        resize(preferredSize());
    }
}

void Panel::addPanel()
{
    if (corona()) {
        // make a panel at the top
        Containment* panel = corona()->addContainment("panel");
        panel->showConfigurationInterface();

        panel->setScreen(screen());
        panel->setLocation(Plasma::TopEdge);

        // trigger an instant layout so we immediately have a proper geometry
        // rather than waiting around for the event loop
        panel->updateConstraints(Plasma::StartupCompletedConstraint);
        panel->flushPendingConstraintsEvents();
    }
}

void Panel::updateBorders(const QRect &geom)
{
    Plasma::Location loc = location();
    FrameSvg::EnabledBorders enabledBorders = FrameSvg::AllBorders;

    int s = screen();
    //kDebug() << loc << s << formFactor() << geometry();

    qreal topHeight = m_background->marginSize(Plasma::TopMargin);
    qreal bottomHeight = m_background->marginSize(Plasma::BottomMargin);
    qreal leftWidth = m_background->marginSize(Plasma::LeftMargin);
    qreal rightWidth = m_background->marginSize(Plasma::RightMargin);

    //remove unwanted borders
    if (s < 0) {
        // do nothing in this case, we want all the borders
    } else if (loc == BottomEdge || loc == TopEdge) {
        QRect r = QApplication::desktop()->screenGeometry(s);

        if (loc == BottomEdge) {
            enabledBorders ^= FrameSvg::BottomBorder;
            bottomHeight = 0;
        } else {
            enabledBorders ^= FrameSvg::TopBorder;
            topHeight = 0;
        }

        if (geom.x() <= r.x()) {
            enabledBorders ^= FrameSvg::LeftBorder;
            leftWidth = 0;
        }
        if (geom.right() >= r.right()) {
            enabledBorders ^= FrameSvg::RightBorder;
            rightWidth = 0;
        }

        //kDebug() << "top/bottom: Width:" << width << ", height:" << height;
    } else if (loc == LeftEdge || loc == RightEdge) {
        QRect r = QApplication::desktop()->screenGeometry(s);

        if (loc == RightEdge) {
            enabledBorders ^= FrameSvg::RightBorder;
            rightWidth = 0;
        } else {
            enabledBorders ^= FrameSvg::LeftBorder;
            leftWidth = 0;
        }
        if (geom.y() <= r.y()) {
            enabledBorders ^= FrameSvg::TopBorder;
            topHeight = 0;
        }
        if (geom.bottom() >= r.bottom()) {
            enabledBorders ^= FrameSvg::BottomBorder;
            bottomHeight = 0;
        }

        //kDebug() << "left/right: Width:" << width << ", height:" << height;
    } else {
        kDebug() << "no location!?";
    }

    //activate borders and fetch sizes again
    m_background->setEnabledBorders(enabledBorders);
    m_background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    //calculation of extra margins has to be done after getMargins
    if (formFactor() == Vertical) {
        //hardcoded extra margin for the toolbox right now
        if (immutability() == Mutable) {
            bottomHeight += 20;
        }
    //Default to horizontal for now
    } else {
        //hardcoded extra margin for the toolbox for now
        if (immutability() == Mutable) {
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                leftWidth += 20;
            } else {
                rightWidth += 20;
            }
        }
    }


    //invalidate the layout and set again
    if (layout()) {
        layout()->setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);
        layout()->invalidate();
    }

    update();
}

void Panel::constraintsEvent(Plasma::Constraints constraints)
{
    kDebug() << "constraints updated with" << constraints << "!!!!!!";

    if (constraints & Plasma::FormFactorConstraint) {
        Plasma::FormFactor form = formFactor();
        Qt::Orientation layoutDirection = form == Plasma::Vertical ? Qt::Vertical : Qt::Horizontal;
        // create our layout!
        if (layout()) {
            QGraphicsLayout *lay = layout();
            QGraphicsLinearLayout * linearLay = dynamic_cast<QGraphicsLinearLayout *>(lay);
            if (linearLay) {
                linearLay->setOrientation(layoutDirection);
            }
        } else {
            QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(this);
            lay->setOrientation(layoutDirection);
            lay->setContentsMargins(0, 0, 0, 0);
            lay->setSpacing(4);
            lay->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
            setLayout(lay);
            updateBorders(geometry().toRect());

            foreach (Applet *applet, applets()) {
                lay->addItem(applet);
            }
        }
    }

    //we need to know if the width or height is 100%
    if (constraints & Plasma::LocationConstraint || constraints & Plasma::SizeConstraint) {
        m_currentSize = geometry().size().toSize();
        QRectF screenRect = screen() >= 0 ? QApplication::desktop()->screenGeometry(screen()) :
            geometry();

        if ((formFactor() == Horizontal && m_currentSize.width() >= screenRect.width()) ||
                (formFactor() == Vertical && m_currentSize.height() >= screenRect.height())) {
            m_background->setElementPrefix(location());
        } else {
            m_background->setElementPrefix(QString());
        }

        m_background->resizeFrame(m_currentSize);
    }

    //FIXME: this seems the only way to correctly resize the layout the first time when the
    // saved panel size is less than the default is to setting a maximum size.
    // this shouldn't happen. maybe even a qgraphicslayout bug?
    if (layout() && (constraints & Plasma::SizeConstraint)) {
        layout()->setMaximumSize(size());
    }

    if (constraints & Plasma::LocationConstraint) {
        setFormFactorFromLocation(location());
    }

    if (constraints & Plasma::ImmutableConstraint) {
        bool unlocked = immutability() == Plasma::Mutable;

        if (m_addPanelAction) {
            m_addPanelAction->setEnabled(unlocked);
            m_addPanelAction->setVisible(unlocked);
        }

        if (m_configureAction) {
            m_configureAction->setEnabled(unlocked);
            m_configureAction->setVisible(unlocked);
        }

        QGraphicsView *panelView = view();
        if (panelView) {
            updateBorders(panelView->geometry());
        }
    }
}

void Panel::saveState(KConfigGroup &config) const
{
    config.writeEntry("minimumSize", minimumSize());
    config.writeEntry("maximumSize", maximumSize());
}

void Panel::themeUpdated()
{
    //if the theme is changed all the calculations needs to be done again
    //and resize based on the change in the theme bordersize

    qreal oldLeftWidth;
    qreal newLeftWidth;
    qreal oldTopHeight;
    qreal newTopHeight;
    qreal oldRightWidth;
    qreal newRightWidth;
    qreal oldBottomHeight;
    qreal newBottomHeight;

    layout()->getContentsMargins(&oldLeftWidth, &oldTopHeight, &oldRightWidth, &oldBottomHeight);
    m_background->getMargins(newLeftWidth, newTopHeight, newRightWidth, newBottomHeight);

    QSize newSize(size().width()-(oldLeftWidth - newLeftWidth)-(oldRightWidth - newRightWidth),
           size().height()-(oldTopHeight - newTopHeight)-(oldBottomHeight - newBottomHeight));

    resize(newSize);

    if (formFactor() == Plasma::Vertical) {
        setMaximumWidth(newSize.width());
        setMinimumWidth(newSize.width());
    } else {
        setMaximumHeight(newSize.height());
        setMinimumHeight(newSize.height());
    }

    updateBorders(geometry().toRect());
}

void Panel::paintInterface(QPainter *painter,
                           const QStyleOptionGraphicsItem *option,
                           const QRect& contentsRect)
{
    Q_UNUSED(contentsRect)
    //FIXME: this background drawing is bad and ugly =)
    // draw the background untransformed (saves lots of per-pixel-math)
    painter->save();
    painter->resetTransform();

    const Containment::StyleOption *containmentOpt = qstyleoption_cast<const Containment::StyleOption *>(option);

    QRect viewGeom;
    if (containmentOpt) {
        viewGeom = containmentOpt->view->geometry();
    }

    if (viewGeom != m_lastViewGeom) {
        m_lastViewGeom = viewGeom;
        updateBorders(viewGeom);
        if (containmentOpt && containmentOpt->view) {
            containmentOpt->view->setMask(m_background->mask());
        }
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->setRenderHint(QPainter::Antialiasing);

    m_background->paintFrame(painter, option->exposedRect);

    // restore transformation and composition mode
    painter->restore();
}

void Panel::setFormFactorFromLocation(Plasma::Location loc) {
    switch (loc) {
        case BottomEdge:
        case TopEdge:
            //kDebug() << "setting horizontal form factor";
            setFormFactor(Plasma::Horizontal);
            break;
        case RightEdge:
        case LeftEdge:
            //kDebug() << "setting vertical form factor";
            setFormFactor(Plasma::Vertical);
            break;
        case Floating:
            //TODO: implement a form factor for floating panels
            kDebug() << "Floating is unimplemented.";
            break;
        default:
            kDebug() << "invalid location!!";
    }
}

void Panel::showDropZone(const QPoint pos)
{
    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout*>(layout());

    if (!lay) {
        return;
    }

    if (m_spacer && pos == QPoint()) {
        lay->removeItem(m_spacer);
        m_spacer->hide();
        return;
    }

    //lucky case: the spacer is already in the right position
    if (m_spacer && m_spacer->geometry().contains(pos)) {
        return;
    }

    Plasma::FormFactor f = formFactor();
    int insertIndex = -1;

    //FIXME: needed in two places, make it a function?
    for (int i = 0; i < lay->count(); ++i) {
        QRectF siblingGeometry = lay->itemAt(i)->geometry();

        if (f == Plasma::Horizontal) {
            qreal middle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
            if (pos.x() < middle) {
                insertIndex = i;
                break;
            } else if (pos.x() <= siblingGeometry.right()) {
                insertIndex = i + 1;
                break;
            }
        } else { // Plasma::Vertical
            qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
            if (pos.y() < middle) {
                insertIndex = i;
                break;
            } else if (pos.y() <= siblingGeometry.bottom()) {
                insertIndex = i + 1;
                break;
            }
        }
    }

    m_spacerIndex = insertIndex;
    if (insertIndex != -1) {
        if (!m_spacer) {
            m_spacer = new Spacer(this);
            m_spacer->panel = this;
        }
        lay->removeItem(m_spacer);
        m_spacer->show();
        lay->insertItem(insertIndex, m_spacer);
    }
}


K_EXPORT_PLASMA_APPLET(panel, Panel)

#include "panel.moc"

