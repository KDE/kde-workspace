/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>

Based on glcompmgr code by Felix Bellaby.
Using code from Compiz and Beryl.

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


/*
 This is the OpenGL-based compositing code. It is the primary and most powerful
 compositing backend.
 
Sources and other compositing managers:
=======================================

- http://opengl.org
    - documentation
        - OpenGL Redbook (http://opengl.org/documentation/red_book/ - note it's only version 1.1)
        - GLX docs (http://opengl.org/documentation/specs/glx/glx1.4.pdf)
        - extensions docs (http://www.opengl.org/registry/)

- glcompmgr
    - http://lists.freedesktop.org/archives/xorg/2006-July/017006.html ,
    - http://www.mail-archive.com/compiz%40lists.freedesktop.org/msg00023.html
    - simple and easy to understand
    - works even without texture_from_pixmap extension
    - claims to support several different gfx cards
    - compile with something like
      "gcc -Wall glcompmgr-0.5.c `pkg-config --cflags --libs glib-2.0` -lGL -lXcomposite -lXdamage -L/usr/X11R6/lib"

- compiz
    - git clone git://anongit.freedesktop.org/git/xorg/app/compiz
    - the ultimate <whatever>
    - glxcompmgr
        - git clone git://anongit.freedesktop.org/git/xorg/app/glxcompgr
        - a rather old version of compiz, but also simpler and as such simpler
            to understand

- beryl
    - a fork of Compiz
    - http://beryl-project.org
    - git clone git://anongit.beryl-project.org/beryl/beryl-core (or beryl-plugins etc. ,
        the full list should be at git://anongit.beryl-project.org/beryl/)

- libcm (metacity)
    - cvs -d :pserver:anonymous@anoncvs.gnome.org:/cvs/gnome co libcm
    - not much idea about it, the model differs a lot from KWin/Compiz/Beryl
    - does not seem to be very powerful or with that much development going on

*/

#include "scene_opengl.h"

#include <kxerrorhandler.h>

#include "utils.h"
#include "client.h"
#include "deleted.h"
#include "effects.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>

// turns on checks for opengl errors in various places (for easier finding of them)
// normally only few of them are enabled
//#define CHECK_GL_ERROR

#ifdef KWIN_HAVE_OPENGL_COMPOSITING

#include <X11/extensions/Xcomposite.h>

#include <qpainter.h>

