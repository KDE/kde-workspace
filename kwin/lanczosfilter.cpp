/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2010 by Fredrik Höglund <fredrik@kde.org>

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

#include "lanczosfilter.h"
#include "effects.h"

#ifdef KWIN_HAVE_OPENGL_COMPOSITING
    #include <kwinglutils.h>
#endif

#include <kwineffects.h>
#include <KDE/KGlobalSettings>

#include <qmath.h>
#include <cmath>

namespace KWin
{

LanczosFilter::LanczosFilter( QObject* parent )
    : QObject( parent )
#ifdef KWIN_HAVE_OPENGL_COMPOSITING
    , m_offscreenTex( 0 )
    , m_offscreenTarget( 0 )
    , m_shader( 0 )
#endif
    , m_inited( false)
    {
    }

LanczosFilter::~LanczosFilter()
    {
#ifdef KWIN_HAVE_OPENGL_COMPOSITING
    delete m_offscreenTarget;
    delete m_offscreenTex;
    delete m_shader;
#endif
    }

void LanczosFilter::init()
    {
    if (m_inited)
        return;
    m_inited = true;
#ifdef KWIN_HAVE_OPENGL_COMPOSITING
    // check the blacklist
    KSharedConfigPtr config = KSharedConfig::openConfig( "kwinrc" );
    KConfigGroup blacklist = config->group( "Blacklist" ).group( "Lanczos" );
    if( effects->checkDriverBlacklist( blacklist ) )
        {
        kDebug() << "Lanczos Filter disabled by driver blacklist";
        return;
        }

    if ( GLShader::fragmentShaderSupported() &&
         GLShader::vertexShaderSupported() &&
         GLRenderTarget::supported() )
        {
        m_shader = new GLShader(":/resources/lanczos-vertex.glsl", ":/resources/lanczos-fragment.glsl");
        if (m_shader->isValid())
            {
            m_shader->bind();
            m_uTexUnit    = m_shader->uniformLocation("texUnit");
            m_uKernel     = m_shader->uniformLocation("kernel");
            m_uOffsets    = m_shader->uniformLocation("offsets");
            m_shader->unbind();
            }
        else
            {
            kDebug(1212) << "Shader is not valid";
            m_shader = 0;
            }
        }
#endif
    }


void LanczosFilter::updateOffscreenSurfaces()
    {
#ifdef KWIN_HAVE_OPENGL_COMPOSITING
    int w = displayWidth();
    int h = displayHeight();
    if ( !GLTexture::NPOTTextureSupported() )
        {
        w = nearestPowerOfTwo( w );
        h = nearestPowerOfTwo( h );
        }
    if ( !m_offscreenTex || m_offscreenTex->width() != w || m_offscreenTex->height() != h )
        {
        if ( m_offscreenTex )
            {
            delete m_offscreenTex;
            delete m_offscreenTarget;
            }
        m_offscreenTex = new GLTexture( w, h );
        m_offscreenTex->setFilter( GL_LINEAR );
        m_offscreenTex->setWrapMode( GL_CLAMP_TO_EDGE );
        m_offscreenTarget = new GLRenderTarget( m_offscreenTex );
        }
#endif
    }

static float sinc( float x )
    {
    return std::sin( x * M_PI ) / ( x * M_PI );
    }

static float lanczos( float x, float a )
    {
    if ( qFuzzyCompare( x + 1.0, 1.0 ) )
        return 1.0;

    if ( qAbs( x ) >= a )
        return 0.0;

    return sinc( x ) * sinc( x / a );
    }

void LanczosFilter::createKernel( float delta, int *size )
    {
    const float a = 2.0;

    // The two outermost samples always fall at points where the lanczos
    // function returns 0, so we'll skip them.
    const int sampleCount = qBound( 3, qCeil(delta * a) * 2 + 1 - 2, 49 );
    const int center = sampleCount / 2;
    const int kernelSize = center + 1;
    const float factor = 1.0 / delta;

    QVector<float> values( kernelSize );
    float sum = 0;

    for ( int i = 0; i < kernelSize; i++ ) {
        const float val = lanczos( i * factor, a );
        sum += i > 0 ? val * 2 : val;
        values[i] = val;
    }

    memset(m_kernel, 0, 25 * sizeof(QVector4D));

    // Normalize the kernel
    for ( int i = 0; i < kernelSize; i++ ) {
        const float val = values[i] / sum;
        m_kernel[i] = QVector4D( val, val, val, val );
    }

    *size = kernelSize;
    }

void LanczosFilter::createOffsets( int count, float width, Qt::Orientation direction )
    {
    memset(m_offsets, 0, 25 * sizeof(QVector2D));
    for ( int i = 0; i < count; i++ ) {
        m_offsets[i] = ( direction == Qt::Horizontal ) ?
                QVector2D( i / width, 0 ) : QVector2D( 0, i / width );
    }
    }

void LanczosFilter::performPaint( EffectWindowImpl* w, int mask, QRegion region, WindowPaintData& data )
    {
#ifdef KWIN_HAVE_OPENGL_COMPOSITING
    if( effects->compositingType() == KWin::OpenGLCompositing &&
        KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects )
        {
        if (!m_inited)
            init();
        const QRect screenRect = Workspace::self()->clientArea( ScreenArea, w->screen(), w->desktop() );
        // window geometry may not be bigger than screen geometry to fit into the FBO
        if ( m_shader && w->width() <= screenRect.width() && w->height() <= screenRect.height() )
            {
            double left = 0;
            double top = 0;
            double right = w->width();
            double bottom = w->height();
            foreach( const WindowQuad& quad, data.quads )
                {
                // we need this loop to include the decoration padding
                left   = qMin(left, quad.left());
                top    = qMin(top, quad.top());
                right  = qMax(right, quad.right());
                bottom = qMax(bottom, quad.bottom());
                }
            double width = right - left;
            double height = bottom - top;
            if( width > screenRect.width() || height > screenRect.height() )
                {
                // window with padding does not fit into the framebuffer
                // so cut of the shadow
                left = 0;
                top = 0;
                width = w->width();
                height = w->height();
                }
            int tx = data.xTranslate + w->x() + left*data.xScale;
            int ty = data.yTranslate + w->y() + top*data.yScale;
            int tw = width*data.xScale;
            int th = height*data.yScale;
            const QRect textureRect(tx, ty, tw, th);

            int sw = width;
            int sh = height;
            GLTexture *cachedTexture = static_cast< GLTexture*>(w->data( LanczosCacheRole ).value<void*>());
            if( cachedTexture )
                {
                if( cachedTexture->width() == tw && cachedTexture->height() == th )
                    {
                    cachedTexture->bind();
                    prepareRenderStates( cachedTexture, data.opacity, data.brightness, data.saturation );
                    cachedTexture->render( textureRect, textureRect );
                    restoreRenderStates( cachedTexture, data.opacity, data.brightness, data.saturation );
                    cachedTexture->unbind();
                    m_timer.start( 5000, this );
                    return;
                    }
                else
                    {
                    // offscreen texture not matching - delete
                    delete cachedTexture;
                    cachedTexture = 0;
                    w->setData( LanczosCacheRole, QVariant() );
                    }
                }

            WindowPaintData thumbData = data;
            thumbData.xScale = 1.0;
            thumbData.yScale = 1.0;
            thumbData.xTranslate = -w->x() - left;
            thumbData.yTranslate = -w->y() - top;
            thumbData.brightness = 1.0;
            thumbData.opacity = 1.0;
            thumbData.saturation = 1.0;

            // Bind the offscreen FBO and draw the window on it unscaled
            updateOffscreenSurfaces();
            effects->pushRenderTarget( m_offscreenTarget );

            glClear( GL_COLOR_BUFFER_BIT );
            w->sceneWindow()->performPaint( mask, QRegion(0, 0, sw, sh), thumbData );

            // Create a scratch texture and copy the rendered window into it
            GLTexture tex( sw, sh );
            tex.setFilter( GL_LINEAR );
            tex.setWrapMode( GL_CLAMP_TO_EDGE );
            tex.bind();

            glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, m_offscreenTex->height() - sh, sw, sh );

            // Set up the shader for horizontal scaling
            float dx = sw / float(tw);
            int kernelSize;
            createKernel( dx, &kernelSize );
            createOffsets( kernelSize, sw, Qt::Horizontal );

            m_shader->bind();
            glUniform1i( m_uTexUnit, 0 );
            glUniform2fv( m_uOffsets, 25, (const GLfloat*)m_offsets );
            glUniform4fv( m_uKernel, 25, (const GLfloat*)m_kernel );

            // Draw the window back into the FBO, this time scaled horizontally
            glClear( GL_COLOR_BUFFER_BIT );

            glBegin( GL_QUADS );
            glTexCoord2f( 0, 0 ); glVertex2i( 0,   0 ); // Top left
            glTexCoord2f( 1, 0 ); glVertex2i( tw,  0 ); // Top right
            glTexCoord2f( 1, 1 ); glVertex2i( tw, sh ); // Bottom right
            glTexCoord2f( 0, 1 ); glVertex2i( 0,  sh ); // Bottom left
            glEnd();

            // At this point we don't need the scratch texture anymore
            tex.unbind();
            tex.discard();

            // create scratch texture for second rendering pass
            GLTexture tex2( tw, sh );
            tex2.setFilter( GL_LINEAR );
            tex2.setWrapMode( GL_CLAMP_TO_EDGE );
            tex2.bind();

            glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, m_offscreenTex->height() - sh, tw, sh );

            // Set up the shader for vertical scaling
            float dy = sh / float(th);
            createKernel( dy, &kernelSize );
            createOffsets( kernelSize, m_offscreenTex->height(), Qt::Vertical );

            glUniform2fv( m_uOffsets, 25, (const GLfloat*)m_offsets );
            glUniform4fv( m_uKernel, 25, (const GLfloat*)m_kernel );

            // Now draw the horizontally scaled window in the FBO at the right
            // coordinates on the screen, while scaling it vertically and blending it.
            glClear( GL_COLOR_BUFFER_BIT );

            glBegin( GL_QUADS );
            glTexCoord2f( 0, 0 ); glVertex2i(  0,  0 );      // Top left
            glTexCoord2f( 1, 0 ); glVertex2i( tw,  0 );      // Top right
            glTexCoord2f( 1, 1 ); glVertex2i( tw, th ); // Bottom right
            glTexCoord2f( 0, 1 ); glVertex2i(  0, th ); // Bottom left
            glEnd();

            tex2.unbind();
            tex2.discard();
            m_shader->unbind();

            // create cache texture
            GLTexture *cache = new GLTexture( tw, th );

            cache->setFilter( GL_LINEAR );
            cache->setWrapMode( GL_CLAMP_TO_EDGE );
            cache->bind();
            glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, m_offscreenTex->height() - th, tw, th );
            effects->popRenderTarget();
            prepareRenderStates( cache, data.opacity, data.brightness, data.saturation );
            cache->render( textureRect, textureRect );
            restoreRenderStates( cache, data.opacity, data.brightness, data.saturation );
            cache->unbind();
            w->setData( LanczosCacheRole, QVariant::fromValue( static_cast<void*>( cache )));

