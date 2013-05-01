/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2010, 2012 Martin Gräßlin <mgraesslin@kde.org>

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
#include "eglonxbackend.h"
// kwin
#include "options.h"
#include "overlaywindow.h"
#include "xcbutils.h"
// kwin libs
#include <kwinglplatform.h>
// KDE
#include <KDE/KDebug>

namespace KWin
{

EglOnXBackend::EglOnXBackend()
    : OpenGLBackend()
    , surfaceHasSubPost(0)
{
    init();
    // Egl is always direct rendering
    setIsDirectRendering(true);
}

EglOnXBackend::~EglOnXBackend()
{
    cleanupGL();
    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(dpy, ctx);
    eglDestroySurface(dpy, surface);
    eglTerminate(dpy);
    eglReleaseThread();
    if (overlayWindow()->window()) {
        overlayWindow()->destroy();
    }
}

void EglOnXBackend::init()
{
    if (!initRenderingContext()) {
        setFailed("Could not initialize rendering context");
        return;
    }

    initEGL();
    if (!hasGLExtension("EGL_KHR_image") &&
        (!hasGLExtension("EGL_KHR_image_base") ||
         !hasGLExtension("EGL_KHR_image_pixmap"))) {
        setFailed("Required support for binding pixmaps to EGLImages not found, disabling compositing");
        return;
    }
    GLPlatform *glPlatform = GLPlatform::instance();
    glPlatform->detect(EglPlatformInterface);
    glPlatform->printResults();
    initGL(EglPlatformInterface);
    if (!hasGLExtension("GL_OES_EGL_image")) {
        setFailed("Required extension GL_OES_EGL_image not found, disabling compositing");
        return;
    }

    // check for EGL_NV_post_sub_buffer and whether it can be used on the surface
    if (eglPostSubBufferNV) {
        if (eglQuerySurface(dpy, surface, EGL_POST_SUB_BUFFER_SUPPORTED_NV, &surfaceHasSubPost) == EGL_FALSE) {
            EGLint error = eglGetError();
            if (error != EGL_SUCCESS && error != EGL_BAD_ATTRIBUTE) {
                setFailed("query surface failed");
                return;
            } else {
                surfaceHasSubPost = EGL_FALSE;
            }
        }
    }
    if (surfaceHasSubPost) {
        kDebug(1212) << "EGL implementation and surface support eglPostSubBufferNV, let's use it";

        if (options->glPreferBufferSwap() != Options::NoSwapEncourage) {
            // check if swap interval 1 is supported
            EGLint val;
            eglGetConfigAttrib(dpy, config, EGL_MAX_SWAP_INTERVAL, &val);
            if (val >= 1) {
                if (eglSwapInterval(dpy, 1)) {
                    kDebug(1212) << "Enabled v-sync";
                    setHasWaitSync(true);
                }
            } else {
                kWarning(1212) << "Cannot enable v-sync as max. swap interval is" << val;
            }
        } else {
            // disable v-sync
            eglSwapInterval(dpy, 0);
        }
    } else {
        /* In the GLX backend, we fall back to using glCopyPixels if we have no extension providing support for partial screen updates.
         * However, that does not work in EGL - glCopyPixels with glDrawBuffer(GL_FRONT); does nothing.
         * Hence we need EGL to preserve the backbuffer for us, so that we can draw the partial updates on it and call
         * eglSwapBuffers() for each frame. eglSwapBuffers() then does the copy (no page flip possible in this mode),
         * which means it is slow and not synced to the v-blank. */
        kWarning(1212) << "eglPostSubBufferNV not supported, have to enable buffer preservation - which breaks v-sync and performance";
        eglSurfaceAttrib(dpy, surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);
    }
}

bool EglOnXBackend::initRenderingContext()
{
    dpy = eglGetDisplay(display());
    if (dpy == EGL_NO_DISPLAY)
        return false;
    EGLint major, minor;
    if (eglInitialize(dpy, &major, &minor) == EGL_FALSE)
        return false;
#ifdef KWIN_HAVE_OPENGLES
    eglBindAPI(EGL_OPENGL_ES_API);
#else
    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
        kError(1212) << "bind OpenGL API failed";
        return false;
    }
#endif
    initBufferConfigs();
    if (!overlayWindow()->create()) {
        kError(1212) << "Could not get overlay window";
        return false;
    } else {
        overlayWindow()->setup(None);
    }
    surface = eglCreateWindowSurface(dpy, config, overlayWindow()->window(), 0);

    const EGLint context_attribs[] = {
#ifdef KWIN_HAVE_OPENGLES
        EGL_CONTEXT_CLIENT_VERSION, 2,
#endif
        EGL_NONE
    };

    ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, context_attribs);
    if (ctx == EGL_NO_CONTEXT) {
        kError(1212) << "Create Context failed";
        return false;
    }
    if (eglMakeCurrent(dpy, surface, surface, ctx) == EGL_FALSE) {
        kError(1212) << "Make Context Current failed";
        return false;
    }
    kDebug(1212) << "EGL version: " << major << "." << minor;
    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
        kWarning(1212) << "Error occurred while creating context " << error;
        return false;
    }
    return true;
}