namespace KWin
{

//****************************************
// SceneOpenGL
//****************************************

// the configs used for the destination
GLXFBConfig SceneOpenGL::fbcbuffer_db;
GLXFBConfig SceneOpenGL::fbcbuffer_nondb;
// the configs used for windows
SceneOpenGL::FBConfigInfo SceneOpenGL::fbcdrawableinfo[ 32 + 1 ];
// GLX content
GLXContext SceneOpenGL::ctxbuffer;
GLXContext SceneOpenGL::ctxdrawable;
// the destination drawable where the compositing is done
GLXDrawable SceneOpenGL::glxbuffer = None;
GLXDrawable SceneOpenGL::last_pixmap = None;
bool SceneOpenGL::tfp_mode; // using glXBindTexImageEXT (texture_from_pixmap)
bool SceneOpenGL::db; // destination drawable is double-buffered
bool SceneOpenGL::shm_mode;
#ifdef HAVE_XSHM
XShmSegmentInfo SceneOpenGL::shm;
#endif


SceneOpenGL::SceneOpenGL( Workspace* ws )
    : Scene( ws )
    , init_ok( false )
    , selfCheckDone( false )
    {
    if( !Extensions::glxAvailable())
        {
        kDebug( 1212 ) << "No glx extensions available";
        return; // error
        }
    initGLX();
    // check for FBConfig support
    if( !hasGLExtension( "GLX_SGIX_fbconfig" ) || !glXGetFBConfigAttrib || !glXGetFBConfigs ||
            !glXGetVisualFromFBConfig || !glXCreatePixmap || !glXDestroyPixmap ||
            !glXCreateWindow || !glXDestroyWindow )
        {
        kError( 1212 ) << "GLX_SGIX_fbconfig or required GLX functions missing";
        return; // error
        }
    if( !selectMode())
        return; // error
    if( !initBuffer()) // create destination buffer
        return; // error
    if( !initRenderingContext())
        return; // error
    // Initialize OpenGL
    initGL();
    if( !hasGLExtension( "GL_ARB_texture_non_power_of_two" )
        && !hasGLExtension( "GL_ARB_texture_rectangle" ))
        {
        kError( 1212 ) << "GL_ARB_texture_non_power_of_two and GL_ARB_texture_rectangle missing";
        return; // error
        }
    if( db )
        glDrawBuffer( GL_BACK );
    // Check whether certain features are supported
    has_waitSync = false;
    if( glXGetVideoSync && glXIsDirect( display(), ctxbuffer ) && options->glVSync )
        {
        unsigned int sync;
        if( glXGetVideoSync( &sync ) == 0 )
            {
            if( glXWaitVideoSync( 1, 0, &sync ) == 0 )
                has_waitSync = true;
            }
        }

    // OpenGL scene setup
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    float fovy = 60.0f;
    float aspect = 1.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;
    float ymax = zNear * tan( fovy  * M_PI / 360.0f );
    float ymin = -ymax;
    float xmin =  ymin * aspect;
    float xmax = ymax * aspect;
    // swap top and bottom to have OpenGL coordinate system match X system
    glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    float scaleFactor = 1.1 * tan( fovy * M_PI / 360.0f )/ymax;
    glTranslatef( xmin*scaleFactor, ymax*scaleFactor, -1.1 );
    glScalef( (xmax-xmin)*scaleFactor/displayWidth(), -(ymax-ymin)*scaleFactor/displayHeight(), 0.001 );
    if( checkGLError( "Init" ))
        {
        kError( 1212 ) << "OpenGL compositing setup failed";
        return; // error
        }
// selfcheck is broken (see Bug 253357)
#if 0
    // Do self-check immediatelly during compositing setup only when it's not KWin startup
    // at the same time (in other words, only when activating compositing using the kcm).
    // Currently selfcheck causes bad flicker (due to X mapping the overlay window
    // for too long?) which looks bad during KDE startup.
    if( !initting )
        {
        if( !selfCheck())
            return;
        selfCheckDone = true;
        }
#endif
    kDebug( 1212 ) << "DB:" << db << ", TFP:" << tfp_mode << ", SHM:" << shm_mode
        << ", Direct:" << bool( glXIsDirect( display(), ctxbuffer )) << endl;
    init_ok = true;
    }

SceneOpenGL::~SceneOpenGL()
    {
    if( !init_ok )
        {
        // TODO this probably needs to clean up whatever has been created until the failure
        wspace->destroyOverlay();
        return;
        }
    foreach( Window* w, windows )
        delete w;
    // do cleanup after initBuffer()
    glXMakeCurrent( display(), None, NULL );
    glXDestroyContext( display(), ctxbuffer );
    if( wspace->overlayWindow())
        {
        if( hasGLXVersion( 1, 3 ))
            glXDestroyWindow( display(), glxbuffer );
        XDestroyWindow( display(), buffer );
        wspace->destroyOverlay();
        }
    else
        {
        glXDestroyPixmap( display(), glxbuffer );
        XFreeGC( display(), gcroot );
        XFreePixmap( display(), buffer );
        }
    if( shm_mode )
        cleanupShm();
    if( !tfp_mode && !shm_mode )
        {
        if( last_pixmap != None )
            glXDestroyPixmap( display(), last_pixmap );
        glXDestroyContext( display(), ctxdrawable );
        }
    EffectFrame::cleanup();
    checkGLError( "Cleanup" );
    }

bool SceneOpenGL::initFailed() const
    {
    return !init_ok;
    }

bool SceneOpenGL::selectMode()
    {
    // select mode - try TFP first, then SHM, otherwise fallback mode
    shm_mode = false;
    tfp_mode = false;
    if( options->glMode == Options::GLTFP )
        {
        if( initTfp())
            tfp_mode = true;
        else if( initShm())
            shm_mode = true;
        }
    else if( options->glMode == Options::GLSHM )
        {
        if( initShm())
            shm_mode = true;
        else if( initTfp())
            tfp_mode = true;
        }
    if( !initDrawableConfigs())
        return false;
    return true;
    }

bool SceneOpenGL::initTfp()
    {
    if( glXBindTexImageEXT == NULL || glXReleaseTexImageEXT == NULL )
        return false;
    return true;
    }

bool SceneOpenGL::initShm()
    {
#ifdef HAVE_XSHM
    int major, minor;
    Bool pixmaps;
    if( !XShmQueryVersion( display(), &major, &minor, &pixmaps ) || !pixmaps )
        return false;
    if( XShmPixmapFormat( display()) != ZPixmap )
        return false;
    const int MAXSIZE = 4096 * 2048 * 4; // TODO check there are not larger windows
    // TODO check that bytes_per_line doesn't involve padding?
    shm.readOnly = False;
    shm.shmid = shmget( IPC_PRIVATE, MAXSIZE, IPC_CREAT | 0600 );
    if( shm.shmid < 0 )
        return false;
    shm.shmaddr = ( char* ) shmat( shm.shmid, NULL, 0 );
    if( shm.shmaddr == ( void * ) -1 )
        {
        shmctl( shm.shmid, IPC_RMID, 0 );
        return false;
        }
#ifdef __linux__
    // mark as deleted to automatically free the memory in case
    // of a crash (but this doesn't work e.g. on Solaris ... oh well)
    shmctl( shm.shmid, IPC_RMID, 0 );
#endif
    KXErrorHandler errs;
    XShmAttach( display(), &shm );
    if( errs.error( true ))
        {
#ifndef __linux__
        shmctl( shm.shmid, IPC_RMID, 0 );
#endif
        shmdt( shm.shmaddr );
        return false;
        }
    return true;
#else
    return false;
#endif
    }

void SceneOpenGL::cleanupShm()
    {
#ifdef HAVE_XSHM
    shmdt( shm.shmaddr );
#ifndef __linux__
    shmctl( shm.shmid, IPC_RMID, 0 );
#endif
#endif
    }

bool SceneOpenGL::initRenderingContext()
    {
    bool direct_rendering = options->glDirect;
    if( !tfp_mode && !shm_mode )
        direct_rendering = false; // fallback doesn't seem to work with direct rendering
    KXErrorHandler errs1;
    ctxbuffer = glXCreateNewContext( display(), fbcbuffer, GLX_RGBA_TYPE, NULL,
        direct_rendering ? GL_TRUE : GL_FALSE );
    bool failed = ( ctxbuffer == NULL || !glXMakeCurrent( display(), glxbuffer, ctxbuffer ));
    if( errs1.error( true )) // always check for error( having it all in one if() could skip
        failed = true;       // it due to evaluation short-circuiting
    if( failed )
        {
        if( !direct_rendering )
            {
            kDebug( 1212 ).nospace() << "Couldn't initialize rendering context ("
                << KXErrorHandler::errorMessage( errs1.errorEvent()) << ")";
            return false;
            }
	glXMakeCurrent( display(), None, NULL );
        if( ctxbuffer != NULL )
            glXDestroyContext( display(), ctxbuffer );
        direct_rendering = false; // try again
        KXErrorHandler errs2;
        ctxbuffer = glXCreateNewContext( display(), fbcbuffer, GLX_RGBA_TYPE, NULL, GL_FALSE );
        bool failed = ( ctxbuffer == NULL || !glXMakeCurrent( display(), glxbuffer, ctxbuffer ));
        if( errs2.error( true ))
            failed = true;
        if( failed )
            {
            kDebug( 1212 ).nospace() << "Couldn't initialize rendering context ("
                << KXErrorHandler::errorMessage( errs2.errorEvent()) << ")";
            return false;
            }
        }
    if( !tfp_mode && !shm_mode )
        {
        ctxdrawable = glXCreateNewContext( display(), fbcdrawableinfo[ QX11Info::appDepth() ].fbconfig, GLX_RGBA_TYPE, ctxbuffer,
            direct_rendering ? GL_TRUE : GL_FALSE );
        }
    return true;
    }

// create destination buffer
bool SceneOpenGL::initBuffer()
    {
    if( !initBufferConfigs())
        return false;
    if( fbcbuffer_db != NULL && wspace->createOverlay())
        { // we have overlay, try to create double-buffered window in it
        fbcbuffer = fbcbuffer_db;
        XVisualInfo* visual = glXGetVisualFromFBConfig( display(), fbcbuffer );
        XSetWindowAttributes attrs;
        attrs.colormap = XCreateColormap( display(), rootWindow(), visual->visual, AllocNone );
        buffer = XCreateWindow( display(), wspace->overlayWindow(), 0, 0, displayWidth(), displayHeight(),
            0, visual->depth, InputOutput, visual->visual, CWColormap, &attrs );
        if( hasGLXVersion( 1, 3 ))
            glxbuffer = glXCreateWindow( display(), fbcbuffer, buffer, NULL );
        else
            glxbuffer = buffer;
        wspace->setupOverlay( buffer );
        db = true;
        XFree( visual );
        }
    else if( fbcbuffer_nondb != NULL )
        { // cannot get any double-buffered drawable, will double-buffer using a pixmap
        fbcbuffer = fbcbuffer_nondb;
        XVisualInfo* visual = glXGetVisualFromFBConfig( display(), fbcbuffer );
        XGCValues gcattr;
        gcattr.subwindow_mode = IncludeInferiors;
        gcroot = XCreateGC( display(), rootWindow(), GCSubwindowMode, &gcattr );
        buffer = XCreatePixmap( display(), rootWindow(), displayWidth(), displayHeight(),
            visual->depth );
        glxbuffer = glXCreatePixmap( display(), fbcbuffer, buffer, NULL );
        db = false;
        XFree( visual );
        }
    else
        {
        kError( 1212 ) << "Couldn't create output buffer (failed to create overlay window?) !";
        return false; // error
        }
    int vis_buffer;
    glXGetFBConfigAttrib( display(), fbcbuffer, GLX_VISUAL_ID, &vis_buffer );
    XVisualInfo* visinfo_buffer = glXGetVisualFromFBConfig( display(), fbcbuffer );
    kDebug( 1212 ) << "Buffer visual (depth " << visinfo_buffer->depth << "): 0x" << QString::number( vis_buffer, 16 );
    XFree( visinfo_buffer );
    return true;
    }

// choose the best configs for the destination buffer
bool SceneOpenGL::initBufferConfigs()
    {
    int cnt;
    GLXFBConfig *fbconfigs = glXGetFBConfigs( display(), DefaultScreen( display() ), &cnt );
    fbcbuffer_db = NULL;
    fbcbuffer_nondb = NULL;

    for( int i = 0; i < 2; i++ )
        {
        int back, stencil, depth, caveat, alpha;
        back = i > 0 ? INT_MAX : 1;
        stencil = INT_MAX;
        depth = INT_MAX;
        caveat = INT_MAX;
        alpha = 0;
        for( int j = 0; j < cnt; j++ )
            {
            XVisualInfo *vi;
            int visual_depth;
            vi = glXGetVisualFromFBConfig( display(), fbconfigs[ j ] );
            if( vi == NULL )
                continue;
            visual_depth = vi->depth;
            XFree( vi );
            if( visual_depth != DefaultDepth( display(), DefaultScreen( display())))
                continue;
            int value;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_ALPHA_SIZE, &alpha );
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_BUFFER_SIZE, &value );
            if( value != visual_depth && ( value - alpha ) != visual_depth )
                continue;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_RENDER_TYPE, &value );
            if( !( value & GLX_RGBA_BIT ))
                continue;
            int back_value;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_DOUBLEBUFFER, &back_value );
            if( i > 0 )
                {
                if( back_value > back )
                    continue;
                }
            else
                {
                if( back_value < back )
                    continue;
                }
            int stencil_value;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_STENCIL_SIZE, &stencil_value );
            if( stencil_value > stencil )
                continue;
            int depth_value;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_DEPTH_SIZE, &depth_value );
            if( depth_value > depth )
                continue;
            int caveat_value;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_CONFIG_CAVEAT, &caveat_value );
            if( caveat_value > caveat )
                continue;
            back = back_value;
            stencil = stencil_value;
            depth = depth_value;
            caveat = caveat_value;
            if( i > 0 )
                fbcbuffer_nondb = fbconfigs[ j ];
            else
                fbcbuffer_db = fbconfigs[ j ];
            }
        }
    if( cnt )
        XFree( fbconfigs );
    if( fbcbuffer_db == NULL && fbcbuffer_nondb == NULL )
        {
        kError( 1212 ) << "Couldn't find framebuffer configuration for buffer!";
        return false;
        }
    for( int i = 0; i <= 32; i++ )
        {
        if( fbcdrawableinfo[ i ].fbconfig == NULL )
            continue;
        int vis_drawable = 0;
        glXGetFBConfigAttrib( display(), fbcdrawableinfo[ i ].fbconfig, GLX_VISUAL_ID, &vis_drawable );
        kDebug( 1212 ) << "Drawable visual (depth " << i << "): 0x" << QString::number( vis_drawable, 16 );
        }
    return true;
    }

