/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2012 Casian Andrei <skeletk13@gmail.com>

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

#include "kwinglcolorcorrection.h"
#include "kwinglcolorcorrection_p.h"

#include "kwinglplatform.h"
#include "kwinglutils.h"

#include <KDebug>

#include <QByteArrayMatcher>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QPair>
#include <QVector3D>

namespace KWin {

/*
 * Color lookup table
 *
 * The 3D lookup texture has 64 points in each dimension, using 16 bit integers.
 * That means each active region will use 1.5MiB of texture memory.
 */
static const int LUT_GRID_POINTS = 64;
static const size_t CLUT_ELEMENT_SIZE = sizeof(quint16);
static const uint CLUT_ELEMENT_COUNT = LUT_GRID_POINTS * LUT_GRID_POINTS * LUT_GRID_POINTS * 3;
static const size_t CLUT_DATA_SIZE = CLUT_ELEMENT_COUNT * CLUT_ELEMENT_SIZE;

inline static void buildDummyClut(Clut &c)
{
    c.resize(CLUT_ELEMENT_COUNT);
    quint16 *p = c.data();

    for (int ib = 0; ib < LUT_GRID_POINTS; ++ ib) {
        quint16 b = (quint16) ((float) ib / (LUT_GRID_POINTS - 1) * 65535.0 + 0.5);
        for (int ig = 0; ig < LUT_GRID_POINTS; ++ ig) {
            quint16 g = (quint16) ((float) ig / (LUT_GRID_POINTS - 1) * 65535.0 + 0.5);
            for (int ir = 0; ir < LUT_GRID_POINTS; ++ ir) {
                quint16 r = (quint16) ((float) ir / (LUT_GRID_POINTS - 1) * 65535.0 + 0.5);

                *(p ++) = r;
                *(p ++) = g;
                *(p ++) = b;
            }
        }
    }
}


/*
 * Color Server Interface
 */

ColorServerInterface::ColorServerInterface(const QString &service,
                                           const QString &path,
                                           const QDBusConnection &connection,
                                           QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
    , m_versionInfoWatcher(0)
    , m_outputClutsWatcher(0)
    , m_regionClutsWatcher(0)
    , m_versionInfoUpdated(false)
    , m_outputClutsUpdated(false)
    , m_regionClutsUpdated(false)
    , m_versionInfo(0)
    , m_signaledFail(false)
{
    qDBusRegisterMetaType< Clut >();
    qDBusRegisterMetaType< ClutList >();
    qDBusRegisterMetaType< RegionalClut >();
    qDBusRegisterMetaType< RegionalClutMap >();

    connect(this, SIGNAL(outputClutsChanged()), this, SLOT(update()));
    connect(this, SIGNAL(regionClutsChanged()), this, SLOT(update()));
}

ColorServerInterface::~ColorServerInterface()
{
}

uint ColorServerInterface::versionInfo() const
{
    if (!m_versionInfoUpdated)
        kWarning(1212) << "Version info not updated";
    return m_versionInfo;
}

const ClutList& ColorServerInterface::outputCluts() const
{
    return m_outputCluts;
}

const RegionalClutMap& ColorServerInterface::regionCluts() const
{
    return m_regionCluts;
}

void ColorServerInterface::update()
{
    m_versionInfoUpdated = false;
    m_outputClutsUpdated = false;
    m_regionClutsUpdated = false;
    delete m_versionInfoWatcher;
    delete m_outputClutsWatcher;
    delete m_regionClutsWatcher;
    m_versionInfoWatcher = new QDBusPendingCallWatcher(getVersionInfo(), this);
    m_outputClutsWatcher = new QDBusPendingCallWatcher(getOutputCluts(), this);
    m_regionClutsWatcher = new QDBusPendingCallWatcher(getRegionCluts(), this);
    connect(m_versionInfoWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(callFinishedSlot(QDBusPendingCallWatcher*)));
    connect(m_outputClutsWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(callFinishedSlot(QDBusPendingCallWatcher*)));
    connect(m_regionClutsWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(callFinishedSlot(QDBusPendingCallWatcher*)));

    m_signaledFail = false;
}

QDBusPendingReply< uint > ColorServerInterface::getVersionInfo()
{
    return QDBusPendingReply< uint >(asyncCall("getVersionInfo"));
}

QDBusPendingReply< ClutList > ColorServerInterface::getOutputCluts()
{
    return QDBusPendingReply< ClutList >(asyncCall("getOutputCluts"));
}

QDBusPendingReply< RegionalClutMap > ColorServerInterface::getRegionCluts()
{
    return QDBusPendingReply< RegionalClutMap >(asyncCall("getRegionCluts"));
}

void ColorServerInterface::callFinishedSlot(QDBusPendingCallWatcher *watcher)
{
    if (watcher == m_versionInfoWatcher) {
        kDebug(1212) << "Version info call finished";
        QDBusPendingReply< uint > reply = *watcher;
        if (reply.isError()) {
            kWarning(1212) << reply.error();
            if (!m_signaledFail)
                emit updateFailed();
            m_signaledFail = true;
            return;
        } else {
            m_versionInfo = reply.value();
            m_versionInfoUpdated = true;
        }
    }

    if (watcher == m_outputClutsWatcher) {
        kDebug(1212) << "Output cluts call finished";
        QDBusPendingReply< ClutList > reply = *watcher;
        if (reply.isError()) {
            kWarning(1212) << reply.error();
            if (!m_signaledFail)
                emit updateFailed();
            m_signaledFail = true;
            return;
        } else {
            m_outputCluts = reply.value();
            m_outputClutsUpdated = true;
        }
    }

    if (watcher == m_regionClutsWatcher) {
        kDebug(1212) << "Region cluts call finished";
        QDBusPendingReply< RegionalClutMap > reply = *watcher;
        if (reply.isError()) {
            kWarning(1212) << reply.error();
            if (!m_signaledFail)
                emit updateFailed();
            m_signaledFail = true;
            return;
        } else {
            m_regionCluts = reply.value();
            m_regionClutsUpdated = true;
        }
    }

    if (m_versionInfoUpdated &&
        m_outputClutsUpdated &&
        m_regionClutsUpdated) {
        kDebug(1212) << "Update succeeded";
        emit updateSucceeded();
    }
}


/*
 * To be injected in the fragment shader sources
 */
static const char s_ccVars[] =
    "uniform sampler3D u_ccLookupTexture;\n";
static const char s_ccAlteration[] =
    "gl_FragColor.rgb = texture3D(u_ccLookupTexture, gl_FragColor.rgb / gl_FragColor.a).rgb * gl_FragColor.a;\n";
static const char s_ccAlteration_140[] =
    "fragColor.rgb = texture(u_ccLookupTexture, fragColor.rgb / fragColor.a).rgb * fragColor.a;\n";


/*
 * Color Correction
 */

ColorCorrection::ColorCorrection(QObject *parent)
    : QObject(parent)
    , d_ptr(new ColorCorrectionPrivate(this))
{
    connect(this, SIGNAL(errorOccured()), this, SIGNAL(changed()));
}

ColorCorrection::~ColorCorrection()
{
    setEnabled(false);
}

ColorCorrectionPrivate::ColorCorrectionPrivate(ColorCorrection *parent)
    : QObject(parent)
    , m_enabled(false)
    , m_hasError(false)
    , m_duringEnablingPhase(false)
    , m_ccTextureUnit(-1)
    , m_dummyCCTexture(0)
    , m_lastOutput(-1)
    , q_ptr(parent)
{
    // We need a dummy color lookup table (sRGB profile to sRGB profile)
    buildDummyClut(m_dummyClut);

    // Establish a D-Bus communication interface with KolorServer
    m_csi = new ColorServerInterface(
        "org.kde.kded",
        "/modules/kolorserver",
        QDBusConnection::sessionBus(),
        this);

    m_outputCluts = &m_csi->outputCluts();

    connect(m_csi, SIGNAL(updateSucceeded()), this, SLOT(colorServerUpdateSucceededSlot()));
    connect(m_csi, SIGNAL(updateFailed()), this, SLOT(colorServerUpdateFailedSlot()));
}

ColorCorrectionPrivate::~ColorCorrectionPrivate()
{

}

bool ColorCorrection::isEnabled() const
{
    Q_D(const ColorCorrection);
    return d->m_enabled;
}

bool ColorCorrection::setEnabled(bool enabled)
{
    Q_D(ColorCorrection);

    if (enabled == d->m_enabled)
        return true;

    if (enabled && d->m_hasError) {
        kError(1212) << "cannot enable color correction because of a previous error";
        return false;
    }

#ifdef KWIN_HAVE_OPENGLES
    const GLPlatform *gl = GLPlatform::instance();
    if (enabled && gl->isGLES() && glTexImage3D == 0) {
        kError(1212) << "color correction is not supported on OpenGL ES without OES_texture_3D";
        d->m_hasError = true;
        return false;
    }
#endif // KWIN_HAVE_OPENGLES

    if (enabled) {
        // Update all profiles and regions
        d->m_csi->update();
        kDebug(1212) << "color correction will be enabled after contacting KolorManager";
        d->m_duringEnablingPhase = true;
        // d->m_enabled will be set to true in colorServerUpdateSucceededSlot()
    } else {
        d->deleteCCTextures();
        d->m_enabled = false;
        GLShader::sColorCorrect = false;
        kDebug(1212) << "color correction has been disabled";

        // Reload all shaders
        ShaderManager::cleanup();
        ShaderManager::instance();
    }

    return true;
}

void ColorCorrection::setupForOutput(int screen)
{
    Q_D(ColorCorrection);

    if (!d->m_enabled || d->m_hasError)
        return;

    GLShader *shader = ShaderManager::instance()->getBoundShader();
    if (!shader) {
        kError(1212) << "no bound shader for color correction setup";
        d->m_hasError = true;
        emit errorOccured();
        return;
    }

    if (d->m_ccTextureUnit < 0) {
        GLint maxUnits = 0;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits);
        d->m_ccTextureUnit = maxUnits - 1;
    }

    if (!shader->setUniform(GLShader::ColorCorrectionLookupTextureUnit, d->m_ccTextureUnit)) {
        kError(1212) << "unable to set uniform for the color correction lookup texture";
        d->m_hasError = true;
        emit errorOccured();
        return;
    }

    if (!d->setupCCTextures()) {
        kError(1212) << "unable to setup color correction textures";
        d->m_hasError = true;
        emit errorOccured();
        return;
    }

    GLint activeTexture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glActiveTexture(GL_TEXTURE0 + d->m_ccTextureUnit);

    if (d->m_outputCCTextures.isEmpty() || screen < 0 || screen >= d->m_outputCCTextures.count()) {
        // Configure with a dummy texture in case something is wrong
        Q_ASSERT(d->m_dummyCCTexture != 0);
        glBindTexture(GL_TEXTURE_3D, d->m_dummyCCTexture);
    } else {
        // Everything looks ok, configure with the proper color correction texture
        glBindTexture(GL_TEXTURE_3D, d->m_outputCCTextures[screen]);
    }

    glActiveTexture(activeTexture);

    d->m_lastOutput = screen;
}

void ColorCorrection::reset()
{
    setupForOutput(-1);
}

QByteArray ColorCorrection::prepareFragmentShader(const QByteArray &sourceCode)
{
    bool sourceIsValid = true;

    /*
     * Detect comments to ignore them later
     */
    QList< QPair< int, int > > comments;
    int beginIndex, endIndex = 0;
    int i1, i2;

    enum {ctNone, ctBegin, ctEnd} commentType;
    QByteArrayMatcher commentBegin1("/*"), commentBegin2("//");
    QByteArrayMatcher commentEnd1("*/"), commentEnd2("\n");

    do {
        // Determine the next comment begin index
        i1 = commentBegin1.indexIn(sourceCode, endIndex);
        i2 = commentBegin2.indexIn(sourceCode, endIndex);
        if (i1 == -1 && i2 == -1)   commentType = ctNone;
        else if (i1 == -1)          commentType = ctEnd;
        else if (i2 == -1)          commentType = ctBegin;
        else if (i1 < i2)           commentType = ctBegin;
        else                        commentType = ctEnd;
        if (commentType == ctNone)
            break;

        // Determine the comment's end index
        if (commentType == ctBegin) {
            beginIndex = i1;
            endIndex = commentEnd1.indexIn(sourceCode, beginIndex + 2);
        }
        if (commentType == ctEnd) {
            beginIndex = i2;
            endIndex = commentEnd2.indexIn(sourceCode, beginIndex + 2);
        }

        if (endIndex != -1) {
            if (commentType == ctBegin)
                endIndex ++; // adjust for "*/" to be removed
            if (commentType == ctEnd)
                endIndex --; // adjust for "\n" to be kept
            comments.append(QPair< int, int >(beginIndex, endIndex));
        } else {
            if (commentType == ctBegin)
                sourceIsValid = false;
            if (commentType == ctEnd)
                comments.append(QPair< int, int >(beginIndex, sourceCode.length()));
            break;
        }
    } while (sourceIsValid);
    if (!sourceIsValid)
        return sourceCode;

    // Create a version of the source code with the comments stripped out
    QByteArray cfSource(sourceCode); // comment-free source code
    for (int i = comments.size() - 1; i >= 0; -- i) {
        beginIndex = comments[i].first;
        endIndex = comments[i].second;
        cfSource.replace(beginIndex, endIndex - beginIndex + 1, " ");
    }

    /*
     * Browse through the code while counting braces
     * Search for "void main() { ... }:
     */
    QByteArrayMatcher braceOpen("{");
    QByteArrayMatcher braceClose("}");
    QByteArrayMatcher voidKeyword("void");
    int levelOfScope = 0;
    enum {brNone, brOpen, brClose} braceType;

    int mainFuncBegin = -1; // where "void main" begins
    int mainFuncEnd = -1;   // at the closing brace of "void main"
    bool insideMainFunc = false;
    int i = 0;

    do {
        // Determine where the next brace is
        i1 = braceOpen.indexIn(cfSource, i);
        i2 = braceClose.indexIn(cfSource, i);
        if (i1 == -1 && i2 == -1)   braceType = brNone;
        else if (i1 == -1)          braceType = brClose;
        else if (i2 == -1)          braceType = brOpen;
        else if (i1 < i2)           braceType = brOpen;
        else                        braceType = brClose;
        if (braceType == brNone) {
            if (levelOfScope > 0)
                sourceIsValid = false;
            break;
        }

        // Handle opening brance (see if is from void main())
        if (braceType == brOpen) {
            if (levelOfScope == 0) {
                // Need to search between i and i1 (the last '}' and the current '{'
                QByteArray section = cfSource.mid(i, i1 - i);
                int i_void = -1;
                while ((i_void = section.indexOf("void", i_void + 1)) != -1) {
                    // Extract the subsection that begins with "void"
                    QByteArray subSection = section.mid(i_void).simplified();
                    subSection.replace('(', " ( ");
                    subSection.replace(')', " ) ");
                    QList<QByteArray> tokens = subSection.split(' ');
                    for (int i_token = tokens.size() - 1; i_token >= 0; -- i_token)
                        if (tokens[i_token].trimmed().isEmpty())
                            tokens.removeAt(i_token);
                    if (tokens.size() == 4 &&
                            tokens[0] == "void" &&
                            tokens[1] == "main" &&
                            tokens[2] == "(" &&
                            tokens[3] == ")") {
                        if (mainFuncBegin != -1) {
                            sourceIsValid = false;
                            break;
                        }
                        mainFuncBegin = i + i_void;
                        insideMainFunc = true;
                    }
                }
            }

            levelOfScope ++;
            i = i1 + 1;
        }

        // Handle closing brace (see if it is from void main())
        if (braceType == brClose) {
            levelOfScope --;
            if (levelOfScope < 0) {
                sourceIsValid = false;
                break;
            }

            if (levelOfScope == 0 && insideMainFunc) {
                mainFuncEnd = i2;
                insideMainFunc = false;
            }

            i = i2 + 1;
        }
    } while (sourceIsValid);
    sourceIsValid = sourceIsValid && mainFuncBegin != -1 && mainFuncEnd != -1;
    if (!sourceIsValid)
        return sourceCode;

    QByteArray mainFunc = cfSource.mid(mainFuncBegin, mainFuncEnd - mainFuncBegin + 1);

    /*
     * Insert color correction variables at the beginning and
     * the color correction code at the end of the main function.
     * Need to handle return "jumps" inside the main function.
     */
    if (GLPlatform::instance()->glslVersion() >= kVersionNumber(1, 40))
        mainFunc.insert(mainFunc.size() - 1, s_ccAlteration_140);
    else
        mainFunc.insert(mainFunc.size() - 1, s_ccAlteration);
    mainFunc.insert(0, s_ccVars);

    // Search for return statements inside the main function
    QByteArrayMatcher returnMatcher("return");
    i = -1;
    while ((i = returnMatcher.indexIn(mainFunc, i)) != -1) {
        i1 = mainFunc.indexOf(';', i);
        mainFunc.insert(i1 + 1, '}');
        mainFunc.insert(i, '{');
        mainFunc.insert(i + 1, s_ccAlteration);
        mainFuncEnd += strlen(s_ccAlteration) + 2;

        i = i1 + strlen(s_ccAlteration) + 2;
    }

    // Replace the main function
    cfSource.replace(mainFuncBegin, mainFuncEnd - mainFuncBegin + 1, mainFunc);

    return cfSource;
}

bool ColorCorrectionPrivate::setupCCTextures()
{
    if (!m_enabled || m_hasError) {
        kWarning(1212) << "Color correction not enabled or an error occurred, refusing to set up textures";
        return false;
    }

    // Dummy texture first
    if (!m_dummyCCTexture) {
        glGenTextures(1, &m_dummyCCTexture);
        if (!setupCCTexture(m_dummyCCTexture, m_dummyClut)) {
            kError(1212) << "unable to setup dummy color correction texture";
            m_dummyCCTexture = 0;
            return false;
        }
    }

    bool success = true;

    // Setup actual color correction textures
    if (m_outputCCTextures.isEmpty() && !m_outputCluts->isEmpty()) {
        kDebug(1212) << "setting up output color correction textures";

        const int outputCount = m_outputCluts->size();
        m_outputCCTextures.resize(outputCount);
        glGenTextures(outputCount, m_outputCCTextures.data());

        for (int i = 0; i < outputCount; ++i)
            if (!setupCCTexture(m_outputCCTextures[i], m_outputCluts->at(i))) {
                kError(1212) << "unable to set up color correction texture for output" << i;
                success = false;
            }
    }

    return success;
}

bool ColorCorrectionPrivate::deleteCCTextures()
{
    // Clear any previous GL errors
    checkGLError("deleteCCTextures-clearErrors");

    // Delete dummy texture
    if (m_dummyCCTexture) {
        glDeleteTextures(1, &m_dummyCCTexture);
        m_dummyCCTexture = 0;
    }

    // Delete actual color correction extures
    if (!m_outputCCTextures.isEmpty()) {
        glDeleteTextures(m_outputCCTextures.size(), m_outputCCTextures.data());
        m_outputCCTextures.clear();
    }

    return !checkGLError("deleteCCTextures");
}

bool ColorCorrectionPrivate::setupCCTexture(GLuint texture, const Clut& clut)
{
    if ((uint) clut.size() != CLUT_ELEMENT_COUNT) {
        kError(1212) << "cannot setup CC texture: invalid color lookup table";
        return false;
    }

    // Clear any previous GL errors
    checkGLError("setupCCTexture-clearErrors");

    glBindTexture(GL_TEXTURE_3D, texture);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

#ifndef KWIN_HAVE_OPENGLES
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16,
                 LUT_GRID_POINTS, LUT_GRID_POINTS, LUT_GRID_POINTS,
                 0, GL_RGB, GL_UNSIGNED_SHORT, clut.data());
#else
    const int textureDataSize = clut.size();
    QVector<quint8> textureData(textureDataSize);
    quint8 *pTextureData = textureData.data();
    const quint16 *pClutData = clut.data();
    for (int i = 0; i < textureDataSize; ++i)
        *(pTextureData++) = *(pClutData++) >> 8;

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB,
                 LUT_GRID_POINTS, LUT_GRID_POINTS, LUT_GRID_POINTS,
                 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.data());
