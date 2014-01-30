/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2012 Filip Wieladek <wattos@gmail.com>

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

#include "mouseclick.h"
// KConfigSkeleton
#include "mouseclickconfig.h"

#include <kwinglutils.h>

#ifdef KWIN_HAVE_XRENDER_COMPOSITING
#include <kwinxrenderutils.h>
#include <xcb/xcb.h>
#include <xcb/render.h>
#endif

#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KConfigGroup>

#include <math.h>

namespace KWin
{

KWIN_EFFECT(mouseclick, MouseClickEffect)

MouseClickEffect::MouseClickEffect()
{
    m_enabled = false;
    KActionCollection* actionCollection = new KActionCollection(this);
    KAction* a = static_cast<KAction*>(actionCollection->addAction("ToggleMouseClick"));
    a->setText(i18n("Toggle Effect"));
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Asterisk));
    connect(a, SIGNAL(triggered(bool)), this, SLOT(toggleEnabled()));
    reconfigure(ReconfigureAll);

    m_buttons[0] = new MouseButton(i18n("Left"), Qt::LeftButton);
    m_buttons[1] = new MouseButton(i18n("Middle"), Qt::MiddleButton);
    m_buttons[2] = new MouseButton(i18n("Right"), Qt::RightButton);
}

MouseClickEffect::~MouseClickEffect()
{
    if (m_enabled)
        effects->stopMousePolling();
    foreach (const MouseEvent* click, m_clicks) {
        delete click;
    }
    m_clicks.clear();

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        delete m_buttons[i];
    }
}

void MouseClickEffect::reconfigure(ReconfigureFlags)
{
    MouseClickConfig::self()->readConfig();
    m_colors[0] = MouseClickConfig::color1();
    m_colors[1] = MouseClickConfig::color2();
    m_colors[2] = MouseClickConfig::color3();
    m_lineWidth = MouseClickConfig::lineWidth();
    m_ringLife = MouseClickConfig::ringLife();
    m_ringMaxSize = MouseClickConfig::ringSize();
    m_ringCount = MouseClickConfig::ringCount();
    m_showText = MouseClickConfig::showText();
    m_font = MouseClickConfig::font();
}

void MouseClickEffect::prePaintScreen(ScreenPrePaintData& data, int time)
{
    foreach (MouseEvent* click, m_clicks) {
        click->m_time += time;
    }

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        if (m_buttons[i]->m_isPressed) {
            m_buttons[i]->m_time += time;
        }
    }

    while (m_clicks.size() > 0) {
        MouseEvent* first = m_clicks[0];
        if (first->m_time <= m_ringLife) {
            break;
        }
        m_clicks.pop_front();
        delete first;
    }

    effects->prePaintScreen(data, time);
}

void MouseClickEffect::paintScreen(int mask, QRegion region, ScreenPaintData& data)
{
    effects->paintScreen(mask, region, data);

    paintScreenSetup(mask, region, data);
    foreach (const MouseEvent* click, m_clicks) {
        for (int i = 0; i < m_ringCount; ++i) {
            float alpha = computeAlpha(click, i);
            float size = computeRadius(click, i);
            if (size > 0 && alpha > 0) {
                QColor color = m_colors[click->m_button];
                color.setAlphaF(alpha);
                drawCircle(color, click->m_pos.x(), click->m_pos.y(), size);
            }
        }

        if (m_showText && click->m_frame) {
            float frameAlpha = (click->m_time * 2.0f - m_ringLife) / m_ringLife;
            frameAlpha = frameAlpha < 0 ? 1 : -(frameAlpha * frameAlpha) + 1;
            click->m_frame->render(infiniteRegion(), frameAlpha, frameAlpha);
        }
    }
    paintScreenFinish(mask, region, data);
}

void MouseClickEffect::postPaintScreen()
{
    effects->postPaintScreen();
    repaint();
}

float MouseClickEffect::computeRadius(const MouseEvent* click, int ring)
{
    float ringDistance = m_ringLife / (m_ringCount * 3);
    if (click->m_press) {
        return ((click->m_time - ringDistance * ring) / m_ringLife) * m_ringMaxSize;
    }
    return ((m_ringLife - click->m_time - ringDistance * ring) / m_ringLife) * m_ringMaxSize;
}