// make a list of the best configs for windows by depth
bool SceneOpenGL::initDrawableConfigs()
    {
    int cnt;
    GLXFBConfig *fbconfigs = glXGetFBConfigs( display(), DefaultScreen( display() ), &cnt );

    for( int i = 0; i <= 32; i++ )
        {
        int back, stencil, depth, caveat, alpha, mipmap, rgba;
        back = INT_MAX;
        stencil = INT_MAX;
        depth = INT_MAX;
        caveat = INT_MAX;
        mipmap = 0;
        rgba = 0;
        fbcdrawableinfo[ i ].fbconfig = NULL;
        fbcdrawableinfo[ i ].bind_texture_format = 0;
        fbcdrawableinfo[ i ].texture_targets = 0;
        fbcdrawableinfo[ i ].y_inverted = 0;
        fbcdrawableinfo[ i ].mipmap = 0;
        for( int j = 0; j < cnt; j++ )
            {
            XVisualInfo *vi;
            int visual_depth;
            vi = glXGetVisualFromFBConfig( display(), fbconfigs[ j ] );
            if( vi == NULL )
                continue;
            visual_depth = vi->depth;
            XFree( vi );
            if( visual_depth != i )
                continue;
            int value;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_ALPHA_SIZE, &alpha );
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_BUFFER_SIZE, &value );
            if( value != i && ( value - alpha ) != i )
                continue;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_RENDER_TYPE, &value );
            if( !( value & GLX_RGBA_BIT ))
                continue;
            if( tfp_mode )
                {
                value = 0;
                if( i == 32 )
                    {
                    glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                          GLX_BIND_TO_TEXTURE_RGBA_EXT, &value );
                    if( value )
                        {
                        // TODO I think this should be set only after the config passes all tests
                        rgba = 1;
                        fbcdrawableinfo[ i ].bind_texture_format = GLX_TEXTURE_FORMAT_RGBA_EXT;
                        }
                    }
                if( !value )
                    {
                    if( rgba )
                        continue;
                    glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                          GLX_BIND_TO_TEXTURE_RGB_EXT, &value );
                    if( !value )
                        continue;
                    fbcdrawableinfo[ i ].bind_texture_format = GLX_TEXTURE_FORMAT_RGB_EXT;
                    }
                }
            int back_value;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_DOUBLEBUFFER, &back_value );
            if( back_value > back )
                continue;
            int stencil_value;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_STENCIL_SIZE, &stencil_value );
            if( stencil_value > stencil )
                continue;
            int depth_value;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_DEPTH_SIZE, &depth_value );
            if( depth_value > depth )
                continue;
            int mipmap_value = -1;
            if( tfp_mode && GLTexture::framebufferObjectSupported())
                {
                glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                      GLX_BIND_TO_MIPMAP_TEXTURE_EXT, &mipmap_value );
                if( mipmap_value < mipmap )
                    continue;
                }
            int caveat_value;
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_CONFIG_CAVEAT, &caveat_value );
            if( caveat_value > caveat )
                continue;
            // ok, config passed all tests, it's the best one so far
            fbcdrawableinfo[ i ].fbconfig = fbconfigs[ j ];
            caveat = caveat_value;
            back = back_value;
            stencil = stencil_value;
            depth = depth_value;
            mipmap = mipmap_value;
            if ( tfp_mode )
                {
                glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                      GLX_BIND_TO_TEXTURE_TARGETS_EXT, &value );
                fbcdrawableinfo[ i ].texture_targets = value;
                }
            glXGetFBConfigAttrib( display(), fbconfigs[ j ],
                                  GLX_Y_INVERTED_EXT, &value );
            fbcdrawableinfo[ i ].y_inverted = value;
            fbcdrawableinfo[ i ].mipmap = mipmap;
            }
        }
    if( cnt )
        XFree( fbconfigs );
    if( fbcdrawableinfo[ DefaultDepth( display(), DefaultScreen( display())) ].fbconfig == NULL )
        {
        kError( 1212 ) << "Couldn't find framebuffer configuration for default depth!";
        return false;
        }
    if( fbcdrawableinfo[ 32 ].fbconfig == NULL )
        {
        kError( 1212 ) << "Couldn't find framebuffer configuration for depth 32 (no ARGB GLX visual)!";
        return false;
        }
    return true;
    }

// Test if compositing actually _really_ works, by creating a texture from a testing
// window, drawing it on the screen, reading the contents back and comparing. This
// should test whether compositing really works.
// This function does the whole selfcheck, it can be done also in two parts
// during actual drawing (to avoid flicker, see selfCheck() call from the ctor).
bool SceneOpenGL::selfCheck()
    {
    QRegion reg = selfCheckRegion();
    if( wspace->overlayWindow())
        { // avoid covering the whole screen too soon
        wspace->setOverlayShape( reg );
        wspace->showOverlay();
        }
    selfCheckSetup();
    flushBuffer( PAINT_SCREEN_REGION, reg );
    bool ok = selfCheckFinish();
    if( wspace->overlayWindow())
        wspace->hideOverlay();
    return ok;
    }

void SceneOpenGL::selfCheckSetup()
    {
    KXErrorHandler err;
    QImage img( selfCheckWidth(), selfCheckHeight(), QImage::Format_RGB32 );
    img.setPixel( 0, 0, QColor( Qt::red ).rgb());
    img.setPixel( 1, 0, QColor( Qt::green ).rgb());
    img.setPixel( 2, 0, QColor( Qt::blue ).rgb());
    img.setPixel( 0, 1, QColor( Qt::white ).rgb());
    img.setPixel( 1, 1, QColor( Qt::black ).rgb());
    img.setPixel( 2, 1, QColor( Qt::white ).rgb());
    QPixmap pix = QPixmap::fromImage( img );
    foreach( const QPoint& p, selfCheckPoints())
        {
        XSetWindowAttributes wa;
        wa.override_redirect = True;
        ::Window window = XCreateWindow( display(), rootWindow(), 0, 0, selfCheckWidth(), selfCheckHeight(),
            0, QX11Info::appDepth(), CopyFromParent, CopyFromParent, CWOverrideRedirect, &wa );
        XSetWindowBackgroundPixmap( display(), window, pix.handle());
        XClearWindow( display(), window );
        XMapWindow( display(), window );
        // move the window one down to where the result will be rendered too, just in case
        // the render would fail completely and eventual check would try to read this window's contents
        XMoveWindow( display(), window, p.x() + 1, p.y());
        XCompositeRedirectWindow( display(), window, CompositeRedirectAutomatic );
        Pixmap wpix = XCompositeNameWindowPixmap( display(), window );
        glXWaitX();
        Texture texture;
        texture.load( wpix, QSize( selfCheckWidth(), selfCheckHeight()), QX11Info::appDepth());
        texture.bind();
        QRect rect( p.x(), p.y(), selfCheckWidth(), selfCheckHeight());
        texture.render( infiniteRegion(), rect );
        texture.unbind();
        glXWaitGL();
        XFreePixmap( display(), wpix );
        XDestroyWindow( display(), window );
        }
    err.error( true ); // just sync and discard
    }

