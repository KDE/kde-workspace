/*
 * $Id$
 *
 * KDE3 HighColor Style (version 0.0.3a)
 * Copyright (C) 2001 Karol Szwed      <gallium@kde.org>
 *           (C) 2001 Fredrik H�glund  <fredrik@kde.org> 
 *
 * Drawing routines adapted from the KDE2 HCStyle,
 * Copyright (C) 2000 Daniel M. Duley  <mosfet@kde.org>
 *           (C) 2000 Dirk Mueller     <mueller@kde.org>
 *           (C) 2001 Martijn Klingens <mklingens@yahoo.com>
 *
 * Many thanks to Bradley T. Hughes for the 3 button scrollbar code.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <qstyleplugin.h>
#include <qstylefactory.h>
#include <qcommonstyle.h>
#include <qpointarray.h>
#include <qpainter.h>
#include <qtabbar.h>
#include <qcombobox.h>
#include <qscrollbar.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qdrawutil.h>
#include <qapplication.h>
#include <qvariant.h>
#include <kpixmapeffect.h>
#include <kdrawutil.h>

#include "highcolor.h"
#include "highcolor.moc"
#include "bitmaps.h"


// -- Style Plugin Interface -------------------------
class HighColorStylePlugin : public QStylePlugin
{
	public:
		HighColorStylePlugin() {}
		~HighColorStylePlugin() {}

		QStringList keys() const
		{
			return QStringList() << "HighColor" << "Default";
		}

		QStyle* create( const QString& key )
		{
			if ( key == "highcolor" )
				return new HighColorStyle( true );
			
			else if ( key == "default" )
				return new HighColorStyle( false );
			
			return 0;
		}
};

Q_EXPORT_PLUGIN( HighColorStylePlugin )
// ---------------------------------------------------


// ### Remove globals
QBitmap lightBmp;
QBitmap grayBmp;
QBitmap dgrayBmp;
QBitmap centerBmp;
QBitmap maskBmp;
QBitmap xBmp;
QIntDict<GradientSet> gDict;

static const int itemFrame       = 2;
static const int itemHMargin     = 3;
static const int itemVMargin     = 1;
static const int arrowHMargin    = 6;
static const int rightBorder     = 12;

// ---------------------------------------------------------------------------

GradientSet::GradientSet(const QColor &baseColor)
{
	c = baseColor;
	for(int i=0; i < GradientCount; i++)
		gradients[i] = NULL;
}


GradientSet::~GradientSet()
{
	for(int i=0; i < GradientCount; i++)
		if(gradients[i])
			delete gradients[i];
}


KPixmap* GradientSet::gradient(GradientType type)
{
	if (gradients[type])
		return gradients[type];

	switch(type)
	{
		case VSmall: {
			gradients[VSmall] = new KPixmap;
			gradients[VSmall]->resize(18, 24);
			KPixmapEffect::gradient(*gradients[VSmall], c.light(110), c.dark(110),
											KPixmapEffect::VerticalGradient);
			break;
		}

		case VMed: {
			gradients[VMed] = new KPixmap;
			gradients[VMed]->resize(18, 34);
			KPixmapEffect::gradient(*gradients[VMed], c.light(110), c.dark(110),
											KPixmapEffect::VerticalGradient);
			break;
		}

		case VLarge: {
			gradients[VLarge] = new KPixmap;
			gradients[VLarge]->resize(18, 64);
			KPixmapEffect::gradient(*gradients[VLarge], c.light(110), c.dark(110),
											KPixmapEffect::VerticalGradient);
			break;
		}

		case HMed: {
			gradients[HMed] = new KPixmap;
			gradients[HMed]->resize(34, 18);
			KPixmapEffect::gradient(*gradients[HMed], c.light(110), c.dark(110),
											KPixmapEffect::HorizontalGradient);
			break;
		}

		case HLarge: {
			gradients[HLarge] = new KPixmap;
			gradients[HLarge]->resize(52, 18);
			KPixmapEffect::gradient(*gradients[HLarge], c.light(110), c.dark(110),
											KPixmapEffect::HorizontalGradient);
			break;
		}

		default:
			break;
	}
	return(gradients[type]);
}



// ---------------------------------------------------------------------------

HighColorStyle::HighColorStyle( bool hc ) : QCommonStyle()
{
	winstyle = QStyleFactory::create("Windows");
	if(!winstyle) {
		// We don't have the Windows style, neither builtin nor as a plugin.
		// Use any style rather than crashing.
		winstyle = QStyleFactory::create(*(QStyleFactory::keys().begin()));
	}
	highcolor = (QPixmap::defaultDepth() > 8) ? hc : false;
	gDict.setAutoDelete(true);
}


HighColorStyle::~HighColorStyle()
{
}


void HighColorStyle::polish(QWidget* widget)
{
	// Put in order of highest occurance to maximise hit rate
	if (widget->inherits("QPushButton")) {
		widget->installEventFilter(this);
	}

	else if (widget->inherits("QToolBar")) {
		widget->installEventFilter(this);
	}

	else if (widget->inherits("QMenuBar")) {
		widget->installEventFilter(this);
		widget->setBackgroundMode(QWidget::NoBackground);
	}
}


void HighColorStyle::unPolish(QWidget* widget)
{
	if (widget->inherits("QPushButton")) {
		widget->removeEventFilter(this);
	}

	else if (widget->inherits("QToolBar")) {
		widget->removeEventFilter(this);
	}
	
	else if (widget->inherits("QMenuBar")) {
		widget->setBackgroundMode(QWidget::PaletteBackground);
		widget->removeEventFilter(this);
	}
}


// This function draws primitive elements as well as their masks.
void HighColorStyle::drawPrimitive( PrimitiveElement pe,
									QPainter *p,
									const QRect &r,
									const QColorGroup &cg,
									SFlags flags,
									const QStyleOption& opt ) const
{
	bool down = flags & Style_Down;
	bool on   = flags & Style_On;

	switch(pe)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PE_ButtonDefault: {
			if ( ! highcolor ) {
				int x1, y1, x2, y2;
				r.coords( &x1, &y1, &x2, &y2 );
				
				// Button default indicator
				p->setPen( cg.shadow() );
				p->drawLine( x1+1, y1, x2-1, y1 );
				p->drawLine( x1, y1+1, x1, y2-1 );
				p->drawLine( x1+1, y2, x2-1, y2 );
				p->drawLine( x2, y1+1, x2, y2-1 );
			}
			break;
		}
			
		case PE_ButtonDropDown:
		case PE_ButtonTool: {
			bool sunken = on || down;
			int  x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			// Outer frame (round style)
			p->setPen(cg.shadow());
			p->drawLine(x+1,y,x2-1,y);
			p->drawLine(x,y+1,x,y2-1);
			p->drawLine(x+1,y2,x2-1,y2);
			p->drawLine(x2,y+1,x2,y2-1);

			// Bevel
			p->setPen(sunken ? cg.mid() : cg.light());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->setPen(sunken ? cg.light() : cg.mid());
			p->drawLine(x+2, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+2, x2-1, y2-1);

			p->fillRect(x+2, y+2, w-4, h-4, cg.button());

			break;
		}

		// PUSH BUTTON
		// -------------------------------------------------------------------
		case PE_ButtonCommand: {
			bool sunken = on || down;
			int  x, y, w, h;
			r.rect(&x, &y, &w, &h);

			if ( sunken )
				kDrawBeButton( p, x, y, w, h, cg, true,
						&cg.brush(QColorGroup::Mid) );
			
			else if ( flags & Style_MouseOver ) {
				QBrush brush(cg.button().light(110));
				kDrawBeButton( p, x, y, w, h, cg, false, &brush );
			}

			// "Flat" button
			else if (!(flags & (Style_Raised | Style_Sunken)))
				p->fillRect(r, cg.button());
			
			else if( highcolor )
			{
				int x2 = x+w-1;
				int y2 = y+h-1;
				p->setPen(cg.shadow());
				p->drawLine(x+1, y, x2-1, y);
				p->drawLine(x+1, y2, x2-1, y2);
				p->drawLine(x, y+1, x, y2-1);
				p->drawLine(x2, y+1, x2, y2-1);

				p->setPen(cg.light());
				p->drawLine(x+2, y+2, x2-1, y+2);
				p->drawLine(x+2, y+3, x2-2, y+3);
				p->drawLine(x+2, y+4, x+2, y2-1);
				p->drawLine(x+3, y+4, x+3, y2-2);

				p->setPen(cg.mid());
				p->drawLine(x2-1, y+2, x2-1, y2-1);
				p->drawLine(x+2, y2-1, x2-1, y2-1);

				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+2, x+1, y2-1);
				p->drawLine(x2-2, y+3, x2-2, y2-2);

				renderGradient(p, QRect(x+4, y+4, w-6, h-6),
								cg.button(), false);
			} else
				kDrawBeButton(p, x, y, w, h, cg, false,
							  &cg.brush(QColorGroup::Button));
			break;
		}


		// BEVELS
		// -------------------------------------------------------------------
		case PE_ButtonBevel: {
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			bool sunken = on || down;
			int x2 = x+w-1;
			int y2 = y+h-1;

			// Outer frame
			p->setPen(cg.shadow());
			p->drawRect(r);

			// Bevel
			p->setPen(sunken ? cg.mid() : cg.light());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->setPen(sunken ? cg.light() : cg.mid());
			p->drawLine(x+2, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+2, x2-1, y2-1);

			if (w > 4) {
				if (sunken)
					p->fillRect(x+2, y+2, w-4, h-4, cg.button());
				else
					renderGradient( p, QRect(x+2, y+2, w-4, h-4),
								    cg.button(), flags & Style_Horizontal );
			}
			break;
		}


		// FOCUS RECT
		// -------------------------------------------------------------------
		case PE_FocusRect: {
			p->drawWinFocusRect( r );
			break;
		}


		// HEADER SECTION
		// -------------------------------------------------------------------
		case PE_HeaderSection: {
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			bool sunken = on || down;
			int x2 = x+w-1;
			int y2 = y+h-1;

			// Bevel
			p->setPen(sunken ? cg.mid() : cg.light());
			p->drawLine(x, y, x2-1, y);
			p->drawLine(x, y, x, y2-1);
			p->setPen(sunken ? cg.light() : cg.mid());
			p->drawLine(x+1, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+1, x2-1, y2-1);
			p->setPen(cg.shadow());
			p->drawLine(x, y2, x2, y2);
			p->drawLine(x2, y, x2, y2);

			if (sunken)
				p->fillRect(x+1, y+1, w-3, h-3, cg.button());
			else
				renderGradient( p, QRect(x+1, y+1, w-3, h-3),
							    cg.button(), flags & Style_Horizontal );
			break;
		}


		// SCROLLBAR
		// -------------------------------------------------------------------
		case PE_ScrollBarSlider: {
			// ### Small hack to ensure scrollbar gradients are drawn the right way.
			flags ^= Style_Horizontal;

			drawPrimitive(PE_ButtonBevel, p, r, cg, flags | Style_Enabled | Style_Raised);

			// Draw a scrollbar riffle (note direction after above changes)
			if (flags & Style_Horizontal) {
				if (r.height() >= 15) {
					int x = r.x()+3;
					int y = r.y() + (r.height()-7)/2;
					int x2 = r.right()-3;
					p->setPen(cg.light());
					p->drawLine(x, y, x2, y);
					p->drawLine(x, y+3, x2, y+3);
					p->drawLine(x, y+6, x2, y+6);

					p->setPen(cg.mid());
					p->drawLine(x, y+1, x2, y+1);
					p->drawLine(x, y+4, x2, y+4);
					p->drawLine(x, y+7, x2, y+7);
				}
			} else {
				if (r.width() >= 15) {
					int y = r.y()+3;
					int x = r.x() + (r.width()-7)/2;
					int y2 = r.bottom()-3;
					p->setPen(cg.light());
					p->drawLine(x, y, x, y2);
					p->drawLine(x+3, y, x+3, y2);
					p->drawLine(x+6, y, x+6, y2);

					p->setPen(cg.mid());
					p->drawLine(x+1, y, x+1, y2);
					p->drawLine(x+4, y, x+4, y2);
					p->drawLine(x+7, y, x+7, y2);
				}
			}
			break;
		}

		case PE_ScrollBarAddPage:
		case PE_ScrollBarSubPage: {
			p->setPen(cg.shadow());
			int x, y, w, h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			if (flags & Style_Horizontal) {
				p->drawLine(x, y, x2, y);
				p->drawLine(x, y2, x2, y2);
				renderGradient(p, QRect(x, y+1, w, h-2),
							   cg.mid(), false);
			} else {
				p->drawLine(x, y, x, y2);
				p->drawLine(x2, y, x2, y2);
				renderGradient(p, QRect(x+1, y, w-2, h),
							   cg.mid(), true);
			}
			break;
		}


		case PE_ScrollBarAddLine: {
			drawPrimitive( PE_ButtonBevel, p, r, cg, (flags & Style_Enabled) |
					((flags & Style_Down) ? Style_Down : Style_Raised) );

			drawPrimitive( ((flags & Style_Horizontal) ? PE_ArrowRight : PE_ArrowDown),
					p, r, cg, flags );
			break;
		}


		case PE_ScrollBarSubLine: {
			drawPrimitive( PE_ButtonBevel, p, r, cg, (flags & Style_Enabled) |
					((flags & Style_Down) ? Style_Down : Style_Raised) );

			drawPrimitive( ((flags & Style_Horizontal) ? PE_ArrowLeft : PE_ArrowUp),
					p, r, cg, flags );
			break;
		}


		// CHECKBOX (indicator)
		// -------------------------------------------------------------------
		case PE_Indicator: {
			
			bool enabled  = flags & Style_Enabled;
			bool nochange = flags & Style_NoChange;
			
			if (xBmp.isNull()) {
				xBmp = QBitmap(7, 7, x_bits, true);
				xBmp.setMask(xBmp);
			}

			int x,y,w,h;
			x=r.x(); y=r.y(); w=r.width(); h=r.height();
			int x2 = x+w-1;
			int y2 = y+h-1;

			p->setPen(cg.mid());
			p->drawLine(x, y, x2, y);
			p->drawLine(x, y, x, y2);

			p->setPen(cg.light());
			p->drawLine(x2, y+1, x2, y2);
			p->drawLine(x+1, y2, x2, y2);

			p->setPen(cg.shadow());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);

			p->setPen(cg.midlight());
			p->drawLine(x2-1, y+2, x2-1, y2-1);
			p->drawLine(x+2, y2-1, x2-1, y2-1);

			if ( enabled )
				p->fillRect(x+2, y+2, w-4, h-4, 
						down ? cg.button(): cg.base());
			else
				p->fillRect(x+2, y+2, w-4, h-4, cg.background());
			
			if (!(flags & Style_Off)) {
				if (on) {
					p->setPen(nochange ? cg.dark() : cg.text());
					p->drawPixmap(3, 3, xBmp);
				}
				else {
					p->setPen(cg.shadow());
					p->drawRect(x+2, y+2, w-4, h-4);
					p->setPen(nochange ? cg.text() : cg.dark());
					p->drawLine(x+3, (y+h)/2-2, x+w-4, (y+h)/2-2);
					p->drawLine(x+3, (y+h)/2, x+w-4, (y+h)/2);
					p->drawLine(x+3, (y+h)/2+2, x+w-4, (y+h)/2+2);
				}
			}
			break;
		}


		// RADIOBUTTON (exclusive indicator)
		// -------------------------------------------------------------------
		case PE_ExclusiveIndicator: {
			
			bool enabled = flags & Style_Enabled;
			
			if (lightBmp.isNull()) {
				lightBmp  = QBitmap(13, 13, radiooff_light_bits,  true);
				grayBmp   = QBitmap(13, 13, radiooff_gray_bits,   true);
				dgrayBmp  = QBitmap(13, 13, radiooff_dgray_bits,  true);
				centerBmp = QBitmap(13, 13, radiooff_center_bits, true);
			}

			// Surrouding background
			p->fillRect(r, cg.brush(QColorGroup::Background));
			
			// The center fill of the indicator (grayed out when disabled)
			if ( enabled )
				p->setPen( down ? cg.button() : cg.base() );
			else
				p->setPen( cg.background() );
			
			p->drawPixmap( r.x(), r.y(), centerBmp );
			
			// Bevel
			kColorBitmaps(p, cg, r.x(), r.y(), &lightBmp , &grayBmp,
						  NULL, &dgrayBmp);

			// Indicator "dot"
			if ( on ) {
				QColor color = flags & Style_NoChange ?
					cg.dark() : cg.text();
				
				p->setPen(color);
				p->drawLine(5, 4, 7, 4);
				p->drawLine(4, 5, 4, 7);
				p->drawLine(5, 8, 7, 8);
				p->drawLine(8, 5, 8, 7);
				p->fillRect(5, 5, 3, 3, color);
			}

			break;
		}


		// Draw the exclusive indicator (radio button) mask.
		case PE_ExclusiveIndicatorMask: {
			if (maskBmp.isNull())
				maskBmp = QBitmap(13, 13, radiomask_bits, true);
			p->setPen(Qt::color0);
			p->drawPixmap(r.x(), r.y(), maskBmp);
			break;
		}


		// PROGRESSBAR
		// -------------------------------------------------------------------
//		case PE_ProgressBarChunk: {
			// ### KDE 3.0 - Make KDE2-style smooth gradient slider
//			break;
//		}


		// SPLITTER PANEL
		// -------------------------------------------------------------------
		case PE_Splitter: {
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			p->setPen(cg.dark());
			p->drawRect(x, y, w, h);
			p->setPen(cg.background());
			p->drawPoint(x, y);
			p->drawPoint(x2, y);
			p->drawPoint(x, y2);
			p->drawPoint(x2, y2);
			p->setPen(cg.light());
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->setPen(cg.midlight());
			p->drawLine(x+2, y+2, x+2, y2-2);
			p->drawLine(x+2, y+2, x2-2, y+2);
			p->setPen(cg.mid());
			p->drawLine(x2-1, y+1, x2-1, y2-1);
			p->drawLine(x+1, y2-1, x2-1, y2-1);
			p->fillRect(x+3, y+3, w-5, h-5, cg.brush(QColorGroup::Background));
			break;
		}


		// GENERAL PANELS
		// -------------------------------------------------------------------
		case PE_Panel:
		case PE_PanelPopup: {
			bool sunken = flags & Style_Sunken;
			int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth)
										: opt.lineWidth();
			if (lw == 2 && sunken)
			{
				QPen oldPen = p->pen();
				int x,y,w,h;
				r.rect(&x, &y, &w, &h);
				int x2 = x+w-1;
				int y2 = y+h-1;
				p->setPen(cg.light());
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);
				p->setPen(cg.mid());
				p->drawLine(x, y, x2, y);
				p->drawLine(x, y, x, y2);
				p->setPen(cg.midlight());
				p->drawLine(x+1, y2-1, x2-1, y2-1);
				p->drawLine(x2-1, y+1, x2-1, y2-1);
				p->setPen(cg.dark());
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+1, x+1, y2-1);
				p->setPen(oldPen);
			} else
				QCommonStyle::drawPrimitive(pe, p, r, cg, flags, opt);

			break;
		}


		// MENU / TOOLBAR PANEL
		// -------------------------------------------------------------------
		case PE_PanelMenuBar: 			// Menu
		case PE_PanelDockWindow: {		// Toolbar
			int x2 = r.x()+r.width()-1;
			int y2 = r.y()+r.height()-1;

			p->setPen(cg.light());
			p->drawLine(r.x(), r.y(), x2-1,  r.y());
			p->drawLine(r.x(), r.y(), r.x(), y2-1);
			p->setPen(cg.dark());
			p->drawLine(r.x(), y2, x2, y2);
			p->drawLine(x2, r.y(), x2, y2);

			// ### Qt should specify Style_Horizontal where appropriate
			renderGradient( p, QRect(r.x()+1, r.y()+1, x2-1, y2-1),
				cg.button(), (r.width() < r.height()) &&
								 (pe != PE_PanelMenuBar) );

			break;
		}


		// TOOLBAR/DOCK WINDOW HANDLE
		// -------------------------------------------------------------------
		case PE_DockWindowHandle: {

			// wild hacks are here. beware.
			QWidget* widget;
			if (p->device()->devType() == QInternal::Widget)
				widget = dynamic_cast<QWidget*>(p->device());
			else
				return;		// Don't paint on non-widgets

			// Check if we are a normal toolbar or a hidden dockwidget.
			if ( widget->parent() &&
			    (widget->parent()->inherits("QToolBar") ||		// Normal toolbar
				(widget->parent()->inherits("QMainWindow")) ))	// Collapsed dock
			{
				// Draw a toolbar handle
				int x = r.x(); int y = r.y();
				int x2 = r.x() + r.width()-1;
				int y2 = r.y() + r.height()-1;

				if (!(flags & Style_Horizontal)) {
					renderGradient( p, r, cg.button(), true);

					p->setPen(cg.light());
					p->drawLine(x+4, y+3, x2-4, y+3);
					p->drawLine(x+4, y+5, x2-4, y+5);
					p->drawLine(x+4, y+7, x2-4, y+7);

					p->setPen(cg.mid());
					p->drawLine(x+4, y+4, x2-4, y+4);
					p->drawLine(x+4, y+6, x2-4, y+6);
					p->drawLine(x+4, y+8, x2-4, y+8);

				} else {
					renderGradient( p, r, cg.button(), false);

					p->setPen(cg.light());
					p->drawLine(x+3, y+4, x+3, y2-4);
					p->drawLine(x+5, y+4, x+5, y2-4);
					p->drawLine(x+7, y+4, x+7, y2-4);

					p->setPen(cg.mid());
					p->drawLine(x+4, y+4, x+4, y2-4);
					p->drawLine(x+6, y+4, x+6, y2-4);
					p->drawLine(x+8, y+4, x+8, y2-4);
				}
			} else
				// We have a docked handle with a close button
				if (widget->inherits("QDockWindowHandle"))
				{
					bool horizontal = flags & Style_Horizontal;
					int x,y,w,h,x2,y2;

					r.rect( &x, &y, &w, &h );
					x2 = x + w - 1;
					y2 = y + h - 1;

					QFont fnt;
					fnt = QApplication::font(widget);
					fnt.setPointSize( fnt.pointSize()-2 );

					// Draw the item on an off-screen pixmap
					// to preserve Xft antialiasing for 
					// vertically oriented handles.
					QPixmap pix;
					if (horizontal)
						pix.resize( h-2, w-2 );
					else	
						pix.resize( w-2, h-2 );

					pix.fill(cg.highlight());
					QPainter p2;
					p2.begin(&pix);
					p2.setPen(cg.background());
					p2.setFont(fnt);
					p2.drawText(pix.rect(), AlignCenter, 
							widget->property("caption").toString());
					p2.end();

					// Draw a sunken bevel
					p->setPen(cg.dark());
					p->drawLine(x, y, x2, y);
					p->drawLine(x, y, x, y2);
					p->setPen(cg.light());
					p->drawLine(x+1, y2, x2, y2);
					p->drawLine(x2, y+1, x2, y2);
					
					if (horizontal) {
						QWMatrix m;
						m.rotate(-90.0);
						QPixmap vpix = pix.xForm(m);
						bitBlt(widget, r.x()+1, r.y()+1, &vpix); 
					} else
						bitBlt(widget, r.x()+1, r.y()+1, &pix);

				} else
					p->fillRect(r, cg.highlight());
			break;
		}


		// TOOLBAR SEPARATOR
		// -------------------------------------------------------------------
		case PE_DockWindowSeparator: {
			renderGradient( p, r, cg.button(),
							!(flags & Style_Horizontal));
			if ( !(flags & Style_Horizontal) ) {
				p->setPen(cg.mid());
				p->drawLine(4, r.height()/2, r.width()-5, r.height()/2);
				p->setPen(cg.light());
				p->drawLine(4, r.height()/2+1, r.width()-5, r.height()/2+1);
			} else {
				p->setPen(cg.mid());
				p->drawLine(r.width()/2, 4, r.width()/2, r.height()-5);
				p->setPen(cg.light());
				p->drawLine(r.width()/2+1, 4, r.width()/2+1, r.height()-5);
			}
			break;
		}


		default:
		{
			// ARROWS
			// -------------------------------------------------------------------
			if (pe >= PE_ArrowUp && pe <= PE_ArrowLeft)
			{
				QPointArray a;

				switch(pe) {
					case PE_ArrowUp:
						a.setPoints(QCOORDARRLEN(u_arrow), u_arrow);
						break;

					case PE_ArrowDown:
						a.setPoints(QCOORDARRLEN(d_arrow), d_arrow);
						break;

					case PE_ArrowLeft:
						a.setPoints(QCOORDARRLEN(l_arrow), l_arrow);
						break;

					default:
						a.setPoints(QCOORDARRLEN(r_arrow), r_arrow);
				}

				p->save();
				if ( flags & Style_Down )
					p->translate( pixelMetric( PM_ButtonShiftHorizontal ),
								  pixelMetric( PM_ButtonShiftVertical ) );

				if ( flags & Style_Enabled ) {
					a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
					p->setPen( cg.buttonText() );
					p->drawLineSegments( a );
				} else {
					a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
					p->setPen( cg.light() );
					p->drawLineSegments( a );
					a.translate( -1, -1 );
					p->setPen( cg.mid() );
					p->drawLineSegments( a );
				}
				p->restore();

			} else
				winstyle->drawPrimitive( pe, p, r, cg, flags, opt );
		}
	}
}


void HighColorStyle::drawControl( ControlElement element,
								  QPainter *p,
								  const QWidget *widget,
								  const QRect &r,
								  const QColorGroup &cg,
								  SFlags flags,
								  const QStyleOption& opt ) const
{
	switch (element)
	{
		// PUSHBUTTON
		// ----------------------------------------------------------------------------------
		case CE_PushButton: {
			if ( widget == hoverWidget )
				flags |= Style_MouseOver;
			
			if ( ! highcolor ) {
				QPushButton *button = (QPushButton*) widget;
				QRect br = r;
				
				// ### Optimize!
				//   Dont't call button->isDefault() twice.
				//   Dont't call pixelMetric() - use a constant or variable instead.
				//   Dont't call drawPrimitive() to draw the indicator - draw it inline.
				
				if ( button->isDefault() || button->autoDefault() ) {
					// Compensate for default indicator
					int di = pixelMetric( PM_ButtonDefaultIndicator );
					br.addCoords( di, di, -di, -di );
				}

				if ( button->isDefault() )
					drawPrimitive( PE_ButtonDefault, p, r, cg, flags );
				
				drawPrimitive( PE_ButtonCommand, p, br, cg, flags );
				
			} else
				drawPrimitive( PE_ButtonCommand, p, r, cg, flags );
			
			break;
		}

							   
		// PUSHBUTTON LABEL
		// ----------------------------------------------------------------------------------
		case CE_PushButtonLabel: {
			const QPushButton* button = (const QPushButton*)widget;
			bool active = button->isOn() || button->isDown();
			int x, y, w, h;
			r.rect( &x, &y, &w, &h );

			if ( active ) {
				x += pixelMetric(PM_ButtonShiftHorizontal, widget); 
				y += pixelMetric(PM_ButtonShiftVertical, widget);
			}

			// Does the button have a popup menu?
			if ( button->isMenuButton() ) {
				SFlags arrow_flags = Style_Default;
				int dx = pixelMetric( PM_MenuButtonIndicator, widget );
				int xx = x + w - dx - 4;
				int yy = y - 3;
				int hh = h + 6;

				// Draw a seperator line between the button contents and the arrow
				if ( !active ) {
					p->setPen( cg.light() );
					p->drawLine( xx + 1, yy + 5, xx + 1, yy + hh - 6 );
					p->setPen( cg.mid() );
					p->drawLine( xx, yy + 6, xx, yy + hh - 6 );
				} else {
					p->setPen( cg.button() );
					p->drawLine( xx, yy + 4, xx, yy + hh - 4 );
				}
				drawPrimitive( PE_ArrowDown, p, QRect(x + w - dx - 2, y + 2, dx, h - 4),
							   cg, arrow_flags, opt );
				w -= dx;
			}

			// Draw the icon if there is one
			if ( button->iconSet() && !button->iconSet()->isNull() ) {
				QIconSet::Mode mode = QIconSet::Disabled;

				if (button->isEnabled())
					if (button->hasFocus())
						mode = QIconSet::Active;
					else
						mode = QIconSet::Normal;

				QPixmap pixmap = button->iconSet()->pixmap( QIconSet::Small, mode );
				int pixw = pixmap.width();
				int pixh = pixmap.height();

				p->drawPixmap( x + 6, y + h / 2 - pixh / 2, pixmap );
				x += pixw + 8;
				w -= pixw + 8;
			}

			// Make the label indicate if the button is a default button or not
			if ( active || button->isDefault() ) {
				QFont font = button->font();
				font.setBold( true );
				p->setFont( font );

				drawItem( p, QRect(x+1, y+1, w, h), AlignCenter | ShowPrefix, button->colorGroup(),
						button->isEnabled(), button->pixmap(), button->text(), -1,
						active ? &button->colorGroup().dark() : &button->colorGroup().mid() );

				drawItem( p, QRect(x, y, w, h), AlignCenter | ShowPrefix, button->colorGroup(),
						button->isEnabled(), button->pixmap(), button->text(), -1,
						active ? &button->colorGroup().light() : &button->colorGroup().text() );
			} else {
				drawItem( p, QRect(x, y, w, h), AlignCenter | ShowPrefix, button->colorGroup(),
						button->isEnabled(), button->pixmap(), button->text(), -1,
						active ? &button->colorGroup().light() : &button->colorGroup().buttonText() );
			}

			// Draw a focus rect if the button has focus
			if ( flags & Style_HasFocus )
				drawPrimitive( PE_FocusRect, p,
						QStyle::visualRect(subRect(SR_PushButtonFocusRect, widget), widget),
						cg, flags );
			break;
		}


		// TABS
		// -------------------------------------------------------------------
		case CE_TabBarTab: {
			const QTabBar * tb = (const QTabBar *) widget;

			// ### Only rounded "above" tabs are implemented thus far
			if (tb->shape() != QTabBar::RoundedAbove) {
				winstyle->drawControl(element, p, widget, r, cg, flags, opt);
				return;
			}

			if (flags & Style_Selected) {

				p->setPen(cg.light());
				p->drawLine(r.x(), r.y()+4, r.x(), r.bottom());
				p->drawLine(r.x(), r.y()+4, r.x()+4, r.y());
				p->drawLine(r.x()+4, r.y(), r.right()-1, r.y());
				p->setPen(cg.shadow());
				p->drawLine(r.right(), r.y()+1, r.right(), r.bottom());

				p->setPen(cg.midlight());
				p->drawLine(r.x()+1, r.y()+4, r.x()+1, r.bottom());
				p->drawLine(r.x()+1, r.y()+4, r.x()+4, r.y()+1);
				p->drawLine(r.x()+5, r.y()+1, r.right() - 2, r.y()+1);
				p->setPen(cg.mid());
				p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom());

			} else {

				p->translate(0,1);
				p->setPen(cg.shadow());
				p->drawLine(r.x(), r.y()+4, r.x(), r.bottom()-2);
				p->drawLine(r.x(), r.y()+4, r.x()+4, r.y());
				p->drawLine(r.x()+4, r.y(), r.right()-1, r.y());
				p->drawLine(r.right(), r.y()+1, r.right(), r.bottom()-2);

				p->setPen(cg.midlight());
				p->drawLine(r.x()+1, r.y()+4, r.x()+1, r.bottom());
				p->drawLine(r.x()+1, r.y()+4, r.x()+4, r.y()+1);
				p->drawLine(r.x()+5, r.y()+1, r.right()-2, r.y()+1);
				p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());

				p->setPen(cg.mid());
				p->drawPoint(r.right()-1, r.y()+1);
				p->drawLine(r.x()+4, r.y()+2, r.right()-1, r.y()+2);
				p->drawLine(r.x()+3, r.y()+3, r.right()-1, r.y()+3);

				p->fillRect(r.x()+2, r.y()+4, r.width()-3, r.height()-6,
							cg.brush(QColorGroup::Mid));

				p->setPen(cg.light());
				p->drawLine(r.x(), r.bottom()-1, r.right(), r.bottom()-1);
				p->translate(0,-1);
			}
			break;
		}


		// MENUBAR ITEM (sunken panel on mouse over)
		// -------------------------------------------------------------------
		case CE_MenuBarItem:
		{
			QMenuBar* mb = (QMenuBar*)widget;
			QRect pr = mb->rect();

			bool active  = flags & Style_Active;
			bool focused = flags & Style_HasFocus;

			if ( active && focused )
				qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(),
								cg, true, 1, &cg.brush(QColorGroup::Midlight));
			else
				renderGradient( p, r, cg.button(), false,
								r.x(), r.y()-1, pr.width()-2, pr.height()-2);

			QCommonStyle::drawControl(element, p, widget, r, cg, flags, opt);
			break;
		}


		// POPUPMENU ITEM
		// -------------------------------------------------------------------
		case CE_PopupMenuItem: {
			int x, y, w, h;
			r.rect( &x, &y, &w, &h );

			const QPopupMenu *popupmenu = (const QPopupMenu *) widget;
			QMenuItem *mi = opt.menuItem();
			if ( !mi )
				break;

			int  tab        = opt.tabWidth();
			int  checkcol   = QMAX( opt.maxIconWidth(), 20 );
			bool enabled    = mi->isEnabled();
			bool checkable  = popupmenu->isCheckable();
			bool active     = flags & Style_Active;
			bool reverse	= QApplication::reverseLayout();

			// Are we a menu item separator?
			if ( mi->isSeparator() ) {
				p->setPen( cg.dark() );
				p->drawLine( x, y, x+w, y );
				p->setPen( cg.light() );
				p->drawLine( x, y+1, x+w, y+1 );
				break;
			}

			// Draw the menu item background
			if ( active )
				qDrawShadePanel( p, x, y, w, h, cg, true, 1,
						         &cg.brush(QColorGroup::Midlight) );
			else
				p->fillRect( r, cg.button() );

			// Do we have an icon?
			if ( mi->iconSet() ) {
				QRect cr( reverse ? x+w - checkcol : x, y, checkcol, h );
				QIconSet::Mode mode;

				// Select the correct icon from the iconset
				if ( active )
					mode = enabled ? QIconSet::Active : QIconSet::Disabled;
				else
					mode = enabled ? QIconSet::Normal : QIconSet::Disabled;

				// Do we have an icon and are checked at the same time?
				// Then draw a "pressed" background behind the icon
				if ( checkable && !active && mi->isChecked() )
					qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(),
									 cg, true, 1, &cg.brush(QColorGroup::Midlight) );

				// Draw the icon
				QPixmap pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
				int pixw = pixmap.width();
				int pixh = pixmap.height();
				QRect pmr( 0, 0, pixw, pixh );
				pmr.moveCenter( cr.center() );
				p->setPen( cg.highlightedText() );
				p->drawPixmap( pmr.topLeft(), pixmap );
			}

			// Are we checked? (This time without an icon)
			else if ( checkable && mi->isChecked() ) {
				int cx = reverse ? x+w - checkcol : x;

				// We only have to draw the background if the menu item is inactive -
				// if it's active the "pressed" background is already drawn
				if ( ! active )
					qDrawShadePanel( p, cx, y, checkcol, h, cg, true, 1,
					                 &cg.brush(QColorGroup::Midlight) );

				// Draw the checkmark
				SFlags cflags = Style_Default;
				cflags |= active ? Style_Enabled : Style_On;

				drawPrimitive( PE_CheckMark, p, QRect( cx + itemFrame, y + itemFrame,
								checkcol - itemFrame*2, h - itemFrame*2), cg, cflags );
			}

			// Time to draw the menu item label...
			int xm = itemFrame + checkcol + itemHMargin; // X position margin
			int xp = reverse ?
					x + tab + rightBorder + itemHMargin + itemFrame :
					x + xm;
			int offset = reverse ? -1 : 1;	// Shadow offset for etched text

			// Set the color for enabled and disabled text (used for both active and inactive menu items)
			p->setPen( enabled ? cg.buttonText() : cg.mid() );

			// This color will be used instead of the above if the menu item
			// is active and disabled at the same time. (etched text)
			QColor discol = cg.mid();

			// Does the menu item draw it's own label?
			if ( mi->custom() ) {
				int m = itemVMargin;
				// Save the painter state in case the custom
				// paint method changes it in some way
				p->save();

				// Draw etched text if we're inactive and the menu item is disabled
				if ( !enabled && !active ) {
					p->setPen( cg.light() );
					mi->custom()->paint( p, cg, active, enabled, xp+offset, y+m+1, w-xm-tab+1, h-2*m );
					p->setPen( discol );
				}
				mi->custom()->paint( p, cg, active, enabled, xp, y+m, w-xm-tab+1, h-2*m );
				p->restore();
			}
			else {
				// The menu item doesn't draw it's own label
				QString s = mi->text();

				// Does the menu item have a text label?
				if ( !s.isNull() ) {
					int t = s.find( '\t' );
					int m = itemVMargin;
					const int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;

					// Does the menu item have a tabstop? (for the accelerator text)
					if ( t >= 0 ) {
						int tabx = reverse ? x + rightBorder + itemFrame :
							x + w - tab - rightBorder - itemHMargin - itemFrame;

						// Draw the right part of the label (accelerator text)
						if ( !enabled && !active ) {
							// Draw etched text if we're inactive and the menu item is disabled
							p->setPen( cg.light() );
							p->drawText( tabx+offset, y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ) );
							p->setPen( discol );
						}
						p->drawText( tabx, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
						s = s.left( t );
					}

					// Draw the left part of the label (or the whole label if there's no accelerator)
					if ( !enabled && !active ) {
						// Etched text again for inactive disabled menu items...
						p->setPen( cg.light() );
						p->drawText( xp+offset, y+m+1, w-xm-tab+1, h-2*m, text_flags, s, t );
						p->setPen( discol );
					}

					p->drawText( xp, y+m, w-xm-tab+1, h-2*m, text_flags, s, t );

				}

				// The menu item doesn't have a text label
				// Check if it has a pixmap instead
				else if ( mi->pixmap() ) {
					QPixmap *pixmap = mi->pixmap();

					// Draw the pixmap
					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( OpaqueMode );

					int diffw = ( ( w - pixmap->width() ) / 2 )
									+ ( ( w - pixmap->width() ) % 2 );
					p->drawPixmap( x+diffw, y+itemFrame, *pixmap );
					
					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( TransparentMode );
				}
			}

			// Does the menu item have a popup menu?
			if ( mi->popup() ) {
				PrimitiveElement arrow = reverse ? PE_ArrowLeft : PE_ArrowRight;
				int dim = (h-2*itemFrame) / 2;
				int xp  = reverse ? x + arrowHMargin + itemFrame :
					x + w - arrowHMargin - itemFrame - dim;

				// Draw an arrow at the far end of the menu item
				if ( active ) {
					if ( enabled )
						discol = cg.buttonText();

					QColorGroup g2( discol, cg.highlight(), white, white,
									enabled ? white : discol, discol, white );

					drawPrimitive( arrow, p, QRect(xp, y + h / 2 - dim / 2,dim, dim),
									g2, Style_Enabled );
				} else
					drawPrimitive( arrow, p, QRect(xp, y + h / 2 - dim / 2, dim, dim),
									cg, enabled ? Style_Enabled : Style_Default );
			}
			break;
		}


		default:
			QCommonStyle::drawControl(element, p, widget, r, cg, flags, opt);
	}
}


void HighColorStyle::drawControlMask( ControlElement element,
								  	  QPainter *p,
								  	  const QWidget *widget,
								  	  const QRect &r,
								  	  const QStyleOption& opt ) const
{
	switch (element)
	{
		// PUSHBUTTON MASK
		// ----------------------------------------------------------------------
		case CE_PushButton: {
			int x1, y1, x2, y2;
			r.coords( &x1, &y1, &x2, &y2 );
			QCOORD corners[] = { x1,y1, x2,y1, x1,y2, x2,y2 };
			p->fillRect( r, color1 );
			p->setPen( color0 );
			p->drawPoints( QPointArray(4, corners) );
			break;
		}

		default:
			QCommonStyle::drawControlMask(element, p, widget, r, opt);
	}
}


void HighColorStyle::drawComplexControl( ComplexControl control,
                                         QPainter *p,
                                         const QWidget *widget,
                                         const QRect &r,
                                         const QColorGroup &cg,
                                         SFlags flags,
									     SCFlags controls,
									     SCFlags active,
                                         const QStyleOption& opt ) const
{
	switch(control)
	{
		// COMBOBOX
		// -------------------------------------------------------------------
		case CC_ComboBox: {

			// Draw box and arrow
			if ( controls & SC_ComboBoxArrow ) {
				bool sunken = (active == SC_ComboBoxArrow);

				// Draw the combo
				int x,y,w,h;
				r.rect(&x, &y, &w, &h);

				int x2 = x+w-1;
				int y2 = y+h-1;

				p->setPen(cg.shadow());
				p->drawLine(x+1, y, x2-1, y);
				p->drawLine(x+1, y2, x2-1, y2);
				p->drawLine(x, y+1, x, y2-1);
				p->drawLine(x2, y+1, x2, y2-1);

				// Ensure the edge notches are properly colored
				p->setPen(cg.button());
				p->drawPoint(x,y);
				p->drawPoint(x,y2);
				p->drawPoint(x2,y);
				p->drawPoint(x2,y2);

				renderGradient( p, QRect(x+2, y+2, w-4, h-4),
								cg.button(), false);

				p->setPen(sunken ? cg.light() : cg.mid());
				p->drawLine(x2-1, y+2, x2-1, y2-1);
				p->drawLine(x+1, y2-1, x2-1, y2-1);

				p->setPen(sunken ? cg.mid() : cg.light());
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+2, x+1, y2-2);

				// Get the button bounding box
				QRect ar = QStyle::visualRect(
					querySubControlMetrics(CC_ComboBox, widget, SC_ComboBoxArrow),
					widget );

				// Are we enabled?
				if ( widget->isEnabled() )
					flags |= Style_Enabled;

				// Are we "pushed" ?
				if ( active & Style_Sunken )
					flags |= Style_Sunken;

				drawPrimitive(PE_ArrowDown, p, ar, cg, flags);
			}

			// Draw an edit field if required
			if ( controls & SC_ComboBoxEditField )
			{
				const QComboBox * cb = (const QComboBox *) widget;
				QRect re = QStyle::visualRect(
					querySubControlMetrics( CC_ComboBox, widget,
						                    SC_ComboBoxEditField), widget );

				// Draw the indent
				if (cb->editable()) {
					p->setPen( cg.dark() );
					p->drawLine( re.x(), re.y()-1, re.x()+re.width(), re.y()-1 );
					p->drawLine( re.x()-1, re.y(), re.x()-1, re.y()+re.height() );
				}

				if ( cb->hasFocus() ) {
					p->setPen( cg.highlightedText() );
					p->setBackgroundColor( cg.highlight() );
				} else {
					p->setPen( cg.text() );
					p->setBackgroundColor( cg.button() );
				}

				if ( cb->hasFocus() && !cb->editable() ) {
					// Draw the contents
					p->fillRect( re.x(), re.y(), re.width(), re.height(),
								 cg.brush( QColorGroup::Highlight ) );

					QRect re =
						QStyle::visualRect(subRect(SR_ComboBoxFocusRect, cb), widget);

					drawPrimitive( PE_FocusRect, p, re, cg,
								   Style_FocusAtBorder, QStyleOption(cg.highlight()));
				}
			}
			break;
		}

		// 3 BUTTON SCROLLBAR
		// -------------------------------------------------------------------
		case CC_ScrollBar: {
			const QScrollBar *sb = (const QScrollBar*)widget;

			QRect  addline, subline, subline2, addpage, subpage, slider, first, last;
			bool   maxedOut   = (sb->minValue()    == sb->maxValue());
			bool   horizontal = (sb->orientation() == Qt::Horizontal);
			SFlags sflags     = ((horizontal ? Style_Horizontal : Style_Default) |
								 (maxedOut   ? Style_Default : Style_Enabled));

			subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, opt);
			addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, opt);
			subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, opt);
			addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, opt);
			slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  opt);
			first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   opt);
			last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    opt);
			subline2 = addline;

			if (horizontal)
				subline2.moveBy(-addline.width(), 0);
			else
				subline2.moveBy(0, -addline.height());

			// Draw the up/left button set
			if ((controls & SC_ScrollBarSubLine) && subline.isValid()) {
				drawPrimitive(PE_ScrollBarSubLine, p, subline, cg,
							sflags | (active == SC_ScrollBarSubLine ?
								Style_Down : Style_Default));

				if (subline2.isValid())
					drawPrimitive(PE_ScrollBarSubLine, p, subline2, cg,
							sflags | (active == SC_ScrollBarSubLine ?
								Style_Down : Style_Default));
			}

			if ((controls & SC_ScrollBarAddLine) && addline.isValid())
				drawPrimitive(PE_ScrollBarAddLine, p, addline, cg,
							sflags | ((active == SC_ScrollBarAddLine) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
				drawPrimitive(PE_ScrollBarSubPage, p, subpage, cg,
							sflags | ((active == SC_ScrollBarSubPage) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
				drawPrimitive(PE_ScrollBarAddPage, p, addpage, cg,
							sflags | ((active == SC_ScrollBarAddPage) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarFirst) && first.isValid())
				drawPrimitive(PE_ScrollBarFirst, p, first, cg,
							sflags | ((active == SC_ScrollBarFirst) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarLast) && last.isValid())
				drawPrimitive(PE_ScrollBarLast, p, last, cg,
							sflags | ((active == SC_ScrollBarLast) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
				drawPrimitive(PE_ScrollBarSlider, p, slider, cg,
							sflags | ((active == SC_ScrollBarSlider) ?
										Style_Down : Style_Default));
				// Draw focus rect
				if (sb->hasFocus()) {
					QRect fr(slider.x() + 2, slider.y() + 2,
							 slider.width() - 5, slider.height() - 5);
					drawPrimitive(PE_FocusRect, p, fr, cg, Style_Default);
				}
			}
			break;
		}

		// TOOLBUTTON
		// -------------------------------------------------------------------
		case CC_ToolButton: {
			const QToolButton *toolbutton = (const QToolButton *) widget;

			QRect button, menuarea;
			button   = querySubControlMetrics(control, widget, SC_ToolButton, opt);
			menuarea = querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt);

			SFlags bflags = flags,
				   mflags = flags;

			if (active & SC_ToolButton)
				bflags |= Style_Down;
			if (active & SC_ToolButtonMenu)
				mflags |= Style_Down;

			if (controls & SC_ToolButton)
			{
				// If we're pressed, on, or raised...
				if (bflags & (Style_Down | Style_On | Style_Raised))
					drawPrimitive(PE_ButtonTool, p, button, cg, bflags, opt);

				// Check whether to draw a background pixmap
				else if ( toolbutton->parentWidget() &&
						  toolbutton->parentWidget()->backgroundPixmap() &&
						  !toolbutton->parentWidget()->backgroundPixmap()->isNull() )
				{
					QPixmap pixmap = *(toolbutton->parentWidget()->backgroundPixmap());
					p->drawTiledPixmap( r, pixmap, toolbutton->pos() );
				}
				else if (widget->parent() && widget->parent()->inherits("QToolBar"))
				{
					QToolBar* parent = (QToolBar*)widget->parent();
					QRect pr = parent->rect();

					renderGradient( p, r, cg.button(),
									parent->orientation() == Qt::Vertical,
									r.x(), r.y(), pr.width()-2, pr.height()-2);
				}
			}

			// Draw a toolbutton menu indicator if required
			if (controls & SC_ToolButtonMenu)
			{
				if (mflags & (Style_Down | Style_On | Style_Raised))
					drawPrimitive(PE_ButtonDropDown, p, menuarea, cg, mflags, opt);
				drawPrimitive(PE_ArrowDown, p, menuarea, cg, mflags, opt);
			}

			if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
				QRect fr = toolbutton->rect();
				fr.addCoords(3, 3, -3, -3);
				drawPrimitive(PE_FocusRect, p, fr, cg);
			}

			break;
		}

// ###### TODO
/*		case CC_Slider: {
			break;
		} */

		default:
			winstyle->drawComplexControl(control, p, widget,
							r, cg, flags, controls, active, opt);
			break;
	}
}