bool EglOnXBackend::initBufferConfigs()
{
    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE,         EGL_WINDOW_BIT|EGL_SWAP_BEHAVIOR_PRESERVED_BIT,
        EGL_RED_SIZE,             1,
        EGL_GREEN_SIZE,           1,
        EGL_BLUE_SIZE,            1,
        EGL_ALPHA_SIZE,           0,
#ifdef KWIN_HAVE_OPENGLES
        EGL_RENDERABLE_TYPE,      EGL_OPENGL_ES2_BIT,
#else
        EGL_RENDERABLE_TYPE,      EGL_OPENGL_BIT,
#endif
        EGL_CONFIG_CAVEAT,        EGL_NONE,
        EGL_NONE,
    };

    EGLint count;
    EGLConfig configs[1024];
    if (eglChooseConfig(dpy, config_attribs, configs, 1024, &count) == EGL_FALSE) {
        kError(1212) << "choose config failed";
        return false;
    }

    Xcb::WindowAttributes attribs(rootWindow());
    if (!attribs) {
        kError(1212) << "Failed to get window attributes of root window";
        return false;
    }

    config = configs[0];
    for (int i = 0; i < count; i++) {
        EGLint val;
        if (eglGetConfigAttrib(dpy, configs[i], EGL_NATIVE_VISUAL_ID, &val) == EGL_FALSE) {
            kError(1212) << "egl get config attrib failed";
        }
        if (uint32_t(val) == attribs->visual) {
            config = configs[i];
            break;
        }
    }
    return true;
}

void EglOnXBackend::present()
{
    const QRegion displayRegion(0, 0, displayWidth(), displayHeight());
    const bool fullRepaint = (lastDamage() == displayRegion);

    if (fullRepaint || !surfaceHasSubPost) {
        // the entire screen changed, or we cannot do partial updates (which implies we enabled surface preservation)
        eglSwapBuffers(dpy, surface);
    } else {
        // a part of the screen changed, and we can use eglPostSubBufferNV to copy the updated area
        foreach (const QRect & r, lastDamage().rects()) {
            eglPostSubBufferNV(dpy, surface, r.left(), displayHeight() - r.bottom() - 1, r.width(), r.height());
        }
    }

    setLastDamage(QRegion());
    eglWaitGL();
    xcb_flush(connection());
}

void EglOnXBackend::screenGeometryChanged(const QSize &size)
{
    Q_UNUSED(size)
    // no backend specific code needed
    // TODO: base implementation in OpenGLBackend
}

SceneOpenGL::TexturePrivate *EglOnXBackend::createBackendTexture(SceneOpenGL::Texture *texture)
{
    return new EglTexture(texture, this);
}

void EglOnXBackend::prepareRenderingFrame()
{
    if (!lastDamage().isEmpty())
        present();
    eglWaitNative(EGL_CORE_NATIVE_ENGINE);
    startRenderTimer();
}

void EglOnXBackend::endRenderingFrame(const QRegion &damage)
{
    setLastDamage(damage);
    glFlush();

    if (overlayWindow()->window())  // show the window only after the first pass,
        overlayWindow()->show();   // since that pass may take long
}

/************************************************
 * EglTexture
 ************************************************/

EglTexture::EglTexture(KWin::SceneOpenGL::Texture *texture, KWin::EglOnXBackend *backend)
    : SceneOpenGL::TexturePrivate()
    , q(texture)
    , m_backend(backend)
    , m_image(EGL_NO_IMAGE_KHR)
{
    m_target = GL_TEXTURE_2D;
}

EglTexture::~EglTexture()
{
    if (m_image != EGL_NO_IMAGE_KHR) {
        eglDestroyImageKHR(m_backend->dpy, m_image);
    }
}

OpenGLBackend *EglTexture::backend()
{
    return m_backend;
}

void EglTexture::findTarget()
{
    if (m_target != GL_TEXTURE_2D) {
        m_target = GL_TEXTURE_2D;
    }
}

bool EglTexture::loadTexture(const Pixmap &pix, const QSize &size, int depth)
{
    Q_UNUSED(depth)
    if (pix == None)
        return false;

    glGenTextures(1, &m_texture);
    q->setWrapMode(GL_CLAMP_TO_EDGE);
    q->setFilter(GL_LINEAR);
    q->bind();
    const EGLint attribs[] = {
        EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
        EGL_NONE
    };
    m_image = eglCreateImageKHR(m_backend->dpy, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR,
                                          (EGLClientBuffer)pix, attribs);

    if (EGL_NO_IMAGE_KHR == m_image) {
        kDebug(1212) << "failed to create egl image";
        q->unbind();
        q->discard();
        return false;
    }
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)m_image);
    q->unbind();
    checkGLError("load texture");
    q->setYInverted(true);
    m_size = size;
    return true;
}

void KWin::EglTexture::onDamage()
{
    if (options->isGlStrictBinding()) {
        // This is just implemented to be consistent with
        // the example in mesa/demos/src/egl/opengles1/texture_from_pixmap.c
        eglWaitNative(EGL_CORE_NATIVE_ENGINE);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES) m_image);
    }
    GLTexturePrivate::onDamage();
}

} // namespace