bool SceneOpenGL::selfCheckFinish()
    {
    glXWaitGL();
    KXErrorHandler err;
    bool ok = true;
    foreach( const QPoint& p, selfCheckPoints())
        {
        QPixmap pix = QPixmap::grabWindow( rootWindow(), p.x(), p.y(), selfCheckWidth(), selfCheckHeight());
        QImage img = pix.toImage();
//        kDebug(1212) << "P:" << QColor( img.pixel( 0, 0 )).name();
//        kDebug(1212) << "P:" << QColor( img.pixel( 1, 0 )).name();
//        kDebug(1212) << "P:" << QColor( img.pixel( 2, 0 )).name();
//        kDebug(1212) << "P:" << QColor( img.pixel( 0, 1 )).name();
//        kDebug(1212) << "P:" << QColor( img.pixel( 1, 1 )).name();
//        kDebug(1212) << "P:" << QColor( img.pixel( 2, 1 )).name();
        if( img.pixel( 0, 0 ) != QColor( Qt::red ).rgb()
            || img.pixel( 1, 0 ) != QColor( Qt::green ).rgb()
            || img.pixel( 2, 0 ) != QColor( Qt::blue ).rgb()
            || img.pixel( 0, 1 ) != QColor( Qt::white ).rgb()
            || img.pixel( 1, 1 ) != QColor( Qt::black ).rgb()
            || img.pixel( 2, 1 ) != QColor( Qt::white ).rgb())
            {
            kError( 1212 ) << "OpenGL compositing self-check failed, disabling compositing.";
            ok = false;
            break;
            }
        }
    if( err.error( true ))
        ok = false;
    if( ok )
        kDebug( 1212 ) << "OpenGL compositing self-check passed.";
    if( !ok && options->disableCompositingChecks )
        {
        kWarning( 1212 ) << "Compositing checks disabled, proceeding regardless of self-check failure.";
        return true;
        }
    return ok;
    }

// the entry function for painting
void SceneOpenGL::paint( QRegion damage, ToplevelList toplevels )
    {
    foreach( Toplevel* c, toplevels )
        {
        assert( windows.contains( c ));
        stacking_order.append( windows[ c ] );
        }
    grabXServer();
    glXWaitX();
    glPushMatrix();
    int mask = 0;
#ifdef CHECK_GL_ERROR
    checkGLError( "Paint1" );
#endif
    paintScreen( &mask, &damage ); // call generic implementation
#ifdef CHECK_GL_ERROR
    checkGLError( "Paint2" );
#endif
    glPopMatrix();
    ungrabXServer(); // ungrab before flushBuffer(), it may wait for vsync
    if( wspace->overlayWindow()) // show the window only after the first pass, since
        wspace->showOverlay();   // that pass may take long
// selfcheck is broken (see Bug 253357)
#if 0
    if( !selfCheckDone )
        {
        selfCheckSetup();
        damage |= selfCheckRegion();
        }
#endif
    flushBuffer( mask, damage );
#if 0
    if( !selfCheckDone )
        {
        if( !selfCheckFinish())
            QTimer::singleShot( 0, Workspace::self(), SLOT( finishCompositing()));
        selfCheckDone = true;
        }
#endif
    // do cleanup
    stacking_order.clear();
    checkGLError( "PostPaint" );
    }

// wait for vblank signal before painting
void SceneOpenGL::waitSync()
    { // NOTE that vsync has no effect with indirect rendering
    if( waitSyncAvailable())
        {
        unsigned int sync;

        glFlush();
        glXGetVideoSync( &sync );
        glXWaitVideoSync( 2, ( sync + 1 ) % 2, &sync );
        }
    }

// actually paint to the screen (double-buffer swap or copy from pixmap buffer)
void SceneOpenGL::flushBuffer( int mask, QRegion damage )
    {
    if( db )
        {
        if( mask & PAINT_SCREEN_REGION )
            {
            waitSync();
            if( glXCopySubBuffer )
                {
                foreach( const QRect &r, damage.rects())
                    {
                    // convert to OpenGL coordinates
                    int y = displayHeight() - r.y() - r.height();
                    glXCopySubBuffer( display(), glxbuffer, r.x(), y, r.width(), r.height());
                    }
                }
            else
                { // no idea why glScissor() is used, but Compiz has it and it doesn't seem to hurt
                glEnable( GL_SCISSOR_TEST );
                glDrawBuffer( GL_FRONT );
                int xpos = 0;
                int ypos = 0;
                foreach( const QRect &r, damage.rects())
                    {
                    // convert to OpenGL coordinates
                    int y = displayHeight() - r.y() - r.height();
                    // Move raster position relatively using glBitmap() rather
                    // than using glRasterPos2f() - the latter causes drawing
                    // artefacts at the bottom screen edge with some gfx cards
//                    glRasterPos2f( r.x(), r.y() + r.height());
                    glBitmap( 0, 0, 0, 0, r.x() - xpos, y - ypos, NULL );
                    xpos = r.x();
                    ypos = y;
                    glScissor( r.x(), y, r.width(), r.height());
                    glCopyPixels( r.x(), y, r.width(), r.height(), GL_COLOR );
                    }
                glBitmap( 0, 0, 0, 0, -xpos, -ypos, NULL ); // move position back to 0,0
                glDrawBuffer( GL_BACK );
                glDisable( GL_SCISSOR_TEST );
                }
            }
        else
            {
            waitSync();
            glXSwapBuffers( display(), glxbuffer );
            }
        glXWaitGL();
        XFlush( display());
        }
    else
        {
        glFlush();
        glXWaitGL();
        waitSync();
        if( mask & PAINT_SCREEN_REGION )
            foreach( const QRect &r, damage.rects())
                XCopyArea( display(), buffer, rootWindow(), gcroot, r.x(), r.y(), r.width(), r.height(), r.x(), r.y());
        else
            XCopyArea( display(), buffer, rootWindow(), gcroot, 0, 0, displayWidth(), displayHeight(), 0, 0 );
        XFlush( display());
        }
    }

void SceneOpenGL::paintGenericScreen( int mask, ScreenPaintData data )
    {
    if( mask & PAINT_SCREEN_TRANSFORMED )
        { // apply screen transformations
        glPushMatrix();
        glTranslatef( data.xTranslate, data.yTranslate, data.zTranslate );
        if( data.rotation )
            {
            // translate to rotation point, rotate, translate back
            glTranslatef( data.rotation->xRotationPoint, data.rotation->yRotationPoint, data.rotation->zRotationPoint );
            float xAxis = 0.0;
            float yAxis = 0.0;
            float zAxis = 0.0;
            switch( data.rotation->axis )
                {
                case RotationData::XAxis:
                    xAxis = 1.0;
                    break;
                case RotationData::YAxis:
                    yAxis = 1.0;
                    break;
                case RotationData::ZAxis:
                    zAxis = 1.0;
                    break;
                }
            glRotatef( data.rotation->angle, xAxis, yAxis, zAxis );
            glTranslatef( -data.rotation->xRotationPoint, -data.rotation->yRotationPoint, -data.rotation->zRotationPoint );
            }
        glScalef( data.xScale, data.yScale, data.zScale );
        }
    Scene::paintGenericScreen( mask, data );
    if( mask & PAINT_SCREEN_TRANSFORMED )
        glPopMatrix();
    }

void SceneOpenGL::paintBackground( QRegion region )
    {
    PaintClipper pc( region );
    if( !PaintClipper::clip())
        {
        glPushAttrib( GL_COLOR_BUFFER_BIT );
        glClearColor( 0, 0, 0, 1 ); // black
        glClear( GL_COLOR_BUFFER_BIT );
        glPopAttrib();
        return;
        }
    if( pc.clip() && pc.paintArea().isEmpty())
        return; // no background to paint
    glPushAttrib( GL_CURRENT_BIT );
    glColor4f( 0, 0, 0, 1 ); // black
    for( PaintClipper::Iterator iterator;
         !iterator.isDone();
         iterator.next())
        {
        glBegin( GL_QUADS );
        QRect r = iterator.boundingRect();
        glVertex2i( r.x(), r.y());
        glVertex2i( r.x() + r.width(), r.y());
        glVertex2i( r.x() + r.width(), r.y() + r.height());
        glVertex2i( r.x(), r.y() + r.height());
        glEnd();
        }
    glPopAttrib();
    }

void SceneOpenGL::windowAdded( Toplevel* c )
    {
    assert( !windows.contains( c ));
    windows[ c ] = new Window( c );
    c->effectWindow()->setSceneWindow( windows[ c ]);
    }

void SceneOpenGL::windowClosed( Toplevel* c, Deleted* deleted )
    {
    assert( windows.contains( c ));
    if( deleted != NULL )
        { // replace c with deleted
        Window* w = windows.take( c );
        w->updateToplevel( deleted );
        windows[ deleted ] = w;
        }
    else
        {
        delete windows.take( c );
        c->effectWindow()->setSceneWindow( NULL );
        }
    }