void HighColorStyle::drawComplexControlMask( ComplexControl control,
											 QPainter *p,
											 const QWidget *widget,
											 const QRect &r,
											 const QStyleOption& opt ) const
{
	switch (control)
	{
		// COMBOBOX MASK
		// -------------------------------------------------------------------
		case CC_ComboBox: {
			int x1, y1, x2, y2;
			r.coords( &x1, &y1, &x2, &y2 );
			QCOORD corners[] = { x1,y1, x2,y1, x1,y2, x2,y2 };
			p->fillRect( r, color1 );
			p->setPen( color0 );
			p->drawPoints( QPointArray(4, corners) );
			break;
		}

		default:
			QCommonStyle::drawComplexControlMask(control, p, widget, r, opt);
	}
}


QRect HighColorStyle::subRect(SubRect r, const QWidget *widget) const
{
	switch(r)
	{
		// We want the focus rect for buttons to be adjusted from
		// the Qt3 defaults to be similar to Qt 2's defaults.
		case SR_PushButtonFocusRect: {
			const QPushButton* button = (const QPushButton*) widget;

			QRect wrect(widget->rect());
			int dbw1 = 0, dbw2 = 0;

			if (button->isDefault() || button->autoDefault()) {
				dbw1 = pixelMetric(PM_ButtonDefaultIndicator, widget);
				dbw2 = dbw1 * 2;
			}

			int dfw1 = pixelMetric(PM_DefaultFrameWidth, widget) * 2,
				dfw2 = dfw1 * 2;

			return QRect(wrect.x()      + dfw1 + dbw1 + 1,
						 wrect.y()      + dfw1 + dbw1 + 1,
						 wrect.width()  - dfw2 - dbw2 - 1,
						 wrect.height() - dfw2 - dbw2 - 1);
		}


		default:
			return QCommonStyle::subRect(r, widget);
	}
}


int HighColorStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
	switch(m)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PM_ButtonMargin:				// Space btw. frame and label
			return 4;

		case PM_ButtonDefaultIndicator: {
			if ( highcolor )
				return 0;					// No indicator when highcolor
			else
				return 3;
		}

		case PM_ButtonShiftHorizontal:		// Offset by 1
		case PM_ButtonShiftVertical:		// ### Make configurable
			return 1;

		// CHECKBOXES / RADIO BUTTONS
		// -------------------------------------------------------------------
		case PM_ExclusiveIndicatorWidth:	// Radiobutton size
		case PM_ExclusiveIndicatorHeight:
		case PM_IndicatorWidth:				// Checkbox size
		case PM_IndicatorHeight: {
			return 13;						// 13x13
		}

		// TABS
		// --------------------------------------------------------------------
		case PM_TabBarTabHSpace:
			return 24;

		case PM_TabBarTabVSpace: {
			const QTabBar * tb = (const QTabBar *) widget;
			if ( tb->shape() == QTabBar::RoundedAbove ||
				 tb->shape() == QTabBar::RoundedBelow )
				return 10;
			else
				return 0;
		}

		case PM_TabBarTabOverlap: {
			const QTabBar* tb = (const QTabBar*)widget;
			if (tb->shape() != QTabBar::RoundedAbove)
				return 3;					// Leave standard size alone
			else
				return 0;					// Change size for our tabs only
		}
		
		// SLIDER
		// -------------------------------------------------------------------
		case PM_SliderLength:
			return 18;

		// PROGRESSBAR
		// -------------------------------------------------------------------
		case PM_ProgressBarChunkWidth:
			return 1;

		// FRAMES
		// -------------------------------------------------------------------
		case PM_MenuBarFrameWidth:
			return 1;

		case PM_DockWindowFrameWidth:
			return 1;
			
		// GENERAL
		// -------------------------------------------------------------------
		case PM_MaximumDragDistance:
			return -1;
			
		default:
			return winstyle->pixelMetric(m, widget);
	}
}


