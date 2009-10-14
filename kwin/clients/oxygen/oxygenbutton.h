#ifndef oxygenbutton_h
#define oxygenbutton_h

//////////////////////////////////////////////////////////////////////////////
// oxygenbutton.h
// -------------------
//
// Copyright (c) 2006, 2007 Riccardo Iaconelli <riccardo@kde.org>
// Copyright (c) 2006, 2007 Casper Boemann <cbr@boemann.dk>
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include <kcommondecoration.h>
#include <QtCore/QTimeLine>

#include "oxygen.h"

namespace Oxygen
{
  class OxygenClient;

  enum ButtonStatus {
    Normal,
    Hovered,
    Pressed
  };

  Q_DECLARE_FLAGS(ButtonState, ButtonStatus)

  class OxygenButton : public KCommonDecorationButton
  {

    Q_OBJECT

    public:

    //! flags
    enum RenderFlag
    {

      Background = 1<<0,
      Deco = 1<<1,
      Icon = 1<<2,
      All = Background|Deco|Icon

    };

    Q_DECLARE_FLAGS(RenderFlags, RenderFlag)

    //! constructor
    explicit OxygenButton(OxygenClient &parent,
      const QString &tip=NULL,
      ButtonType type=ButtonHelp,
      RenderFlags flags = All);

    //! destructor
    ~OxygenButton();

    //! destructor
    QSize sizeHint() const;

    //! reset
    void reset(long unsigned int)
    {repaint();}

    //! button type
    ButtonType type( void ) const
    { return type_; }

    //! set force inactive
    /*! returns true if value was actually changed */
    void setForceInactive( const bool& value )
    { forceInactive_ = value; }

    protected:

    //! press event
    void mousePressEvent(QMouseEvent* );

    //! release event
    void mouseReleaseEvent(QMouseEvent* );

    //! enter event
    void enterEvent( QEvent* );

    //! leave event
    void leaveEvent(QEvent* );

    //! paint
    void paintEvent(QPaintEvent* );

    //! draw icon
    void drawIcon(QPainter*, QPalette&, ButtonType& );

    //! color
    QColor buttonDetailColor(const QPalette& );

    //! color
    QColor buttonDetailColor(const QPalette&, bool active );

    //! opacity
    qreal opacity( void ) const
    { return qreal( timeLine_.currentFrame() ) / qreal(timeLine_.endFrame()); }

    //! true if timeline is running
    bool timeLineIsRunning( void ) const
    { return timeLine_.state() == QTimeLine::Running; }

    //! true if button is active
    bool isActive( void ) const;

    protected slots:

    //! update
    void setDirty( void )
    { update(); }

    private:

    //! flags
    RenderFlags renderFlags_;

    //! parent client
    const OxygenClient &client_;

    //! helper
    OxygenHelper &helper_;

    //! button type
    ButtonType type_;

    //! button status
    ButtonState status_;

    //! true if button should be forced inactive
    bool forceInactive_;

    //! true if color cache is to be reseted
    QColor cachedButtonDetailColor_;

    //! timeline used for smooth transitions
    QTimeLine timeLine_;

  };

} //namespace Oxygen

#endif