void SceneOpenGL::windowDeleted( Deleted* c )
    {
    assert( windows.contains( c ));
    delete windows.take( c );
    c->effectWindow()->setSceneWindow( NULL );
    }

void SceneOpenGL::windowGeometryShapeChanged( Toplevel* c )
    {
    if( !windows.contains( c )) // this is ok, shape is not valid
        return;                 // by default
    Window* w = windows[ c ];
    w->discardShape();
    w->checkTextureSize();
    }

void SceneOpenGL::windowOpacityChanged( Toplevel* )
    {
#if 0 // not really needed, windows are painted on every repaint
      // and opacity is used when applying texture, not when
      // creating it
    if( !windows.contains( c )) // this is ok, texture is created
        return;                 // on demand
    Window* w = windows[ c ];
    w->discardTexture();
#endif
    }

//****************************************
// SceneOpenGL::Texture
//****************************************

SceneOpenGL::Texture::Texture() : GLTexture()
    {
    init();
    }

SceneOpenGL::Texture::Texture( const Pixmap& pix, const QSize& size, int depth ) : GLTexture()
    {
    init();
    load( pix, size, depth );
    }

SceneOpenGL::Texture::~Texture()
    {
    discard();
    }

void SceneOpenGL::Texture::init()
    {
    damaged = true;
    glxpixmap = None;
    }

void SceneOpenGL::Texture::createTexture()
    {
    glGenTextures( 1, &mTexture );
    }

void SceneOpenGL::Texture::discard()
    {
    if( mTexture != None )
        release();
    GLTexture::discard();
    }

void SceneOpenGL::Texture::release()
    {
    if( tfp_mode && glxpixmap != None )
        {
        glXReleaseTexImageEXT( display(), glxpixmap, GLX_FRONT_LEFT_EXT );
        glXDestroyPixmap( display(), glxpixmap );
        glxpixmap = None;
        }
    }

void SceneOpenGL::Texture::findTarget()
    {
    unsigned int new_target = 0;
    if( tfp_mode && glXQueryDrawable && glxpixmap != None )
        glXQueryDrawable( display(), glxpixmap, GLX_TEXTURE_TARGET_EXT, &new_target );
    // Hack for XGL - this should not be a fallback for glXQueryDrawable() but instead the case
    // when glXQueryDrawable is not available. However this call fails with XGL, unless KWin
    // is compiled statically with the libGL that Compiz is built against (without which neither
    // Compiz works with XGL). Falling back to doing this manually makes this work.
    if( new_target == 0 )
        {
        if( NPOTTextureSupported() ||
            ( isPowerOfTwo( mSize.width()) && isPowerOfTwo( mSize.height())))
            new_target = GLX_TEXTURE_2D_EXT;
        else
            new_target = GLX_TEXTURE_RECTANGLE_EXT;
        }
    switch( new_target )
        {
        case GLX_TEXTURE_2D_EXT:
            mTarget = GL_TEXTURE_2D;
            mScale.setWidth( 1.0f / mSize.width());
            mScale.setHeight( 1.0f / mSize.height());
          break;
        case GLX_TEXTURE_RECTANGLE_EXT:
            mTarget = GL_TEXTURE_RECTANGLE_ARB;
            mScale.setWidth( 1.0f );
            mScale.setHeight( 1.0f );
          break;
        default:
            abort();
        }
    }

QRegion SceneOpenGL::Texture::optimizeBindDamage( const QRegion& reg, int limit )
    {
    if( reg.rects().count() <= 1 )
        return reg;
    // try to reduce the number of rects, as especially with SHM mode every rect
    // causes X roundtrip, even for very small areas - so, when the size difference
    // between all the areas and the bounding rectangle is small, simply use
    // only the bounding rectangle
    int size = 0;
    foreach( const QRect &r, reg.rects())
        size += r.width() * r.height();
    if( reg.boundingRect().width() * reg.boundingRect().height() - size < limit )
        return reg.boundingRect();
    return reg;
    }

bool SceneOpenGL::Texture::load( const Pixmap& pix, const QSize& size,
    int depth, QRegion region )
    {
#ifdef CHECK_GL_ERROR
    checkGLError( "TextureLoad1" );
#endif
    if( pix == None || size.isEmpty() || depth < 1 )
        return false;
    if( tfp_mode )
        {
        if( fbcdrawableinfo[ depth ].fbconfig == NULL )
            {
            kDebug( 1212 ) << "No framebuffer configuration for depth " << depth
                           << "; not binding pixmap" << endl;
            return false;
            }
        }

    mSize = size;
    if( mTexture == None || !region.isEmpty())
        { // new texture, or texture contents changed; mipmaps now invalid
        setDirty();
        }

#ifdef CHECK_GL_ERROR
    checkGLError( "TextureLoad2" );
#endif
    if( tfp_mode )
        { // tfp mode, simply bind the pixmap to texture
        if( mTexture == None )
            createTexture();
        // The GLX pixmap references the contents of the original pixmap, so it doesn't
        // need to be recreated when the contents change.
        // The texture may or may not use the same storage depending on the EXT_tfp
        // implementation. When options->glStrictBinding is true, the texture uses
        // a different storage and needs to be updated with a call to
        // glXBindTexImageEXT() when the contents of the pixmap has changed.
        if( glxpixmap != None )
            glBindTexture( mTarget, mTexture );
        else
            {
            int attrs[] =
                {
                GLX_TEXTURE_FORMAT_EXT, fbcdrawableinfo[ depth ].bind_texture_format,
                GLX_MIPMAP_TEXTURE_EXT, fbcdrawableinfo[ depth ].mipmap,
                None, None, None
                };
            if ( ( fbcdrawableinfo[ depth ].texture_targets & GLX_TEXTURE_2D_BIT_EXT ) &&
                 ( GLTexture::NPOTTextureSupported() ||
                   ( isPowerOfTwo(size.width()) && isPowerOfTwo(size.height()) )))
                {
                attrs[ 4 ] = GLX_TEXTURE_TARGET_EXT;
                attrs[ 5 ] = GLX_TEXTURE_2D_EXT;
                }
            else if ( fbcdrawableinfo[ depth ].texture_targets & GLX_TEXTURE_RECTANGLE_BIT_EXT )
                {
                attrs[ 4 ] = GLX_TEXTURE_TARGET_EXT;
                attrs[ 5 ] = GLX_TEXTURE_RECTANGLE_EXT;
                }
            glxpixmap = glXCreatePixmap( display(), fbcdrawableinfo[ depth ].fbconfig, pix, attrs );
#ifdef CHECK_GL_ERROR
            checkGLError( "TextureLoadTFP1" );
#endif
            findTarget();
            y_inverted = fbcdrawableinfo[ depth ].y_inverted ? true : false;
            can_use_mipmaps = fbcdrawableinfo[ depth ].mipmap ? true : false;
            glBindTexture( mTarget, mTexture );
#ifdef CHECK_GL_ERROR
            checkGLError( "TextureLoadTFP2" );
#endif
            if( !options->glStrictBinding )
                glXBindTexImageEXT( display(), glxpixmap, GLX_FRONT_LEFT_EXT, NULL );
            }
        if( options->glStrictBinding )
            // Mark the texture as damaged so it will be updated on the next call to bind()
            damaged = true;
        }
    else if( shm_mode )
        { // copy pixmap contents to a texture via shared memory
#ifdef HAVE_XSHM
        GLenum pixfmt, type;
        if( depth >= 24 )
            {
            pixfmt = GL_BGRA;
            type = GL_UNSIGNED_BYTE;
            }
        else
            { // depth 16
            pixfmt = GL_RGB;
            type = GL_UNSIGNED_SHORT_5_6_5;
            }
        findTarget();
#ifdef CHECK_GL_ERROR
        checkGLError( "TextureLoadSHM1" );
#endif
        if( mTexture == None )
            {
            createTexture();
            glBindTexture( mTarget, mTexture );
            y_inverted = false;
            glTexImage2D( mTarget, 0, depth == 32 ? GL_RGBA : GL_RGB,
                mSize.width(), mSize.height(), 0,
                pixfmt, type, NULL );
            }
        else
            glBindTexture( mTarget, mTexture );
        if( !region.isEmpty())
            {
            XGCValues xgcv;
            xgcv.graphics_exposures = False;
            xgcv.subwindow_mode = IncludeInferiors;
            GC gc = XCreateGC( display(), pix, GCGraphicsExposures | GCSubwindowMode, &xgcv );
            Pixmap p = XShmCreatePixmap( display(), rootWindow(), shm.shmaddr, &shm,
                mSize.width(), mSize.height(), depth );
            QRegion damage = optimizeBindDamage( region, 100 * 100 );
            glPixelStorei( GL_UNPACK_ROW_LENGTH, mSize.width());
            foreach( const QRect &r, damage.rects())
                { // TODO for small areas it might be faster to not use SHM to avoid the XSync()
                XCopyArea( display(), pix, p, gc, r.x(), r.y(), r.width(), r.height(), 0, 0 );
                glXWaitX();
                glTexSubImage2D( mTarget, 0,
                    r.x(), r.y(), r.width(), r.height(),
                    pixfmt, type, shm.shmaddr );
                glXWaitGL();
                }
            glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
            XFreePixmap( display(), p );
            XFreeGC( display(), gc );
            }
#ifdef CHECK_GL_ERROR
        checkGLError( "TextureLoadSHM2" );
#endif
        y_inverted = true;
        can_use_mipmaps = true;
#endif
        }
    else
        { // fallback, copy pixmap contents to a texture
        // note that if depth is not QX11Info::appDepth(), this may
        // not work (however, it does seem to work with nvidia)
        findTarget();
        GLXDrawable pixmap = glXCreatePixmap( display(), fbcdrawableinfo[ QX11Info::appDepth() ].fbconfig, pix, NULL );
        glXMakeCurrent( display(), pixmap, ctxdrawable );
        if( last_pixmap != None )
            glXDestroyPixmap( display(), last_pixmap );
        // workaround for ATI - it leaks/crashes when the pixmap is destroyed immediately
        // here (http://lists.kde.org/?l=kwin&m=116353772208535&w=2)
        last_pixmap = pixmap;
        glReadBuffer( GL_FRONT );
        glDrawBuffer( GL_FRONT );
        if( mTexture == None )
            {
            createTexture();
            glBindTexture( mTarget, mTexture );
            y_inverted = false;
            glCopyTexImage2D( mTarget, 0,
                depth == 32 ? GL_RGBA : GL_RGB,
                0, 0, mSize.width(), mSize.height(), 0 );
            }
        else
            {
            glBindTexture( mTarget, mTexture );
            QRegion damage = optimizeBindDamage( region, 30 * 30 );
            foreach( const QRect &r, damage.rects())
                {
                // convert to OpenGL coordinates (this is mapping
                // the pixmap to a texture, this is not affected
                // by using glOrtho() for the OpenGL scene)
                int gly = mSize.height() - r.y() - r.height();
                glCopyTexSubImage2D( mTarget, 0,
                    r.x(), gly, r.x(), gly, r.width(), r.height());
                }
            }
        glXWaitGL();
        if( db )
            glDrawBuffer( GL_BACK );
        glXMakeCurrent( display(), glxbuffer, ctxbuffer );
        glBindTexture( mTarget, mTexture );
        y_inverted = false;
        can_use_mipmaps = true;
        }
#ifdef CHECK_GL_ERROR
    checkGLError( "TextureLoad0" );
#endif
    return true;
    }