QSize HighColorStyle::sizeFromContents( ContentsType contents,
										const QWidget *widget,
										const QSize &contentSize,
										const QStyleOption& opt ) const
{
	switch (contents)
	{
		// PUSHBUTTON SIZE
		// ------------------------------------------------------------------
		case CT_PushButton: {
			const QPushButton *button = (const QPushButton *) widget;
			int w  = contentSize.width(), h = contentSize.height();
			int bm = pixelMetric( PM_ButtonMargin, widget );
			int fw = pixelMetric( PM_DefaultFrameWidth, widget ) * 2;

			w += bm + fw + 6;	// ### Add 6 to make way for bold font.
			h += bm + fw;

			// Ensure we stick to standard width and heights.
			if ( button->isDefault() || button->autoDefault() ) {
				if ( w < 80 && !button->pixmap() )
					w = 80;

				if ( ! highcolor ) {
					// Compensate for default indicator
					int di = pixelMetric( PM_ButtonDefaultIndicator );
					w += di * 2;
					h += di * 2;
				}
			}
				
			if ( h < 22 )
				h = 22;

			return QSize( w, h );
		}


		// POPUPMENU ITEM SIZE
		// -----------------------------------------------------------------
		case CT_PopupMenuItem: {
			if ( ! widget || opt.isDefault() )
				return contentSize;

			const QPopupMenu *popup = (const QPopupMenu *) widget;
			bool checkable = popup->isCheckable();
			QMenuItem *mi = opt.menuItem();
			int maxpmw = opt.maxIconWidth();
			int w = contentSize.width(), h = contentSize.height();

			if ( mi->custom() ) {
				w = mi->custom()->sizeHint().width();
				h = mi->custom()->sizeHint().height();
				if ( ! mi->custom()->fullSpan() )
					h += 2*itemVMargin + 2*itemFrame;
			}
			else if ( mi->widget() ) {
			} else if ( mi->isSeparator() ) {
				w = 10; // Arbitrary
				h = 2;
			}
			else {
				if ( mi->pixmap() )
					h = QMAX( h, mi->pixmap()->height() + 2*itemFrame );
				else
					h = QMAX( h, popup->fontMetrics().height()
							+ 2*itemVMargin + 2*itemFrame );

				if ( mi->iconSet() )
					h = QMAX( h, mi->iconSet()->pixmap(
								QIconSet::Small, QIconSet::Normal).height() +
								2 * itemFrame );
			}

			if ( ! mi->text().isNull() && mi->text().find('\t') >= 0 )
				w += 12;
			else if ( mi->popup() )
				w += 2 * arrowHMargin;

			if ( maxpmw )
				w += maxpmw + 6;
			if ( checkable && maxpmw < 20 )
				w += 20 - maxpmw;
			if ( checkable || maxpmw > 0 )
				w += 12;

			w += rightBorder;

			return QSize( w, h );
		}


		default:
			return QCommonStyle::sizeFromContents( contents, widget, contentSize, opt );
	}
}