float MouseClickEffect::computeAlpha(const MouseEvent* click, int ring)
{
    float ringDistance = m_ringLife / (m_ringCount * 3);
    return (m_ringLife - (float)click->m_time - ringDistance * (ring)) / m_ringLife;
}

void MouseClickEffect::slotMouseChanged(const QPoint& pos, const QPoint&,
                                        Qt::MouseButtons buttons, Qt::MouseButtons oldButtons,
                                        Qt::KeyboardModifiers, Qt::KeyboardModifiers)
{
    if (buttons == oldButtons)
        return;

    MouseEvent* m = NULL;
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        MouseButton* b = m_buttons[i];
        if (isPressed(b->m_button, buttons, oldButtons)) {
            m = new MouseEvent(i, pos, 0, createEffectFrame(pos, b->m_labelDown), true);
        } else if (isReleased(b->m_button, buttons, oldButtons) && (!b->m_isPressed || b->m_time > m_ringLife)) {
            // we might miss a press, thus also check !b->m_isPressed, bug #314762
            m = new MouseEvent(i, pos, 0, createEffectFrame(pos, b->m_labelUp), false);
        }
        b->setPressed(b->m_button & buttons);
    }

    if (m) {
        m_clicks.append(m);
    }
    repaint();
}

EffectFrame* MouseClickEffect::createEffectFrame(const QPoint& pos, const QString& text) {
    if (!m_showText) {
        return NULL;
    }
    QPoint point(pos.x() + m_ringMaxSize, pos.y());
    EffectFrame* frame = effects->effectFrame(EffectFrameStyled, false, point, Qt::AlignLeft);
    frame->setFont(m_font);
    frame->setText(text);
    return frame;
}

void MouseClickEffect::repaint()
{
    if (m_clicks.size() > 0) {
        QRegion dirtyRegion;
        const int radius = m_ringMaxSize + m_lineWidth;
        foreach (MouseEvent* click, m_clicks) {
            dirtyRegion |= QRect(click->m_pos.x() - radius, click->m_pos.y() - radius, 2*radius, 2*radius);
            if (click->m_frame) {
                // we grant the plasma style 32px padding for stuff like shadows...
                dirtyRegion |= click->m_frame->geometry().adjusted(-32,-32,32,32);
            }
        }
        effects->addRepaint(dirtyRegion);
    }
}

bool MouseClickEffect::isReleased(Qt::MouseButtons button, Qt::MouseButtons buttons, Qt::MouseButtons oldButtons)
{
    return !(button & buttons) && (button & oldButtons);
}

bool MouseClickEffect::isPressed(Qt::MouseButtons button, Qt::MouseButtons buttons, Qt::MouseButtons oldButtons)
{
    return (button & buttons) && !(button & oldButtons);
}

void MouseClickEffect::toggleEnabled()
{
    m_enabled = !m_enabled;

    if (m_enabled) {
        connect(effects, SIGNAL(mouseChanged(QPoint,QPoint,Qt::MouseButtons,Qt::MouseButtons,Qt::KeyboardModifiers,Qt::KeyboardModifiers)),
                         SLOT(slotMouseChanged(QPoint,QPoint,Qt::MouseButtons,Qt::MouseButtons,Qt::KeyboardModifiers,Qt::KeyboardModifiers)));
        effects->startMousePolling();
    } else {
        disconnect(effects, SIGNAL(mouseChanged(QPoint,QPoint,Qt::MouseButtons,Qt::MouseButtons,Qt::KeyboardModifiers,Qt::KeyboardModifiers)),
                   this, SLOT(slotMouseChanged(QPoint,QPoint,Qt::MouseButtons,Qt::MouseButtons,Qt::KeyboardModifiers,Qt::KeyboardModifiers)));
        effects->stopMousePolling();
    }

    if (m_clicks.size() > 0) {
        foreach (const MouseEvent* click, m_clicks) {
            delete click;
        }
    }
    m_clicks.clear();

    for (int i = 0; i < BUTTON_COUNT; ++i) {
        m_buttons[i]->m_time = 0;
        m_buttons[i]->m_isPressed = false;
    }
}

bool MouseClickEffect::isActive() const
{
    return m_enabled && (m_clicks.size() > 0);
}