bool SceneOpenGL::Texture::load( const Pixmap& pix, const QSize& size,
    int depth )
    {
    return load( pix, size, depth,
        QRegion( 0, 0, size.width(), size.height()));
    }

bool SceneOpenGL::Texture::load( const QImage& image, GLenum target )
    {
    if( image.isNull())
        return false;
    return load( QPixmap::fromImage( image ), target );
    }

bool SceneOpenGL::Texture::load( const QPixmap& pixmap, GLenum target )
    {
    Q_UNUSED( target ); // SceneOpenGL::Texture::findTarget() detects the target
    if( pixmap.isNull())
        return false;
    return load( pixmap.handle(), pixmap.size(), pixmap.depth());
    }

void SceneOpenGL::Texture::bind()
    {
    glEnable( mTarget );
    glBindTexture( mTarget, mTexture );
    if( tfp_mode )
        {
        if ( options->glStrictBinding && damaged )
            {
            // Update the texture with the new pixmap contents
            assert( glxpixmap != None );
            glXReleaseTexImageEXT( display(), glxpixmap, GLX_FRONT_LEFT_EXT );
            glXBindTexImageEXT( display(), glxpixmap, GLX_FRONT_LEFT_EXT, NULL );
            setDirty(); // Mipmaps have to be regenerated after updating the texture 
            }
        damaged = false;
        }
    enableFilter();
    if( hasGLVersion( 1, 4, 0 ))
        {
        // Lod bias makes the trilinear-filtered texture look a bit sharper
        glTexEnvf( GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, -1.0f );
        }
    }

void SceneOpenGL::Texture::unbind()
    {
    if( hasGLVersion( 1, 4, 0 ))
        {
        glTexEnvf( GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, 0.0f );
        }
    GLTexture::unbind();
    }

//****************************************
// SceneOpenGL::Window
//****************************************

SceneOpenGL::Window::Window( Toplevel* c )
    : Scene::Window( c )
    , texture()
    , topTexture()
    , leftTexture()
    , rightTexture()
    , bottomTexture()
    {
    }

SceneOpenGL::Window::~Window()
    {
    discardTexture();
    }

// Bind the window pixmap to an OpenGL texture.
bool SceneOpenGL::Window::bindTexture()
    {
    if( texture.texture() != None && toplevel->damage().isEmpty())
        {
        // texture doesn't need updating, just bind it
        glBindTexture( texture.target(), texture.texture());
        return true;
        }
    // Get the pixmap with the window contents
    Pixmap pix = toplevel->windowPixmap();
    if( pix == None )
        return false;
    bool success = texture.load( pix, toplevel->size(), toplevel->depth(),
        toplevel->damage());
    if( success )
        toplevel->resetDamage( QRect( toplevel->clientPos(), toplevel->clientSize() ) );
    else
        kDebug( 1212 ) << "Failed to bind window";
    return success;
    }

void SceneOpenGL::Window::discardTexture()
    {
    texture.discard();
    topTexture.discard();
    leftTexture.discard();
    rightTexture.discard();
    bottomTexture.discard();
    }

// This call is used in SceneOpenGL::windowGeometryShapeChanged(),
// which originally called discardTexture(), however this was causing performance
// problems with the launch feedback icon - large number of texture rebinds.
// Since the launch feedback icon does not resize, only changes shape, it
// is not necessary to rebind the texture (with no strict binding), therefore
// discard the texture only if size changes.
void SceneOpenGL::Window::checkTextureSize()
    {
    if( texture.size() != size())
        discardTexture();
    }

// when the window's composite pixmap is discarded, undo binding it to the texture
void SceneOpenGL::Window::pixmapDiscarded()
    {
    texture.release();
    }