QStyle::SubControl HighColorStyle::querySubControl( ComplexControl control,
                                                    const QWidget *widget,
                                                    const QPoint &pos,
                                                    const QStyleOption& opt ) const
{
	QStyle::SubControl ret =
		QCommonStyle::querySubControl(control, widget, pos, opt);

	// Enable third button
	if (control == CC_ScrollBar &&
		ret == SC_None)
		ret = SC_ScrollBarSubLine;

	return ret;
}


// Many thanks to Brad for contributing this code.
QRect HighColorStyle::querySubControlMetrics( ComplexControl control,
											  const QWidget *widget,
											  SubControl sc,
											  const QStyleOption& opt ) const
{
    QRect ret;

    switch (control)
	{
	    case CC_ScrollBar: {
			const QScrollBar *sb = (const QScrollBar*)widget;
			bool horizontal = sb->orientation() == Qt::Horizontal;
			int sliderstart = sb->sliderStart();
			int sbextent    = pixelMetric(PM_ScrollBarExtent, widget);
			int maxlen      = (horizontal ? sb->width() : sb->height()) - (sbextent*3);
			int sliderlen;

			// calculate slider length
			if (sb->maxValue() != sb->minValue())
			{
				uint range = sb->maxValue() - sb->minValue();
				sliderlen = (sb->pageStep() * maxlen) /	(range + sb->pageStep());

				int slidermin = pixelMetric( PM_ScrollBarSliderMin, widget );
				if ( sliderlen < slidermin || range > INT_MAX / 2 )
					sliderlen = slidermin;
				if ( sliderlen > maxlen )
					sliderlen = maxlen;
			} else
				sliderlen = maxlen;

			// Subcontrols
			switch (sc) {
				case SC_ScrollBarSubLine:
					// top/left button
					ret.setRect(0, 0, sbextent, sbextent);
					break;

				case SC_ScrollBarAddLine:
					// bottom/right button
					if (horizontal)
						ret.setRect(sb->width() - sbextent, 0, sbextent, sbextent);
					else
						ret.setRect(0, sb->height() - sbextent, sbextent, sbextent);
					break;

				case SC_ScrollBarSubPage:
					// between top/left button and slider
					if (horizontal)
						ret.setRect(sbextent, 0, sliderstart - sbextent, sbextent);
					else
						ret.setRect(0, sbextent, sbextent, sliderstart - sbextent);
					break;

				case SC_ScrollBarAddPage:
					// between bottom/right button and slider
					if (horizontal)
						ret.setRect(sliderstart + sliderlen, 0,
								maxlen - sliderstart - sliderlen + sbextent, sbextent);
					else
						ret.setRect(0, sliderstart + sliderlen, sbextent,
								maxlen - sliderstart - sliderlen + sbextent);
					break;

				case SC_ScrollBarGroove:
					if (horizontal)
						ret.setRect(sbextent, 0, sb->width() - sbextent * 3, sb->height());
					else
						ret.setRect(0, sbextent, sb->width(), sb->height() - sbextent * 3);
					break;

				case SC_ScrollBarSlider:
					if (horizontal)
						ret.setRect(sliderstart, 0, sliderlen, sbextent);
					else
						ret.setRect(0, sliderstart, sbextent, sliderlen);
					break;

				default:
					break;
			}
			break;
		}

		default:
			ret = winstyle->querySubControlMetrics(control, widget, sc, opt);

			break;
	}
	return ret;
}