void MouseClickEffect::drawCircle(const QColor& color, float cx, float cy, float r)
{
    if (effects->isOpenGLCompositing())
        drawCircleGl(color, cx, cy, r);
    if (effects->compositingType() == XRenderCompositing)
        drawCircleXr(color, cx, cy, r);
}

void MouseClickEffect::paintScreenSetup(int mask, QRegion region, ScreenPaintData& data)
{
    if (effects->isOpenGLCompositing())
        paintScreenSetupGl(mask, region, data);
}

void MouseClickEffect::paintScreenFinish(int mask, QRegion region, ScreenPaintData& data)
{
    if (effects->isOpenGLCompositing())
        paintScreenFinishGl(mask, region, data);
}

void MouseClickEffect::drawCircleGl(const QColor& color, float cx, float cy, float r)
{
    static int num_segments = 80;
    static float theta = 2 * 3.1415926 / float(num_segments);
    static float c = cosf(theta); //precalculate the sine and cosine
    static float s = sinf(theta);
    float t;

    float x = r;//we start at angle = 0
    float y = 0;

    GLVertexBuffer* vbo = GLVertexBuffer::streamingBuffer();
    vbo->reset();
    vbo->setUseColor(true);
    vbo->setColor(color);
    QVector<float> verts;
    verts.reserve(num_segments * 2);

    for (int ii = 0; ii < num_segments; ++ii) {
        verts << x + cx << y + cy;//output vertex
        //apply the rotation matrix
        t = x;
        x = c * x - s * y;
        y = s * t + c * y;
    }
    vbo->setData(verts.size() / 2, 2, verts.data(), NULL);
    vbo->render(GL_LINE_LOOP);
}

void MouseClickEffect::drawCircleXr(const QColor& color, float cx, float cy, float r)
{
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
    if (r <= m_lineWidth)
        return;

    int num_segments = r+8;
    float theta = 2.0 * 3.1415926 / num_segments;
    float cos = cosf(theta); //precalculate the sine and cosine
    float sin = sinf(theta);
    float x[2] = {r, r-m_lineWidth};
    float y[2] = {0, 0};

#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))
    QVector<xcb_render_pointfix_t> strip;
    strip.reserve(2*num_segments+2);

    xcb_render_pointfix_t point;
    point.x = DOUBLE_TO_FIXED(x[1]+cx);
    point.y = DOUBLE_TO_FIXED(y[1]+cy);
    strip << point;

    for (int i = 0; i < num_segments; ++i) {
        //apply the rotation matrix
        const float h[2] = {x[0], x[1]};
        x[0] = cos * x[0] - sin * y[0];
        x[1] = cos * x[1] - sin * y[1];
        y[0] = sin * h[0] + cos * y[0];
        y[1] = sin * h[1] + cos * y[1];

        point.x = DOUBLE_TO_FIXED(x[0]+cx);
        point.y = DOUBLE_TO_FIXED(y[0]+cy);
        strip << point;

        point.x = DOUBLE_TO_FIXED(x[1]+cx);
        point.y = DOUBLE_TO_FIXED(y[1]+cy);
        strip << point;
    }

    const float h = x[0];
    x[0] = cos * x[0] - sin * y[0];
    y[0] = sin * h    + cos * y[0];

    point.x = DOUBLE_TO_FIXED(x[0]+cx);
    point.y = DOUBLE_TO_FIXED(y[0]+cy);
    strip << point;

    XRenderPicture fill = xRenderFill(color);
    xcb_render_tri_strip(connection(), XCB_RENDER_PICT_OP_OVER,
                          fill, effects->xrenderBufferPicture(), 0,
                          0, 0, strip.count(), strip.constData());
#undef DOUBLE_TO_FIXED
#else
    Q_UNUSED(color)
    Q_UNUSED(cx)
    Q_UNUSED(cy)
    Q_UNUSED(r)
#endif
}

void MouseClickEffect::paintScreenSetupGl(int, QRegion, ScreenPaintData&)
{
    if (ShaderManager::instance()->isValid()) {
        ShaderManager::instance()->pushShader(ShaderManager::ColorShader);
    }

    glLineWidth(m_lineWidth);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MouseClickEffect::paintScreenFinishGl(int, QRegion, ScreenPaintData&)
{
    glDisable(GL_BLEND);

    if (ShaderManager::instance()->isValid()) {
        ShaderManager::instance()->popShader();
    }
}

} // namespace

#include "mouseclick.moc"