// paint the window
void SceneOpenGL::Window::performPaint( int mask, QRegion region, WindowPaintData data )
    {
    // check if there is something to paint (e.g. don't paint if the window
    // is only opaque and only PAINT_WINDOW_TRANSLUCENT is requested)
    /* HACK: It seems this causes painting glitches, disable temporarily
    bool opaque = isOpaque() && data.opacity == 1.0;
    if(( mask & PAINT_WINDOW_OPAQUE ) ^ ( mask & PAINT_WINDOW_TRANSLUCENT ))
        { // We are only painting either opaque OR translucent windows, not both
        if( mask & PAINT_WINDOW_OPAQUE && !opaque )
            return; // Only painting opaque and window is translucent
        if( mask & PAINT_WINDOW_TRANSLUCENT && opaque )
            return; // Only painting translucent and window is opaque
        }*/
    // paint only requested areas
    if( region != infiniteRegion()) // avoid integer overflow
        region.translate( -x(), -y());
    if( region.isEmpty())
        return;
    if( !bindTexture())
        return;
    glPushMatrix();
    // set texture filter
    if( options->glSmoothScale != 0 ) // default to yes
        {
        if( mask & PAINT_WINDOW_TRANSFORMED )
            filter = ImageFilterGood;
        else if( mask & PAINT_SCREEN_TRANSFORMED )
            filter = ImageFilterGood;
        else
            filter = ImageFilterFast;
        }
    else
        filter = ImageFilterFast;
    if( filter == ImageFilterGood )
        {
        // avoid unneeded mipmap generation by only using trilinear
        // filtering when it actually makes a difference, that is with
        // minification or changed vertices
        if( options->glSmoothScale == 2
            && ( data.quads.smoothNeeded() || data.xScale < 1 || data.yScale < 1 ))
            {
            texture.setFilter( GL_LINEAR_MIPMAP_LINEAR );
            }
        else
            texture.setFilter( GL_LINEAR );
        }
    else
        texture.setFilter( GL_NEAREST );
    // do required transformations
    int x = toplevel->x();
    int y = toplevel->y();
    double z = 0.0;
    if( mask & PAINT_WINDOW_TRANSFORMED )
        {
        x += data.xTranslate;
        y += data.yTranslate;
        z += data.zTranslate;
        }
    glTranslatef( x, y, z );
    if(( mask & PAINT_WINDOW_TRANSFORMED ) && ( data.xScale != 1 || data.yScale != 1 || data.zScale != 1 ))
        glScalef( data.xScale, data.yScale, data.zScale );
    if(( mask & PAINT_WINDOW_TRANSFORMED ) && data.rotation )
        {
        glTranslatef( data.rotation->xRotationPoint, data.rotation->yRotationPoint, data.rotation->zRotationPoint );
        float xAxis = 0.0;
        float yAxis = 0.0;
        float zAxis = 0.0;
        switch( data.rotation->axis )
            {
            case RotationData::XAxis:
                xAxis = 1.0;
                break;
            case RotationData::YAxis:
                yAxis = 1.0;
                break;
            case RotationData::ZAxis:
                zAxis = 1.0;
                break;
            }
        glRotatef( data.rotation->angle, xAxis, yAxis, zAxis );
        glTranslatef( -data.rotation->xRotationPoint, -data.rotation->yRotationPoint, -data.rotation->zRotationPoint );
        }
    region.translate( toplevel->x(), toplevel->y() );  // Back to screen coords

    WindowQuadList decoration = data.quads.select( WindowQuadDecoration );


    // decorations
    Client *client = dynamic_cast<Client*>(toplevel);
    Deleted *deleted = dynamic_cast<Deleted*>(toplevel);
    if( client || deleted )
        {
        bool noBorder = true;
        bool updateDeco = false;
        const QPixmap *left = NULL;
        const QPixmap *top = NULL;
        const QPixmap *right = NULL;
        const QPixmap *bottom = NULL;
        QRect topRect, leftRect, rightRect, bottomRect;
        if( client && !client->noBorder() )
            {
            noBorder = false;
            updateDeco = client->decorationPixmapRequiresRepaint();
            client->ensureDecorationPixmapsPainted();

            client->layoutDecorationRects(leftRect, topRect, rightRect, bottomRect, Client::WindowRelative);

            left   = client->leftDecoPixmap();
            top    = client->topDecoPixmap();
            right  = client->rightDecoPixmap();
            bottom = client->bottomDecoPixmap();
            }
        if( deleted && !deleted->noBorder() )
            {
            noBorder = false;
            left   = deleted->leftDecoPixmap();
            top    = deleted->topDecoPixmap();
            right  = deleted->rightDecoPixmap();
            bottom = deleted->bottomDecoPixmap();
            deleted->layoutDecorationRects(leftRect, topRect, rightRect, bottomRect);
            }
        if( !noBorder )
            {
            WindowQuadList topList, leftList, rightList, bottomList;

            foreach( const WindowQuad& quad, decoration )
                {
                if( topRect.contains( QPoint( quad.originalLeft(), quad.originalTop() ) ) )
                    {
                    topList.append( quad );
                    continue;
                    }
                if( bottomRect.contains( QPoint( quad.originalLeft(), quad.originalTop() ) ) )
                    {
                    bottomList.append( quad );
                    continue;
                    }
                if( leftRect.contains( QPoint( quad.originalLeft(), quad.originalTop() ) ) )
                    {
                    leftList.append( quad );
                    continue;
                    }
                if( rightRect.contains( QPoint( quad.originalLeft(), quad.originalTop() ) ) )
                    {
                    rightList.append( quad );
                    continue;
                    }
                }
            paintDecoration( top, DecorationTop, region, topRect, data, topList, updateDeco );
            paintDecoration( left, DecorationLeft, region, leftRect, data, leftList, updateDeco );
            paintDecoration( right, DecorationRight, region, rightRect, data, rightList, updateDeco );
            paintDecoration( bottom, DecorationBottom, region, bottomRect, data, bottomList, updateDeco );
            }
        }

    texture.bind();
    texture.enableUnnormalizedTexCoords();

    // paint the content
    if ( !(mask & PAINT_DECORATION_ONLY) )
        {
        prepareStates( Content, data.opacity * data.contents_opacity, data.brightness, data.saturation, data.shader );
        renderQuads( mask, region, data.quads.select( WindowQuadContents ));
        restoreStates( Content, data.opacity * data.contents_opacity, data.brightness, data.saturation, data.shader );
        }

    texture.disableUnnormalizedTexCoords();
    texture.unbind();

    glPopMatrix();
    }

void SceneOpenGL::Window::paintDecoration( const QPixmap* decoration, TextureType decorationType, const QRegion& region, const QRect& rect, const WindowPaintData& data, const WindowQuadList& quads, bool updateDeco )
    {
    if( quads.isEmpty())
        return;
    SceneOpenGL::Texture* decorationTexture;
    switch( decorationType )
        {
        case DecorationTop:
            decorationTexture = &topTexture;
            break;
        case DecorationLeft:
            decorationTexture = &leftTexture;
            break;
        case DecorationRight:
            decorationTexture = &rightTexture;
            break;
        case DecorationBottom:
            decorationTexture = &bottomTexture;
            break;
        default:
            return;
        }
    if( decorationTexture->texture() != None && !updateDeco )
        {
        // texture doesn't need updating, just bind it
        glBindTexture( decorationTexture->target(), decorationTexture->texture());
        }
    else if( !decoration->isNull() )
        {
        bool success = decorationTexture->load( decoration->handle(), decoration->size(), decoration->depth() );
        if( !success )
            {
            kDebug( 1212 ) << "Failed to bind decoartion";
            return;
            }
        }
    else
        return;
    if( filter == ImageFilterGood )
        {
        // avoid unneeded mipmap generation by only using trilinear
        // filtering when it actually makes a difference, that is with
        // minification or changed vertices
        if( options->glSmoothScale == 2
            && ( data.quads.smoothNeeded() || data.xScale < 1 || data.yScale < 1 ))
            {
            decorationTexture->setFilter( GL_LINEAR_MIPMAP_LINEAR );
            }
        else
            decorationTexture->setFilter( GL_LINEAR );
        }
    else
        decorationTexture->setFilter( GL_NEAREST );
    decorationTexture->setWrapMode( GL_CLAMP_TO_EDGE );
    decorationTexture->bind();
    decorationTexture->enableUnnormalizedTexCoords();

    prepareStates( decorationType, data.opacity * data.decoration_opacity, data.brightness, data.saturation, data.shader );
    float* vertices;
    float* texcoords;
    makeDecorationArrays( &vertices, &texcoords, quads, rect );
    if( data.shader )
        {
        int texw = decoration->width();
        int texh = decoration->height();
        if( !GLTexture::NPOTTextureSupported() )
            {
            kWarning( 1212 ) << "NPOT textures not supported, wasting some memory" ;
            texw = nearestPowerOfTwo(texw);
            texh = nearestPowerOfTwo(texh);
            }
        data.shader->setUniform("textureWidth", (float)texw);
        data.shader->setUniform("textureHeight", (float)texh);
        }
    renderGLGeometry( region, quads.count() * 4,
            vertices, texcoords, NULL, 2, 0 );
    delete[] vertices;
    delete[] texcoords;
    restoreStates( decorationType, data.opacity * data.decoration_opacity, data.brightness, data.saturation, data.shader );
    decorationTexture->disableUnnormalizedTexCoords();
    decorationTexture->unbind();
    }

