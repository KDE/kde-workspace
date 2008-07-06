/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2004 by Enrico Ros <rosenric@dei.unipd.it>
 *  Copyright (C) 2004 by Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2004 by Oswald Buddenhagen <ossi@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
 * Generic Kdm Item
 */

#include "kdmitem.h"
#include "kdmlayout.h"
#include "kdmthemer.h"

#include <kdm_greet.h> // debug*

#include <QEvent>
#include <QLineEdit>
#include <QPainter>

static bool
toBool( const QString &str )
{
	bool ok;
	int val = str.toInt( &ok );
	if (!ok)
		return str == "true";
	return val != 0;
}

KdmItem::KdmItem( QObject *parent, const QDomNode &node )
	: QObject( parent )
	, boxManager( 0 )
	, fixedManager( 0 )
	, myWidget( 0 )
	, m_showTypeInvert( false )
	, m_minScrWidth( 0 )
	, m_minScrHeight( 0 )
	, m_visible( true )
	, m_shown( true )
{
	QDomNode showNode = node.namedItem( "show" );
	if (!showNode.isNull()) {
		QDomElement sel = showNode.toElement();

		QString modes = sel.attribute( "modes" );
		if (!modes.isNull() &&
		    (modes == "nowhere" ||
		     (modes != "everywhere" &&
		      !modes.split( ",", QString::SkipEmptyParts ).contains( "console" ))))
		{
			m_visible = false;
			return;
		}

		m_showType = sel.attribute( "type" );
		if (!m_showType.isNull()) {
			if (m_showType[0] == '!') {
				m_showType.remove( 0, 1 );
				m_showTypeInvert = true;
			}
			if (!m_showType.startsWith( "plugin-" ) &&
			    themer()->typeVisible( m_showType ) == m_showTypeInvert)
			{
				m_visible = false;
				return;
			}
		}

		m_minScrWidth = sel.attribute( "min-screen-width" ).toInt();
		m_minScrHeight = sel.attribute( "min-screen-height" ).toInt();
	}

	// Set default layout for every item
	currentManager = MNone;
	geom.pos.x.type = geom.pos.y.type =
		geom.size.x.type = geom.size.y.type = DTnone;
	geom.minSize.x.type = geom.minSize.y.type =
		geom.maxSize.x.type = geom.maxSize.y.type = DTpixel;
	geom.minSize.x.val = geom.minSize.y.val = 0;
	geom.maxSize.x.val = geom.maxSize.y.val = 1000000;
	geom.anchor = "nw";
	geom.expand = 0;

	// Set defaults for derived item's properties
	state = Snormal;

	KdmItem *parentItem = qobject_cast<KdmItem *>( parent );
	if (!parentItem)
		style.frame = false;
	else
		style = parentItem->style;

	// Read the mandatory Pos tag. Other tags such as normal, prelighted,
	// etc.. are read under specific implementations.
	QDomNodeList childList = node.childNodes();
	for (int nod = 0; nod < childList.count(); nod++) {
		QDomNode child = childList.item( nod );
		QDomElement el = child.toElement();
		QString tagName = el.tagName();

		if (tagName == "pos") {
			parseSize( el.attribute( "x", QString() ), geom.pos.x );
			parseSize( el.attribute( "y", QString() ), geom.pos.y );
			parseSize( el.attribute( "width", QString() ), geom.size.x );
			parseSize( el.attribute( "height", QString() ), geom.size.y );
			parseSize( el.attribute( "min-width", QString() ), geom.minSize.x );
			parseSize( el.attribute( "min-height", QString() ), geom.minSize.y );
			parseSize( el.attribute( "max-width", QString() ), geom.maxSize.x );
			parseSize( el.attribute( "max-height", QString() ), geom.maxSize.y );
			geom.anchor = el.attribute( "anchor", "nw" );
			geom.expand = toBool( el.attribute( "expand", "false" ) );
		} else if (tagName == "buddy")
			buddy = el.attribute( "idref", "" );
		else if (tagName == "style")
			parseStyle( el, style );
	}

	if (!style.font.present)
		parseFont( "Sans 14", style.font );

	QDomElement el = node.toElement();
	setObjectName( el.attribute( "id", QString::number( (ulong)this, 16 ) ) );
	isButton = toBool( el.attribute( "button", "false" ) );
	isBackground = toBool( el.attribute( "background", "false" ) );

	if (!parentItem)
		// The "toplevel" node (the screen) is really just like a fixed node
		setFixedLayout();
	else
		// Tell 'parent' to add 'me' to its children
		parentItem->addChildItem( this );
}

KdmItem::~KdmItem()
{
	delete boxManager;
	delete fixedManager;
}