// Fix Qt's wacky image alignment
QPixmap HighColorStyle::stylePixmap(StylePixmap stylepixmap,
									const QWidget *widget,
									const QStyleOption& opt) const
{
	switch (stylepixmap) {
		case SP_TitleBarMinButton:
			return QPixmap((const char **)hc_minimize_xpm);
		case SP_TitleBarCloseButton:
			return QPixmap((const char **)hc_close_xpm);
		default:
			break;
	}
	return winstyle->stylePixmap(stylepixmap, widget, opt);
}


int HighColorStyle::styleHint( StyleHint sh, const QWidget *w, const QStyleOption &opt, QStyleHintReturn *shr) const
{
	switch (sh)
	{
		// We don't want any spacing below the menu bar thanks
		case SH_MainWindow_SpaceBelowMenuBar:
			return 0;

		case SH_EtchDisabledText:
		case SH_Slider_SnapToValue:
		case SH_PrintDialog_RightAlignButtons:
		case SH_FontDialog_SelectAssociatedText:
		case SH_PopupMenu_AllowActiveAndDisabled:
		case SH_MenuBar_AltKeyNavigation:
		case SH_MenuBar_MouseTracking:
		case SH_PopupMenu_MouseTracking:
		case SH_ComboBox_ListMouseTracking:
		case SH_ItemView_ChangeHighlightOnFocus:
			return 1;

		case SH_PopupMenu_SubMenuPopupDelay:
			return 128;
			
		default:
			return QCommonStyle::styleHint(sh, w, opt, shr);
	}
}