void SceneOpenGL::Window::makeDecorationArrays( float** vertices, float** texcoords, const WindowQuadList& quads, const QRect& rect ) const
    {
    *vertices = new float[ quads.count() * 4 * 2 ];
    *texcoords = new float[ quads.count() * 4 * 2 ];
    float* vpos = *vertices;
    float* tpos = *texcoords;
    foreach( const WindowQuad& quad, quads )
        {
        *vpos++ = quad[ 0 ].x();
        *vpos++ = quad[ 0 ].y();
        *vpos++ = quad[ 1 ].x();
        *vpos++ = quad[ 1 ].y();
        *vpos++ = quad[ 2 ].x();
        *vpos++ = quad[ 2 ].y();
        *vpos++ = quad[ 3 ].x();
        *vpos++ = quad[ 3 ].y();

        *tpos++ = quad.originalLeft()-rect.x();
        *tpos++ = quad.originalTop()-rect.y();
        *tpos++ = quad.originalRight()-rect.x();
        *tpos++ = quad.originalTop()-rect.y();
        *tpos++ = quad.originalRight()-rect.x();
        *tpos++ = quad.originalBottom()-rect.y();
        *tpos++ = quad.originalLeft()-rect.x();
        *tpos++ = quad.originalBottom()-rect.y();
        }
    }

void SceneOpenGL::Window::renderQuads( int, const QRegion& region, const WindowQuadList& quads )
    {
    if( quads.isEmpty())
        return;
    // Render geometry
    float* vertices;
    float* texcoords;
    quads.makeArrays( &vertices, &texcoords );
    renderGLGeometry( region, quads.count() * 4,
            vertices, texcoords, NULL, 2, 0 );
    delete[] vertices;
    delete[] texcoords;
    }

void SceneOpenGL::Window::prepareStates( TextureType type, double opacity, double brightness, double saturation, GLShader* shader )
    {
    if(shader)
        prepareShaderRenderStates( type, opacity, brightness, saturation, shader );
    else
        prepareRenderStates( type, opacity, brightness, saturation );
    }

void SceneOpenGL::Window::prepareShaderRenderStates( TextureType type, double opacity, double brightness, double saturation, GLShader* shader )
    {
    // setup blending of transparent windows
    glPushAttrib( GL_ENABLE_BIT );
    bool opaque = isOpaque() && opacity == 1.0;
    if( type != Content )
        opaque = false;
    if( !opaque )
        {
        glEnable( GL_BLEND );
        glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
        }
    shader->setUniform("opacity", (float)opacity);
    shader->setUniform("saturation", (float)saturation);
    shader->setUniform("brightness", (float)brightness);

    // setting texture width and heiht stored in shader
    // only set if it is set by an effect that is not negative
    float texw = shader->textureWidth();
    if( texw >= 0.0f )
        shader->setUniform("textureWidth", texw);
    float texh = shader->textureHeight();
    if( texh >= 0.0f )
        shader->setUniform("textureHeight", texh);
    }

void SceneOpenGL::Window::prepareRenderStates( TextureType type, double opacity, double brightness, double saturation )
    {
    Texture* tex;
    bool alpha = false;
    bool opaque = true;
    switch( type )
        {
        case Content:
            tex = &texture;
            alpha = toplevel->hasAlpha();
            opaque = isOpaque() && opacity == 1.0;
            break;
        case DecorationTop:
            tex = &topTexture;
            alpha = true;
            opaque = false;
            break;
        case DecorationLeft:
            tex = &leftTexture;
            alpha = true;
            opaque = false;
            break;
        case DecorationRight:
            tex = &rightTexture;
            alpha = true;
            opaque = false;
            break;
        case DecorationBottom:
            tex = &bottomTexture;
            alpha = true;
            opaque = false;
            break;
        default:
            return;
        }
    // setup blending of transparent windows
    glPushAttrib( GL_ENABLE_BIT );
    if( !opaque )
        {
        glEnable( GL_BLEND );
        glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
        }
    if( saturation != 1.0 && tex->saturationSupported())
        {
        // First we need to get the color from [0; 1] range to [0.5; 1] range
        glActiveTexture( GL_TEXTURE0 );
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
        glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA );
        const float scale_constant[] = { 1.0, 1.0, 1.0, 0.5};
        glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, scale_constant );
        tex->bind();

        // Then we take dot product of the result of previous pass and
        //  saturation_constant. This gives us completely unsaturated
        //  (greyscale) image
        // Note that both operands have to be in range [0.5; 1] since opengl
        //  automatically substracts 0.5 from them
        glActiveTexture( GL_TEXTURE1 );
        float saturation_constant[] = { 0.5 + 0.5*0.30, 0.5 + 0.5*0.59, 0.5 + 0.5*0.11, saturation };
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
        glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR );
        glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, saturation_constant );
        tex->bind();

        // Finally we need to interpolate between the original image and the
        //  greyscale image to get wanted level of saturation
        glActiveTexture( GL_TEXTURE2 );
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
        glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0 );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA );
        glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, saturation_constant );
        // Also replace alpha by primary color's alpha here
        glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
        // And make primary color contain the wanted opacity
        glColor4f( opacity, opacity, opacity, opacity );
        tex->bind();

        if( alpha || brightness != 1.0f )
            {
            glActiveTexture( GL_TEXTURE3 );
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
            glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE );
            glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS );
            glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR );
            glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR );
            glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR );
            // The color has to be multiplied by both opacity and brightness
            float opacityByBrightness = opacity * brightness;
            glColor4f( opacityByBrightness, opacityByBrightness, opacityByBrightness, opacity );
            if( alpha )
                {
                // Multiply original texture's alpha by our opacity
                glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE );
                glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0 );
                glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
                glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR );
                glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA );
                }
            else
                {
                // Alpha will be taken from previous stage
                glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE );
                glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS );
                glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
                }
            tex->bind();
            }

        glActiveTexture(GL_TEXTURE0 );
        }
    else if( opacity != 1.0 || brightness != 1.0 )
        {
        // the window is additionally configured to have its opacity adjusted,
        // do it
        float opacityByBrightness = opacity * brightness;
        if( alpha)
            {
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
            glColor4f( opacityByBrightness, opacityByBrightness, opacityByBrightness,
                opacity);
            }
        else
            {
            // Multiply color by brightness and replace alpha by opacity
            float constant[] = { opacityByBrightness, opacityByBrightness, opacityByBrightness, opacity };
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
            glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE );
            glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE );
            glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR );
            glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT );
            glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR );
            glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE );
            glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_CONSTANT );
            glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constant );
            }
        }
    else if( !alpha && opaque )
        {
        float constant[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
        glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE );
        glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE );
        glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_CONSTANT );
        glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constant );
        }
    }

void SceneOpenGL::Window::restoreStates( TextureType type, double opacity, double brightness, double saturation, GLShader* shader )
    {
    if(shader)
        restoreShaderRenderStates( type, opacity, brightness, saturation, shader );
    else
        restoreRenderStates( type, opacity, brightness, saturation );
    }

void SceneOpenGL::Window::restoreShaderRenderStates( TextureType type, double opacity, double brightness, double saturation, GLShader* shader )
    {
    Q_UNUSED( type );
    Q_UNUSED( opacity );
    Q_UNUSED( brightness );
    Q_UNUSED( saturation );
    Q_UNUSED( shader );
    glPopAttrib();  // ENABLE_BIT
    }

void SceneOpenGL::Window::restoreRenderStates( TextureType type, double opacity, double brightness, double saturation )
    {
    Texture* tex;
    switch( type )
        {
        case Content:
            tex = &texture;
            break;
        case DecorationTop:
            tex = &topTexture;
            break;
        case DecorationLeft:
            tex = &leftTexture;
            break;
        case DecorationRight:
            tex = &rightTexture;
            break;
        case DecorationBottom:
            tex = &bottomTexture;
            break;
        default:
            return;
        }
    if( opacity != 1.0 || saturation != 1.0 || brightness != 1.0f )
        {
        if( saturation != 1.0 && tex->saturationSupported())
            {
            glActiveTexture(GL_TEXTURE3);
            glDisable( tex->target());
            glActiveTexture(GL_TEXTURE2);
            glDisable( tex->target());
            glActiveTexture(GL_TEXTURE1);
            glDisable( tex->target());
            glActiveTexture(GL_TEXTURE0);
            }
        }
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    glColor4f( 0, 0, 0, 0 );

    glPopAttrib();  // ENABLE_BIT
    }

} // namespace

#endif