void
KdmItem::update()
{
	forEachChild (itm)
		itm->update();
}

void
KdmItem::needUpdate()
{
	emit needUpdate( area.x(), area.y(), area.width(), area.height() );
}

void
KdmItem::updateThisVisible()
{
	bool show = m_shown;
	if (show && (!m_showType.isNull() || m_minScrWidth || m_minScrHeight)) {
		KdmThemer *thm = themer();
		if ((!m_showType.isNull() &&
		     !(thm->typeVisible( m_showType ) ^ m_showTypeInvert)) ||
		    (thm->widget() &&
		     (thm->widget()->width() < m_minScrWidth ||
		      thm->widget()->height() < m_minScrHeight)))
			show = false;
	}
	if (m_visible != show) {
		m_visible = show;
		emit needPlacement();
	}
}

void
KdmItem::setVisible( bool show )
{
	m_shown = show;
	updateThisVisible();
}

void
KdmItem::updateVisible()
{
	updateThisVisible();
	forEachChild (itm)
		itm->updateVisible();
}

KdmThemer *
KdmItem::themer()
{
	if (KdmThemer *thm = qobject_cast<KdmThemer *>(parent()))
		return thm;
	if (KdmItem *parentItem = qobject_cast<KdmItem *>(parent()))
		return parentItem->themer();
	return 0;
}

void
KdmItem::setWidget( QWidget *widget )
{
	if ((myWidget = widget)) {
		myWidget->hide(); // yes, really
		connect( myWidget, SIGNAL(destroyed()), SLOT(widgetGone()) );
		setWidgetAttribs( myWidget );
	}

	emit needPlugging();
	emit needPlacement();
}

void
KdmItem::widgetGone()
{
	myWidget = 0;

	emit needPlugging();
	emit needPlacement();
}

void
KdmItem::setWidgetAttribs( QWidget *widget )
{
	widget->setPalette( style.palette );
	widget->installEventFilter( this );
	updatePalette( myWidget );
	::setWidgetAttribs( widget, style, style.frame );
}

void
KdmItem::updatePalette( QWidget *w )
{
	bool set = w->palette().isBrushSet( w->palette().currentColorGroup(),
	                                    w->backgroundRole() );
	w->setAutoFillBackground( set );
}

bool
KdmItem::eventFilter( QObject *o, QEvent *e )
{
	if (e->type() == QEvent::WindowActivate ||
	    e->type() == QEvent::WindowDeactivate ||
	    e->type() == QEvent::EnabledChange)
	{
		updatePalette( (QWidget *)o );
	}
	return false;
}

void
KdmItem::showWidget( bool show )
{
	if (!isVisible())
		show = false;
	if (myWidget) {
		if (show) {
			QSize sz( area.size().expandedTo( myWidget->minimumSize() )
			                     .boundedTo( myWidget->maximumSize() ) );
			QSize off( (area.size() - sz) / 2 );
			myWidget->setGeometry(
				area.x() + off.width(), area.y() + off.height(),
				sz.width(), sz.height() );
		}
		myWidget->setVisible( show );
	}
	forEachChild (itm)
		itm->showWidget( show );
}

void
KdmItem::plugActions( bool plug )
{
	if (myWidget)
		plug = false;
	doPlugActions( plug );
	forEachChild (itm)
		itm->plugActions( plug );
}

void
KdmItem::doPlugActions( bool )
{
}

/* This is called as a result of KdmLayout::update, and directly on the root */
void
KdmItem::setGeometry( QStack<QSize> &parentSizes, const QRect &newGeometry, bool force )
{
	enter("Item::setGeometry") << objectName() << newGeometry;
	// check if already 'in place'
	if (!force && area == newGeometry) {
		leave() << "unchanged";
		return;
	}

	area = newGeometry;

	// recurr to all boxed children
	if (boxManager && !boxManager->isEmpty())
		boxManager->update( parentSizes, newGeometry, force );

	// recurr to all fixed children
	if (fixedManager && !fixedManager->isEmpty())
		fixedManager->update( parentSizes, newGeometry, force );

	// TODO send *selective* repaint signal

	leave() << "done";
}

void
KdmItem::paint( QPainter *p, const QRect &rect, bool background )
{
	if (!isVisible())
		return;
	if (background &&
	    (p->device()->width() < m_minScrWidth ||
	     p->device()->height() < m_minScrHeight))
		return;

	if (myWidget)
		return;

	QRect contentsRect = area.intersect( rect );
	if (!contentsRect.isEmpty() && (!background || isBackground)) {
		drawContents( p, contentsRect );
		if (debugLevel & DEBUG_THEMING) {
			// Draw bounding rect for this item
			QPen pen( Qt::white );
			pen.setCapStyle( Qt::FlatCap );
			pen.setDashPattern( QVector<qreal>() << 5 << 6 );
			p->setPen( pen );
			p->setBackgroundMode( Qt::OpaqueMode );
			p->setBackground( Qt::black );
			p->drawRect( area.x(), area.y(), area.width() - 1, area.height() - 1 );
			p->setBackgroundMode( Qt::TransparentMode );
		}
	}

	// Dispatch paint events to children
	forEachChild (itm)
		itm->paint( p, rect, background );
}