void HighColorStyle::polishPopupMenu(QPopupMenu *p)
{
	if ( !p->testWState( WState_Polished ) )
		p->setCheckable( true );
}


bool HighColorStyle::eventFilter( QObject *object, QEvent *event )
{
	bool isMenuBar = object->inherits( "QMenuBar" );
	bool isToolBar = object->inherits( "QToolBar" );

	// Make the QMenuBar/QToolBar paintEvent() cover a larger area to 
	// ensure that the gradient contents are properly painted.
	// We essentially modify the paintEvent's rect to include the
	// panel border, which also paints the widget's interior.
	// This is nasty, but I see no other way to properly repaint 
	// gradients in all QMenuBars and QToolBars.
	// -- Karol.
	if ( isMenuBar || isToolBar ) 
	{
		if (event->type() == QEvent::Paint) 
		{
			bool horizontal = true;
			QPaintEvent* pe = (QPaintEvent*)event;
			QFrame* frame   = (QFrame*)object;
			QRect r = pe->rect();

			if ( isToolBar ) {
				QToolBar* tb = (QToolBar*)object;
				if (tb->orientation() == Qt::Vertical)
					horizontal = false;
			}

			if (horizontal) {
				if ( r.height() == frame->height() )
					return false;	// Let QFrame handle the painting now.
			
				// Else, send a new paint event with an updated paint rect.
				QPaintEvent dummyPE( QRect( r.x(), 0, r.width(), frame->height()) );
				QApplication::sendEvent( frame, &dummyPE ); 
			} 
			else {	// Vertical
				if ( r.width() == frame->width() )
					return false;	// Let QFrame handle the painting now.

				QPaintEvent dummyPE( QRect( 0, r.y(), frame->width(), r.height()) );
				QApplication::sendEvent( frame, &dummyPE ); 
			}
			
			// Discard this event as we sent a new paintEvent.
			return true;
		}
		return false; // We obviously can't be a pushbutton
	}

	// Handle push button hover effects.
	else if ( object-inherits("QPushButton") )
	{
		QPushButton* button = (QPushButton*) object;

		if ( (event->type() == QEvent::Enter) &&
			 (button->isEnabled()) ) {
			hoverWidget = button;
			button->repaint( false );
		} 
		else if ( (event->type() == QEvent::Leave) &&
				  (object == hoverWidget) ) {
			hoverWidget = 0L;
			button->repaint( false );
		}
	}
	
	return false;
}


