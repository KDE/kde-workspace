/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "screenlocker.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <KStandardDirs>
#ifdef KWIN_HAVE_OPENGL
#include <kwinglutils.h>
#endif
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
#include <kwinxrenderutils.h>
#endif

namespace KWin
{

KWIN_EFFECT(screenlocker, ScreenLockerEffect)

class EffectDeclarativeView : public QDeclarativeView
{
public:
    void windowInputMouseEvent(QEvent *e)
    {
        QMouseEvent *ev = static_cast<QMouseEvent *>(e);

        if (e->type() == QEvent::MouseMove) {
            mouseMoveEvent(ev);
        } else if (e->type() == QEvent::MouseButtonPress) {
            mousePressEvent(ev);
        } else if (e->type() == QEvent::MouseButtonDblClick) {
            mouseDoubleClickEvent(ev);
        } else if (e->type() == QEvent::MouseButtonRelease) {
            mouseReleaseEvent(ev);
        }
    }
};

ScreenLockerEffect::ScreenLockerEffect()
    : Effect()
    , m_locked(false)
    , m_inputWindow(0)
{
    connect(effects, SIGNAL(requestScreenLock()), SLOT(slotRequestLock()));
    m_declarativeView = new EffectDeclarativeView;
    foreach(const QString &importPath, KGlobal::dirs()->findDirs("module", "imports")) {
        m_declarativeView->engine()->addImportPath(importPath);
    }
    m_declarativeView->setSource(QUrl(KStandardDirs::locate("data", "kwin/lockscreen/main.qml")));
    m_declarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    m_declarativeView->setWindowFlags(Qt::X11BypassWindowManagerHint);
    m_declarativeView->setAttribute(Qt::WA_TranslucentBackground);
    m_declarativeView->setFrameShape(QFrame::NoFrame);

    connect(m_declarativeView->rootObject(), SIGNAL(unlockRequested()), this, SLOT(slotRequestUnlock()));
    //m_declarativeView->hide();
}

ScreenLockerEffect::~ScreenLockerEffect()
{
    delete m_declarativeView;
}

void ScreenLockerEffect::paintScreen(int mask, QRegion region, ScreenPaintData &data)
{
    effects->paintScreen(mask, region, data);
    if (m_locked) {
        // TODO: do our screen locking
    }
}

void ScreenLockerEffect::postPaintScreen()
{
    if (m_locked) {
        // paint screen black
        // TODO: add cross fading
        if (effects->compositingType() == OpenGLCompositing) {
            paintGL();
        } else if (effects->compositingType() == XRenderCompositing) {
            paintXrender();
        }
        EffectWindow *w = effects->findWindow(m_declarativeView->winId());
        if (w) {
            WindowPaintData d(w);
            effects->drawWindow(w, PAINT_WINDOW_OPAQUE, infiniteRegion(), d);
        }
    }
}

void ScreenLockerEffect::paintGL()
{
#ifdef KWIN_HAVE_OPENGL
    GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
    vbo->reset();
    vbo->setUseColor(true);
    if (ShaderManager::instance()->isValid()) {
        ShaderManager::instance()->pushShader(ShaderManager::ColorShader);
    }
    vbo->setColor(Qt::black);
    QVector<float> verts;
    const QRect r(0, 0, displayWidth(), displayHeight());
    verts.reserve(12);
    verts << r.x() + r.width() << r.y();
    verts << r.x() << r.y();
    verts << r.x() << r.y() + r.height();
    verts << r.x() << r.y() + r.height();
    verts << r.x() + r.width() << r.y() + r.height();
    verts << r.x() + r.width() << r.y();
    vbo->setData(6, 2, verts.data(), NULL);
    vbo->render(GL_TRIANGLES);
    if (ShaderManager::instance()->isValid()) {
        ShaderManager::instance()->popShader();
    }
#endif
}

void ScreenLockerEffect::paintXrender()
{
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
    XRenderColor col;
    col.alpha = 1;
    col.red = 0;
    col.green = 0;
    col.blue = 0;
    XRenderFillRectangle(display(), PictOpOver, effects->xrenderBufferPicture(),
                         &col, 0, 0, displayWidth(), displayHeight());
#endif
}

void ScreenLockerEffect::slotRequestLock()
{
    if (!effects->isScreenLockerReferenced()) {
        m_declarativeView->setGeometry(effects->clientArea(FullScreenArea, effects->activeScreen(), effects->currentDesktop()));
        m_declarativeView->show(); 
        // create input window and grab mouse
        Window w = effects->createFullScreenInputWindow(this, Qt::PointingHandCursor);
        if (!w) {
            return;
        }
        if (!effects->grabKeyboard(this)) {
            effects->destroyInputWindow(w);
            return;
        }
        effects->refScreenLocker(this);
        effects->setActiveFullScreenEffect(this);
        m_locked = true;
        m_inputWindow = w;
        
        effects->addRepaintFull();
    }
}

void ScreenLockerEffect::grabbedKeyboardEvent(QKeyEvent *e)
{
    if (!m_locked) {
        return;
    }
    // TODO: implement a proper unlock behavior
    if (e->key() == Qt::Key_Space) {
        // magic key for testing
        doUnlock();
    }
}

void ScreenLockerEffect::slotRequestUnlock()
{
    doUnlock();
}

void ScreenLockerEffect::windowInputMouseEvent(Window w, QEvent *e)
{
    Q_UNUSED(w)
    Q_UNUSED(e)
    if (!m_locked) {
        return;
    }

    QMouseEvent* me = static_cast< QMouseEvent* >(e);
    QMouseEvent event(me->type(), me->pos(), me->pos(), me->button(), me->buttons(), me->modifiers());
    m_declarativeView->windowInputMouseEvent(&event);
    // TODO: implement proper behavior by triggering the unlock screen
}

void ScreenLockerEffect::doUnlock()
{
    m_locked = false;
    effects->destroyInputWindow(m_inputWindow);
    m_inputWindow = 0;
    effects->setActiveFullScreenEffect(NULL);
    effects->ungrabKeyboard();
    effects->unrefScreenLocker(this);
    m_declarativeView->hide();
    effects->addRepaintFull();
}

bool ScreenLockerEffect::isActive() const
{
    return m_locked;
}

bool ScreenLockerEffect::provides(Effect::Feature feature)
{
    if (feature == ScreenLocking) {
        return true;
    }
    return false;
}

} //namespace