bool
KdmItem::childrenContain( int x, int y )
{
	forEachVisibleChild (itm) {
		if (itm->area.contains( x, y ))
			return true;
		if (itm->childrenContain( x, y ))
			return true;
	}
	return false;
}

void
KdmItem::activateBuddy()
{
	if (KdmItem *itm = themer()->findNode( buddy ))
		if (itm->myWidget) {
			itm->myWidget->setFocus();
			if (QLineEdit *le = qobject_cast<QLineEdit *>(itm->myWidget))
				le->selectAll();
		}
}

KdmItem *KdmItem::currentActive = 0;

void
KdmItem::mouseEvent( int x, int y, bool pressed, bool released )
{
	if (!isVisible())
		return;

	ItemState oldState = state;
	if (area.contains( x, y ) || (isButton && childrenContain( x, y ))) {
		if (released && oldState == Sactive) {
			if (isButton)
				emit activated( objectName() );
			state = Sprelight;
			currentActive = 0;
		} else if (pressed && !buddy.isEmpty())
			activateBuddy();
		else if (pressed || currentActive == this) {
			state = Sactive;
			currentActive = this;
		} else if (!currentActive)
			state = Sprelight;
		else
			state = Snormal;
	} else {
		if (released)
			currentActive = 0;
		if (currentActive == this)
			state = Sprelight;
		else
			state = Snormal;
	}
	if (oldState != state)
		statusChanged( isButton );

	if (!isButton)
		forEachChild (itm)
			itm->mouseEvent( x, y, pressed, released );
}

void
KdmItem::statusChanged( bool descend )
{
	if (descend)
		forEachChild (o) {
			o->state = state;
			o->statusChanged( descend );
		}
}

// BEGIN protected inheritable

QSize
KdmItem::sizeHint()
{
	if (myWidget)
		return myWidget->sizeHint();
	return QSize(
		geom.size.x.type == DTpixel ? geom.size.x.val : 0,
		geom.size.y.type == DTpixel ? geom.size.y.val : 0 );
}

const QSize &
KdmItem::ensureHintedSize( QSize &hintedSize )
{
	if (!hintedSize.isValid()) {
		hintedSize = sizeHint();
		debug() << "hinted" << hintedSize;
	}
	return hintedSize;
}

const QSize &
KdmItem::ensureBoxHint( QSize &boxHint, QStack<QSize> &parentSizes, QSize &hintedSize )
{
	if (!boxHint.isValid()) {
		if (myWidget || !boxManager)
			boxHint = ensureHintedSize( hintedSize );
		else
			boxHint = boxManager->sizeHint( parentSizes );
		debug() << "boxHint" << boxHint;
	}
	return boxHint;
}

static const QSize &
getParentSize( const QStack<QSize> &parentSizes, int levels )
{
	int off = parentSizes.size() - 1 - levels;
	if (off < 0) {
		kError() << "Theme references element below the root.";
		off = 0;
	}
	return parentSizes[off];
}

void
KdmItem::calcSize(
	const DataPair &sz,
	QStack<QSize> &parentSizes, QSize &hintedSize, QSize &boxHint,
	QSize &io )
{
	int w, h;

	if (sz.x.type == DTpixel)
		w = sz.x.val;
	else if (sz.x.type == DTnpixel)
		w = io.width() - sz.x.val;
	else if (sz.x.type == DTpercent)
		w = (getParentSize( parentSizes, sz.x.levels ).width() * sz.x.val + 50) / 100;
	else if (sz.x.type == DTbox)
		w = ensureBoxHint( boxHint, parentSizes, hintedSize ).width();
	else
		w = ensureHintedSize( hintedSize ).width();

	if (sz.y.type == DTpixel)
		h = sz.y.val;
	else if (sz.y.type == DTnpixel)
		h = io.height() - sz.y.val;
	else if (sz.y.type == DTpercent)
		h = (getParentSize( parentSizes, sz.y.levels ).height() * sz.y.val + 50) / 100;
	else if (sz.y.type == DTbox)
		h = ensureBoxHint( boxHint, parentSizes, hintedSize ).height();
	else
		h = ensureHintedSize( hintedSize ).height();

	if (sz.x.type == DTscale && h && ensureHintedSize( hintedSize ).height())
		w = w * h / hintedSize.height();
	else if (sz.y.type == DTscale && w && ensureHintedSize( hintedSize ).width())
		h = w * h / hintedSize.width();

	io.setWidth( w );
	io.setHeight( h );
}