            // Delete the offscreen surface after 5 seconds
            m_timer.start( 5000, this );
            return;
            }
        } // if ( effects->compositingType() == KWin::OpenGLCompositing )
#endif
    w->sceneWindow()->performPaint( mask, region, data );
    } // End of function

void LanczosFilter::timerEvent( QTimerEvent *event )
    {
#ifdef KWIN_HAVE_OPENGL_COMPOSITING
    if (event->timerId() == m_timer.timerId())
        {
        m_timer.stop();

        delete m_offscreenTarget;
        delete m_offscreenTex;
        m_offscreenTarget = 0;
        m_offscreenTex = 0;
        foreach( EffectWindow* w, effects->stackingOrder() )
            {
            QVariant cachedTextureVariant = w->data( LanczosCacheRole );
            if( cachedTextureVariant.isValid() )
                {
                GLTexture *cachedTexture = static_cast< GLTexture*>(cachedTextureVariant.value<void*>());
                delete cachedTexture;
                cachedTexture = 0;
                w->setData( LanczosCacheRole, QVariant() );
                }
            }
        }
#endif
    }

void LanczosFilter::prepareRenderStates( GLTexture* tex, double opacity, double brightness, double saturation )
    {
#ifdef KWIN_HAVE_OPENGL_COMPOSITING
    const bool alpha = true;
    // setup blending of transparent windows
    glPushAttrib( GL_ENABLE_BIT );
    glEnable( GL_BLEND );
    glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
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
#endif
    }

void LanczosFilter::restoreRenderStates( GLTexture* tex, double opacity, double brightness, double saturation )
    {
#ifdef KWIN_HAVE_OPENGL_COMPOSITING
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
#endif
    }

} // namespace

