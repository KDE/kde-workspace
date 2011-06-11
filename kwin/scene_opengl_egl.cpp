/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2010 Martin Gräßlin <kde@martin-graesslin.com>

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

// This file is included in scene_opengl.cpp
//#include "scene_opengl.h"
#include <QX11Info>

#ifdef KWIN_HAVE_WAYLAND
#include <wayland-server.h>
#include "wayland/wayland_client.h"
#endif

#include <sys/time.h>

namespace KWin
{

EGLDisplay dpy;
EGLConfig config;
EGLSurface surface;
EGLContext ctx;

SceneOpenGL::SceneOpenGL(Workspace* ws)
    : Scene(ws)
    , init_ok(false)
{
    if (!initRenderingContext())
        return;

    initEGL();
    if (!hasGLExtension("EGL_KHR_image_pixmap")) {
        kError(1212) << "Required extension EGL_KHR_image_pixmap not found, disabling compositing";
        return;
    }
    initGL();
    if (!hasGLExtension("GL_OES_EGL_image")) {
        kError(1212) << "Required extension GL_OES_EGL_image not found, disabling compositing";
        return;
    }
    debug = qstrcmp(qgetenv("KWIN_GL_DEBUG"), "1") == 0;
    if (!ShaderManager::instance()->isValid()) {
        kError(1212) << "Shaders not valid, ES compositing not possible";
        return;
    }
    ShaderManager::instance()->pushShader(ShaderManager::SimpleShader);

    if (checkGLError("Init")) {
        kError(1212) << "OpenGL compositing setup failed";
        return; // error
    }
    init_ok = true;
}

SceneOpenGL::~SceneOpenGL()
{
    if (!init_ok) {
        // TODO this probably needs to clean up whatever has been created until the failure
        wspace->destroyOverlay();
        return;
    }
    foreach (Window * w, windows)
    delete w;
    // do cleanup after initBuffer()
    cleanupGL();
    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(dpy, ctx);
    eglDestroySurface(dpy, surface);
    eglTerminate(dpy);
    eglReleaseThread();
    SceneOpenGL::EffectFrame::cleanup();
    checkGLError("Cleanup");
    if (wspace->overlayWindow()) {
        wspace->destroyOverlay();
    }
}

#ifdef KWIN_HAVE_WAYLAND
void SceneOpenGL::initWaylandEGL(wl_display *display)
{
    if (eglBindWaylandDisplayWL) {
        eglBindWaylandDisplayWL(dpy, display);
    }
}

void SceneOpenGL::releaseWaylandEGL(wl_display *display)
{
    if (eglUnbindWaylandDisplayWL) {
        eglUnbindWaylandDisplayWL(dpy, display);
    }
}
#endif

bool SceneOpenGL::initTfp()
{
    return false;
}

bool SceneOpenGL::initRenderingContext()
{
    dpy = eglGetDisplay(display());
    if (dpy == EGL_NO_DISPLAY)
        return false;
    EGLint major, minor;
    if (eglInitialize(dpy, &major, &minor) == EGL_FALSE)
        return false;
    eglBindAPI(EGL_OPENGL_ES_API);
    initBufferConfigs();
    if (!wspace->createOverlay()) {
        kError(1212) << "Could not get overlay window";
        return false;
    } else {
        wspace->setupOverlay(None);
    }
    surface = eglCreateWindowSurface(dpy, config, wspace->overlayWindow(), 0);

    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, context_attribs);
    if (ctx == EGL_NO_CONTEXT)
        return false;
    if (eglMakeCurrent(dpy, surface, surface, ctx) == EGL_FALSE)
        return false;
    kDebug(1212) << "EGL version: " << major << "." << minor;
    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
        kWarning(1212) << "Error occurred while creating context " << error;
        return false;
    }
    return true;
}

bool SceneOpenGL::initBuffer()
{
    return false;
}

bool SceneOpenGL::initBufferConfigs()
{
    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE,         EGL_WINDOW_BIT,
        EGL_RED_SIZE,             1,
        EGL_GREEN_SIZE,           1,
        EGL_BLUE_SIZE,            1,
        EGL_ALPHA_SIZE,           0,
        EGL_RENDERABLE_TYPE,      EGL_OPENGL_ES2_BIT,
        EGL_CONFIG_CAVEAT,        EGL_NONE,
        EGL_NONE,
    };

    EGLint count;
    EGLConfig configs[1024];
    eglChooseConfig(dpy, config_attribs, configs, 1024, &count);

    EGLint visualId = XVisualIDFromVisual((Visual*)QX11Info::appVisual());

    config = configs[0];
    for (int i = 0; i < count; i++) {
        EGLint val;
        eglGetConfigAttrib(dpy, configs[i], EGL_NATIVE_VISUAL_ID, &val);
        if (visualId == val) {
            config = configs[i];
            break;
        }
    }
    return true;
}

bool SceneOpenGL::initDrawableConfigs()
{
    return false;
}

// the entry function for painting
void SceneOpenGL::paint(QRegion damage, ToplevelList toplevels)
{
    QTime t = QTime::currentTime();
    foreach (Toplevel * c, toplevels) {
        assert(windows.contains(c));
        stacking_order.append(windows[ c ]);
    }
    grabXServer();
    XSync(display(), false);
    int mask = 0;
    paintScreen(&mask, &damage);   // call generic implementation
    ungrabXServer(); // ungrab before flushBuffer(), it may wait for vsync
    if (wspace->overlayWindow())  // show the window only after the first pass, since
        wspace->showOverlay();   // that pass may take long
    lastRenderTime = t.elapsed();
    flushBuffer(mask, damage);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    const int timeStamp = (tv.tv_sec * 1000 + tv.tv_usec / 1000);
    foreach (Scene::Window *w, stacking_order) {
        // inform all windows about the rendered frame
        w->frameRendered(timeStamp);
    }
    // do cleanup
    stacking_order.clear();
    checkGLError("PostPaint");
}

void SceneOpenGL::waitSync()
{
    // not used in EGL
}

void SceneOpenGL::flushBuffer(int mask, QRegion damage)
{
    Q_UNUSED(damage)
    glFlush();
    if (mask & PAINT_SCREEN_REGION) {
        // TODO: implement me properly
        eglSwapBuffers(dpy, surface);
    } else {
        eglSwapBuffers(dpy, surface);
    }
    eglWaitGL();
    // TODO: remove for wayland
    XFlush(display());
}

//****************************************
// SceneOpenGL::Texture
//****************************************

void SceneOpenGL::Texture::init()
{
    m_image = EGL_NO_IMAGE_KHR;
    findTarget();
}

void SceneOpenGL::Texture::release()
{
    if (m_image != EGL_NO_IMAGE_KHR) {
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)m_image);
    }
    mTexture = None;
}

void SceneOpenGL::Texture::findTarget()
{
    mTarget = GL_TEXTURE_2D;
}

bool SceneOpenGL::Texture::load(const Pixmap& pix, const QSize& size,
                                int depth, QRegion region)
{
    Q_UNUSED(size)
    Q_UNUSED(depth)
    Q_UNUSED(region)

    if (pix == None)
        return false;

    if (mTexture == None) {
        createTexture();
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        const EGLint attribs[] = {
            EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
            EGL_NONE
        };
        EGLImageKHR image = eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR,
                                              (EGLClientBuffer)pix, attribs);

        if (EGL_NO_IMAGE_KHR == image) {
            kDebug(1212) << "failed to create egl image";
            unbind();
            return false;
        }
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)image);
        eglDestroyImageKHR(dpy, image);
        unbind();
        checkGLError("load texture");
        setYInverted(true);
    }
    return true;
}

#ifdef KWIN_HAVE_WAYLAND
bool SceneOpenGL::Texture::load(wl_buffer *buffer)
{
    if (!buffer) {
        return false;
    }
    if (mTexture == None) {
        createTexture();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    bind();
    if (wl_buffer_is_shm(buffer)) {
        // create the image from shared memory
        glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT,
                        0, 0, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
        int pitch = wl_shm_buffer_get_stride(buffer) / 4;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT,
                        pitch, buffer->height, 0,
                        GL_BGRA_EXT, GL_UNSIGNED_BYTE,
                        wl_shm_buffer_get_data(buffer));
    } else {
        m_image = eglCreateImageKHR(dpy, NULL, EGL_WAYLAND_BUFFER_WL, buffer, NULL);

        if (m_image == EGL_NO_IMAGE_KHR) {
            kDebug(1212) << "failed to create Wayland egl image";
            unbind();
            return false;
        }
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)m_image);
    }
    unbind();
    checkGLError("load texture");
    setYInverted(true);
    return true;
}
#endif

void SceneOpenGL::Texture::bind()
{
    GLTexture::bind();
}

void SceneOpenGL::Texture::unbind()
{
    GLTexture::unbind();
}

//****************************************
// SceneOpenGL::Window
//****************************************

// Bind the window pixmap to an OpenGL texture.
bool SceneOpenGL::Window::bindTexture()
{
    if (toplevel->isXWindow()) {
        // Get the pixmap with the window contents
        Pixmap pix = toplevel->windowPixmap();
        if (pix == None)
            return false;
        bool success = texture.load(pix, toplevel->size(), toplevel->depth(),
                                    toplevel->damage());
        if (success)
            toplevel->resetDamage(QRect(toplevel->clientPos(), toplevel->clientSize()));
        else
            kDebug(1212) << "Failed to bind window";
        return success;
    } else if (toplevel->isWayland()) {
#ifdef KWIN_HAVE_WAYLAND
        bool success = texture.load(qobject_cast<Wayland::Client*>(toplevel)->buffer());
        if (success) {
            toplevel->resetDamage(QRect(toplevel->clientPos(), toplevel->clientSize()));
        } else {
            kDebug(1212) << "Failed to bind Wayland window";
        }
        return success;
#else
        return false;
#endif
    } else {
        return false;
    }
}

} // namespace