void
KdmItem::sizingHint( QStack<QSize> &parentSizes, SizeHint &hint )
{
	enter("Item::sizingHint") << objectName() << NoSpace << "parentSize #"
		<< parentSizes.size() << Space << parentSizes.top();

	QSize hintedSize, boxHint;
	hint.min = hint.opt = hint.max = parentSizes.top();
	calcSize( geom.size, parentSizes, hintedSize, boxHint, hint.opt );
	calcSize( geom.minSize, parentSizes, hintedSize, boxHint, hint.min );
	calcSize( geom.maxSize, parentSizes, hintedSize, boxHint, hint.max );

	leave() << "size" << hint.opt << "min" << hint.min << "max" << hint.max;

	hint.max = hint.max.expandedTo( hint.min ); // if this triggers, the theme is bust
	hint.opt = hint.opt.boundedTo( hint.max ).expandedTo( hint.min );

	// Note: no clipping to parent because this broke many themes!
}

QRect
KdmItem::placementHint( QStack<QSize> &sizes, const QSize &sz, const QPoint &offset )
{
	const QSize &parentSize = sizes.top();
	int x = offset.x(),
	    y = offset.y(),
	    w = parentSize.width(),
	    h = parentSize.height();

	enter("Item::placementHint") << objectName() << NoSpace << "parentSize #"
		<< sizes.size() << Space << parentSize << "size" << sz << "offset" << offset;

	if (geom.pos.x.type == DTpixel)
		x += geom.pos.x.val;
	else if (geom.pos.x.type == DTnpixel)
		x += w - geom.pos.x.val;
	else if (geom.pos.x.type == DTpercent)
		x += (w * geom.pos.x.val + 50) / 100;

	if (geom.pos.y.type == DTpixel)
		y += geom.pos.y.val;
	else if (geom.pos.y.type == DTnpixel)
		y += h - geom.pos.y.val;
	else if (geom.pos.y.type == DTpercent)
		y += (h * geom.pos.y.val + 50) / 100;

	// defaults to center
	int dx = sz.width() / 2, dy = sz.height() / 2;

	// anchor the rect to an edge / corner
	if (geom.anchor.length() > 0 && geom.anchor.length() < 3) {
		if (geom.anchor.indexOf( 'n' ) >= 0)
			dy = 0;
		if (geom.anchor.indexOf( 's' ) >= 0)
			dy = sz.height();
		if (geom.anchor.indexOf( 'w' ) >= 0)
			dx = 0;
		if (geom.anchor.indexOf( 'e' ) >= 0)
			dx = sz.width();
	}
	leave() << "x:" << x << "dx:" << dx << "y:" << y << "dy:" << dy;
	y -= dy;
	x -= dx;

	return QRect( x, y, sz.width(), sz.height() );
}

QRect
KdmItem::placementHint( QStack<QSize> &sizes, const QPoint &offset )
{
	SizeHint sh;
	sizingHint( sizes, sh );
	return placementHint( sizes, sh.opt, offset );
}

// END protected inheritable


void
KdmItem::showStructure( const QString &pfx )
{
	{
		QDebug ds( QtDebugMsg );
		if (!pfx.isEmpty())
			ds << (qPrintable( pfx ) + 1);
		ds << objectName() << qPrintable( itemType ) << area;
	}
	if (!m_children.isEmpty()) {
		QString npfx( pfx );
		npfx.replace( '\\', ' ' ).replace( '-', ' ' );
		for (int i = 0; i < m_children.count() - 1; i++)
			m_children[i]->showStructure( npfx + " |-" );
		m_children[m_children.count() - 1]->showStructure( npfx + " \\-" );
	}
}

void
KdmItem::addChildItem( KdmItem *item )
{
	m_children.append( item );
	switch (currentManager) {
	case MNone:		// fallback to the 'fixed' case
		setFixedLayout();
	case MFixed:
		fixedManager->addItem( item );
		break;
	case MBox:
		boxManager->addItem( item );
		break;
	}
}

void
KdmItem::setBoxLayout( const QDomNode &node )
{
	if (!boxManager)
		boxManager = new KdmLayoutBox( node );
	currentManager = MBox;
}

void
KdmItem::setFixedLayout( const QDomNode &node )
{
	if (!fixedManager)
		fixedManager = new KdmLayoutFixed( node );
	currentManager = MFixed;
}

#include "kdmitem.moc"