#endif // KWIN_HAVE_OPENGLES

    return !checkGLError("setupCCTexture");
}

void ColorCorrectionPrivate::colorServerUpdateSucceededSlot()
{
    Q_Q(ColorCorrection);

    kDebug(1212) << "Update of color profiles succeeded";

    // Force the color correction textures to be recreated
    deleteCCTextures();

    // If this is reached after enabling color correction using ColorCorrection::setEnabled(true)
    if (m_duringEnablingPhase) {
        m_duringEnablingPhase = false;
        m_enabled = true;
        GLShader::sColorCorrect = true;
        kDebug(1212) << "Color correction has been enabled";

        // Reload all shaders
        ShaderManager::cleanup();
        if (!ShaderManager::instance()->isValid()) {
            kError(1212) << "Shader reinitialization failed, possible compile problems with the shaders "
                "altered for color-correction";
            m_hasError = true;
            kDebug(1212) << "Color correction has been disabled due to shader issues";
            m_enabled = false;
            GLShader::sColorCorrect = false;
            ShaderManager::cleanup();
            ShaderManager::instance();
        }
    }

    emit q->changed();
}

void ColorCorrectionPrivate::colorServerUpdateFailedSlot()
{
    Q_Q(ColorCorrection);

    m_duringEnablingPhase = false;

    kError(1212) << "Update of color profiles failed";
    m_hasError = true;
    emit q->errorOccured();
}

} // KWin namespace