void HighColorStyle::renderGradient( QPainter* p, const QRect& r,
	QColor clr, bool horizontal, int px, int py, int pwidth, int pheight) const
{
	// Make 8 bit displays happy
	if (!highcolor) {
		p->fillRect(r, clr);
		return;
	}

	// px, py specify the gradient pixmap offset relative to the top-left corner.
	// pwidth, pheight specify the width and height of the parent's pixmap.
	// We use these to draw parent-relative pixmaps for toolbar buttons
	// and menubar items.

	GradientSet* grSet = gDict.find( clr.rgb() );

	if (!grSet) {
		grSet = new GradientSet(clr);
		gDict.insert( clr.rgb(), grSet );
	}

	if (horizontal) {

		int width;

		if (pwidth != -1)
			width = pwidth; 	// Parent relative
		else
			width = r.width();

		if (width <= 34)
			p->drawTiledPixmap(r, *grSet->gradient(HMed), QPoint(px, 0));
		else if (width <= 52)
			p->drawTiledPixmap(r, *grSet->gradient(HLarge), QPoint(px, 0));
		else {
			KPixmap *hLarge = grSet->gradient(HLarge);

			// Don't draw a gradient if we don't need to
			if (hLarge->width() > px)
			{
				int pixmapWidth = hLarge->width() - px;

				// Draw the gradient
				p->drawTiledPixmap( r.x(), r.y(), pixmapWidth, r.height(),
									*hLarge, px, 0 );
				// Draw the remaining fill
				p->fillRect(r.x()+pixmapWidth, r.y(), r.width()-pixmapWidth, r.height(),
							clr.dark(110));

			} else
				p->fillRect(r, clr.dark(110));
		}

	} else {
		// Vertical gradient
		// -----------------

		int height;

		if (pheight != -1)
			height = pheight;	// Parent relative
		else
			height = r.height();

		if (height <= 24)
			p->drawTiledPixmap(r, *grSet->gradient(VSmall), QPoint(0, py));
		else if (height <= 34)
			p->drawTiledPixmap(r, *grSet->gradient(VMed), QPoint(0, py));
		else if (height <= 64)
			p->drawTiledPixmap(r, *grSet->gradient(VLarge), QPoint(0, py));
		else {
			KPixmap *vLarge = grSet->gradient(VLarge);

			// Only draw the upper gradient if we need to.
			if (vLarge->height() > py)
			{
				int pixmapHeight = vLarge->height() - py;

				// Draw the gradient
				p->drawTiledPixmap( r.x(), r.y(), r.width(), pixmapHeight,
									*vLarge, 0, py );
				// Draw the remaining fill
				p->fillRect(r.x(), r.y()+pixmapHeight, r.width(), r.height()-pixmapHeight,
							clr.dark(110));

			} else
				p->fillRect(r, clr.dark(110));
		}
	}
}


// vim: set noet ts=4 sw=4:

