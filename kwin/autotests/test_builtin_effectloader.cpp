/********************************************************************
KWin - the KDE window manager
This file is part of the KDE project.

Copyright (C) 2014 Martin Gräßlin <mgraesslin@kde.org>

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
#include "../effectloader.h"
#include "../effects/effect_builtins.h"
#include "mock_effectshandler.h"
#include "../scripting/scriptedeffect.h" // for mocking ScriptedEffect::create
// KDE
#include <KConfig>
#include <KConfigGroup>
// Qt
#include <QtTest/QtTest>
#include <QStringList>
Q_DECLARE_METATYPE(KWin::CompositingType)
Q_DECLARE_METATYPE(KWin::LoadEffectFlag)
Q_DECLARE_METATYPE(KWin::LoadEffectFlags)
Q_DECLARE_METATYPE(KWin::BuiltInEffect)
Q_DECLARE_METATYPE(KWin::Effect*)

namespace KWin
{

ScriptedEffect *ScriptedEffect::create(const QString &, const QString &)
{
    return nullptr;
}

}

class TestBuiltInEffectLoader : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testHasEffect_data();
    void testHasEffect();
    void testKnownEffects();
    void testSupported_data();
    void testSupported();
    void testLoadEffect_data();
    void testLoadEffect();
    void testLoadBuiltInEffect_data();
    void testLoadBuiltInEffect();
    void testLoadAllEffects();
};

void TestBuiltInEffectLoader::testHasEffect_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("expected");

    QTest::newRow("blur")                           << QStringLiteral("blur")              << true;
    QTest::newRow("with kwin4_effect_ prefix")      << QStringLiteral("kwin4_effect_blur") << true;
    QTest::newRow("case sensitive")                 << QStringLiteral("BlUR")              << true;
    QTest::newRow("Contrast")                       << QStringLiteral("contrast")          << true;
    QTest::newRow("CoverSwitch")                    << QStringLiteral("coverswitch")       << true;
    QTest::newRow("Cube")                           << QStringLiteral("cube")              << true;
    QTest::newRow("CubeSlide")                      << QStringLiteral("cubeslide")         << true;
    QTest::newRow("Dashboard")                      << QStringLiteral("dashboard")         << true;
    QTest::newRow("DesktopGrid")                    << QStringLiteral("desktopgrid")       << true;
    QTest::newRow("DimInactive")                    << QStringLiteral("diminactive")       << true;
    QTest::newRow("DimScreen")                      << QStringLiteral("dimscreen")         << true;
    QTest::newRow("FallApart")                      << QStringLiteral("fallapart")         << true;
    QTest::newRow("FlipSwitch")                     << QStringLiteral("flipswitch")        << true;
    QTest::newRow("Glide")                          << QStringLiteral("glide")             << true;
    QTest::newRow("HighlightWindow")                << QStringLiteral("highlightwindow")   << true;
    QTest::newRow("Invert")                         << QStringLiteral("invert")            << true;
    QTest::newRow("Kscreen")                        << QStringLiteral("kscreen")           << true;
    QTest::newRow("Logout")                         << QStringLiteral("logout")            << true;
    QTest::newRow("LookingGlass")                   << QStringLiteral("lookingglass")      << true;
    QTest::newRow("MagicLamp")                      << QStringLiteral("magiclamp")         << true;
    QTest::newRow("Magnifier")                      << QStringLiteral("magnifier")         << true;
    QTest::newRow("MinimizeAnimation")              << QStringLiteral("minimizeanimation") << true;
    QTest::newRow("MouseClick")                     << QStringLiteral("mouseclick")        << true;
    QTest::newRow("MouseMark")                      << QStringLiteral("mousemark")         << true;
    QTest::newRow("PresentWindows")                 << QStringLiteral("presentwindows")    << true;
    QTest::newRow("Resize")                         << QStringLiteral("resize")            << true;
    QTest::newRow("ScreenEdge")                     << QStringLiteral("screenedge")        << true;
    QTest::newRow("ScreenShot")                     << QStringLiteral("screenshot")        << true;
    QTest::newRow("Sheet")                          << QStringLiteral("sheet")             << true;
    QTest::newRow("ShowFps")                        << QStringLiteral("showfps")           << true;
    QTest::newRow("ShowPaint")                      << QStringLiteral("showpaint")         << true;
    QTest::newRow("Slide")                          << QStringLiteral("slide")             << true;
    QTest::newRow("SlideBack")                      << QStringLiteral("slideback")         << true;
    QTest::newRow("SlidingPopups")                  << QStringLiteral("slidingpopups")     << true;
    QTest::newRow("SnapHelper")                     << QStringLiteral("snaphelper")        << true;
    QTest::newRow("StartupFeedback")                << QStringLiteral("startupfeedback")   << true;
    QTest::newRow("ThumbnailAside")                 << QStringLiteral("thumbnailaside")    << true;
    QTest::newRow("TrackMouse")                     << QStringLiteral("trackmouse")        << true;
    QTest::newRow("WindowGeometry")                 << QStringLiteral("windowgeometry")    << true;
    QTest::newRow("WobblyWindows")                  << QStringLiteral("wobblywindows")     << true;
    QTest::newRow("Zoom")                           << QStringLiteral("zoom")              << true;
    QTest::newRow("Non Existing")                   << QStringLiteral("InvalidName")       << false;
    QTest::newRow("Fade - Scripted")                << QStringLiteral("fade")              << false;
    QTest::newRow("Fade - Scripted + kwin4_effect") << QStringLiteral("kwin4_effect_fade") << false;
}

void TestBuiltInEffectLoader::testHasEffect()
{
    QFETCH(QString, name);
    QFETCH(bool, expected);

    KWin::BuiltInEffectLoader loader;
    QCOMPARE(loader.hasEffect(name), expected);
}

void TestBuiltInEffectLoader::testKnownEffects()
{
    QStringList expectedEffects;
    expectedEffects << QStringLiteral("blur")
                    << QStringLiteral("contrast")
                    << QStringLiteral("coverswitch")
                    << QStringLiteral("cube")
                    << QStringLiteral("cubeslide")
                    << QStringLiteral("dashboard")
                    << QStringLiteral("desktopgrid")
                    << QStringLiteral("diminactive")
                    << QStringLiteral("dimscreen")
                    << QStringLiteral("fallapart")
                    << QStringLiteral("flipswitch")
                    << QStringLiteral("glide")
                    << QStringLiteral("highlightwindow")
                    << QStringLiteral("invert")
                    << QStringLiteral("kscreen")
                    << QStringLiteral("logout")
                    << QStringLiteral("lookingglass")
                    << QStringLiteral("magiclamp")
                    << QStringLiteral("magnifier")
                    << QStringLiteral("minimizeanimation")
                    << QStringLiteral("mouseclick")
                    << QStringLiteral("mousemark")
                    << QStringLiteral("presentwindows")
                    << QStringLiteral("resize")
                    << QStringLiteral("screenedge")
                    << QStringLiteral("screenshot")
                    << QStringLiteral("sheet")
                    << QStringLiteral("showfps")
                    << QStringLiteral("showpaint")
                    << QStringLiteral("slide")
                    << QStringLiteral("slideback")
                    << QStringLiteral("slidingpopups")
                    << QStringLiteral("snaphelper")
                    << QStringLiteral("startupfeedback")
                    << QStringLiteral("thumbnailaside")
                    << QStringLiteral("trackmouse")
                    << QStringLiteral("windowgeometry")
                    << QStringLiteral("wobblywindows")
                    << QStringLiteral("zoom");

    KWin::BuiltInEffectLoader loader;
    QStringList result = loader.listOfKnownEffects();
    QCOMPARE(result.size(), expectedEffects.size());
    qSort(result);
    for (int i = 0; i < expectedEffects.size(); ++i) {
        QCOMPARE(result.at(i), expectedEffects.at(i));
    }
}

void TestBuiltInEffectLoader::testSupported_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("expected");
    QTest::addColumn<KWin::CompositingType>("type");

    const KWin::CompositingType xc = KWin::XRenderCompositing;
    const KWin::CompositingType oc = KWin::OpenGL2Compositing;

    QTest::newRow("blur")                           << QStringLiteral("blur")              << false << xc;
    // fails for GL as it does proper tests on what's supported and doesn't just check whether it's GL
    QTest::newRow("blur-GL")                        << QStringLiteral("blur")              << false << oc;
    QTest::newRow("Contrast")                       << QStringLiteral("contrast")          << false << xc;
    // fails for GL as it does proper tests on what's supported and doesn't just check whether it's GL
    QTest::newRow("Contrast-GL")                    << QStringLiteral("contrast")          << false << oc;
    QTest::newRow("CoverSwitch")                    << QStringLiteral("coverswitch")       << false << xc;
    QTest::newRow("CoverSwitch-GL")                 << QStringLiteral("coverswitch")       << true  << oc;
    QTest::newRow("Cube")                           << QStringLiteral("cube")              << false << xc;
    QTest::newRow("Cube-GL")                        << QStringLiteral("cube")              << true  << oc;
    QTest::newRow("CubeSlide")                      << QStringLiteral("cubeslide")         << false << xc;
    QTest::newRow("CubeSlide-GL")                   << QStringLiteral("cubeslide")         << true  << oc;
    QTest::newRow("Dashboard")                      << QStringLiteral("dashboard")         << true  << xc;
    QTest::newRow("DesktopGrid")                    << QStringLiteral("desktopgrid")       << true  << xc;
    QTest::newRow("DimInactive")                    << QStringLiteral("diminactive")       << true  << xc;
    QTest::newRow("DimScreen")                      << QStringLiteral("dimscreen")         << true  << xc;
    QTest::newRow("FallApart")                      << QStringLiteral("fallapart")         << false << xc;
    QTest::newRow("FallApart-GL")                   << QStringLiteral("fallapart")         << true  << oc;
    QTest::newRow("FlipSwitch")                     << QStringLiteral("flipswitch")        << false << xc;
    QTest::newRow("FlipSwitch-GL")                  << QStringLiteral("flipswitch")        << true  << oc;
    QTest::newRow("Glide")                          << QStringLiteral("glide")             << false << xc;
    QTest::newRow("Glide-GL")                       << QStringLiteral("glide")             << true  << oc;
    QTest::newRow("HighlightWindow")                << QStringLiteral("highlightwindow")   << true  << xc;
    QTest::newRow("Invert")                         << QStringLiteral("invert")            << false << xc;
    QTest::newRow("Invert-GL")                      << QStringLiteral("invert")            << true  << oc;
    QTest::newRow("Kscreen")                        << QStringLiteral("kscreen")           << true  << xc;
    QTest::newRow("Logout")                         << QStringLiteral("logout")            << true  << xc;
    QTest::newRow("LookingGlass")                   << QStringLiteral("lookingglass")      << false << xc;
    QTest::newRow("LookingGlass-GL")                << QStringLiteral("lookingglass")      << true  << oc;
    QTest::newRow("MagicLamp")                      << QStringLiteral("magiclamp")         << false << xc;
    QTest::newRow("MagicLamp-GL")                   << QStringLiteral("magiclamp")         << true  << oc;
    QTest::newRow("Magnifier")                      << QStringLiteral("magnifier")         << true  << xc;
    QTest::newRow("MinimizeAnimation")              << QStringLiteral("minimizeanimation") << true  << xc;
    QTest::newRow("MouseClick")                     << QStringLiteral("mouseclick")        << true  << xc;
    QTest::newRow("MouseMark")                      << QStringLiteral("mousemark")         << true  << xc;
    QTest::newRow("PresentWindows")                 << QStringLiteral("presentwindows")    << true  << xc;
    QTest::newRow("Resize")                         << QStringLiteral("resize")            << true  << xc;
    QTest::newRow("ScreenEdge")                     << QStringLiteral("screenedge")        << true  << xc;
    QTest::newRow("ScreenShot")                     << QStringLiteral("screenshot")        << true  << xc;
    QTest::newRow("Sheet")                          << QStringLiteral("sheet")             << false << xc;
    QTest::newRow("Sheet-GL")                       << QStringLiteral("sheet")             << true  << oc;
    QTest::newRow("ShowFps")                        << QStringLiteral("showfps")           << true  << xc;
    QTest::newRow("ShowPaint")                      << QStringLiteral("showpaint")         << true  << xc;
    QTest::newRow("Slide")                          << QStringLiteral("slide")             << true  << xc;
    QTest::newRow("SlideBack")                      << QStringLiteral("slideback")         << true  << xc;
    QTest::newRow("SlidingPopups")                  << QStringLiteral("slidingpopups")     << true  << xc;
    QTest::newRow("SnapHelper")                     << QStringLiteral("snaphelper")        << true  << xc;
    QTest::newRow("StartupFeedback")                << QStringLiteral("startupfeedback")   << false << xc;
    QTest::newRow("StartupFeedback-GL")             << QStringLiteral("startupfeedback")   << true  << oc;
    QTest::newRow("ThumbnailAside")                 << QStringLiteral("thumbnailaside")    << true  << xc;
    QTest::newRow("TrackMouse")                     << QStringLiteral("trackmouse")        << true  << xc;
    QTest::newRow("WindowGeometry")                 << QStringLiteral("windowgeometry")    << true  << xc;
    QTest::newRow("WobblyWindows")                  << QStringLiteral("wobblywindows")     << false << xc;
    QTest::newRow("WobblyWindows-GL")               << QStringLiteral("wobblywindows")     << true  << oc;
    QTest::newRow("Zoom")                           << QStringLiteral("zoom")              << true  << xc;
    QTest::newRow("Non Existing")                   << QStringLiteral("InvalidName")       << false << xc;
    QTest::newRow("Fade - Scripted")                << QStringLiteral("fade")              << false << xc;
    QTest::newRow("Fade - Scripted + kwin4_effect") << QStringLiteral("kwin4_effect_fade") << false << xc;
}

void TestBuiltInEffectLoader::testSupported()
{
    QFETCH(QString, name);
    QFETCH(bool, expected);
    QFETCH(KWin::CompositingType, type);

    MockEffectsHandler mockHandler(type);
    KWin::BuiltInEffectLoader loader;
    QCOMPARE(loader.isEffectSupported(name), expected);
}

void TestBuiltInEffectLoader::testLoadEffect_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("expected");
    QTest::addColumn<KWin::CompositingType>("type");

    const KWin::CompositingType xc = KWin::XRenderCompositing;
    const KWin::CompositingType oc = KWin::OpenGL2Compositing;

    QTest::newRow("blur")                           << QStringLiteral("blur")              << false << xc;
    // fails for GL as it does proper tests on what's supported and doesn't just check whether it's GL
    QTest::newRow("blur-GL")                        << QStringLiteral("blur")              << false << oc;
    QTest::newRow("Contrast")                       << QStringLiteral("contrast")          << false << xc;
    // fails for GL as it does proper tests on what's supported and doesn't just check whether it's GL
    QTest::newRow("Contrast-GL")                    << QStringLiteral("contrast")          << false << oc;
    QTest::newRow("CoverSwitch")                    << QStringLiteral("coverswitch")       << false << xc;
    // TODO: needs GL mocking
//     QTest::newRow("CoverSwitch-GL")                 << QStringLiteral("coverswitch")       << true  << oc;
    QTest::newRow("Cube")                           << QStringLiteral("cube")              << false << xc;
    // TODO: needs GL mocking
//     QTest::newRow("Cube-GL")                        << QStringLiteral("cube")              << true  << oc;
    QTest::newRow("CubeSlide")                      << QStringLiteral("cubeslide")         << false << xc;
    QTest::newRow("CubeSlide-GL")                   << QStringLiteral("cubeslide")         << true  << oc;
    QTest::newRow("Dashboard")                      << QStringLiteral("dashboard")         << true  << xc;
    QTest::newRow("DesktopGrid")                    << QStringLiteral("desktopgrid")       << true  << xc;
    QTest::newRow("DimInactive")                    << QStringLiteral("diminactive")       << true  << xc;
    QTest::newRow("DimScreen")                      << QStringLiteral("dimScreen")         << true  << xc;
    QTest::newRow("FallApart")                      << QStringLiteral("fallapart")         << false << xc;
    QTest::newRow("FallApart-GL")                   << QStringLiteral("fallapart")         << true  << oc;
    QTest::newRow("FlipSwitch")                     << QStringLiteral("flipswitch")        << false << xc;
    QTest::newRow("FlipSwitch-GL")                  << QStringLiteral("flipswitch")        << true  << oc;
    QTest::newRow("Glide")                          << QStringLiteral("glide")             << false << xc;
    QTest::newRow("Glide-GL")                       << QStringLiteral("glide")             << true  << oc;
    QTest::newRow("HighlightWindow")                << QStringLiteral("highlightwindow")   << true  << xc;
    QTest::newRow("Invert")                         << QStringLiteral("invert")            << false << xc;
    QTest::newRow("Invert-GL")                      << QStringLiteral("invert")            << true  << oc;
    QTest::newRow("Kscreen")                        << QStringLiteral("kscreen")           << true  << xc;
    QTest::newRow("Logout")                         << QStringLiteral("logout")            << true  << xc;
    QTest::newRow("LookingGlass")                   << QStringLiteral("lookingglass")      << false << xc;
    QTest::newRow("LookingGlass-GL")                << QStringLiteral("lookingglass")      << true  << oc;
    QTest::newRow("MagicLamp")                      << QStringLiteral("magiclamp")         << false << xc;
    QTest::newRow("MagicLamp-GL")                   << QStringLiteral("magiclamp")         << true  << oc;
    QTest::newRow("Magnifier")                      << QStringLiteral("magnifier")         << true  << xc;
    QTest::newRow("MinimizeAnimation")              << QStringLiteral("minimizeanimation") << true  << xc;
    QTest::newRow("MouseClick")                     << QStringLiteral("mouseclick")        << true  << xc;
    QTest::newRow("MouseMark")                      << QStringLiteral("mousemark")         << true  << xc;
    QTest::newRow("PresentWindows")                 << QStringLiteral("presentwindows")    << true  << xc;
    QTest::newRow("Resize")                         << QStringLiteral("resize")            << true  << xc;
    QTest::newRow("ScreenEdge")                     << QStringLiteral("screenedge")        << true  << xc;
    QTest::newRow("ScreenShot")                     << QStringLiteral("screenshot")        << true  << xc;
    QTest::newRow("Sheet")                          << QStringLiteral("sheet")             << false << xc;
    QTest::newRow("Sheet-GL")                       << QStringLiteral("sheet")             << true  << oc;
    // TODO: Accesses EffectFrame and crashes
//     QTest::newRow("ShowFps")                        << QStringLiteral("showfps")           << true  << xc;
    QTest::newRow("ShowPaint")                      << QStringLiteral("showpaint")         << true  << xc;
    QTest::newRow("Slide")                          << QStringLiteral("slide")             << true  << xc;
    QTest::newRow("SlideBack")                      << QStringLiteral("slideback")         << true  << xc;
    QTest::newRow("SlidingPopups")                  << QStringLiteral("slidingpopups")     << true  << xc;
    QTest::newRow("SnapHelper")                     << QStringLiteral("snaphelper")        << true  << xc;
    QTest::newRow("StartupFeedback")                << QStringLiteral("startupfeedback")   << false << xc;
    QTest::newRow("StartupFeedback-GL")             << QStringLiteral("startupfeedback")   << true  << oc;
    QTest::newRow("ThumbnailAside")                 << QStringLiteral("thumbnailaside")    << true  << xc;
    QTest::newRow("TrackMouse")                     << QStringLiteral("trackmouse")        << true  << xc;
    // TODO: Accesses EffectFrame and crashes
//     QTest::newRow("WindowGeometry")                 << QStringLiteral("windowgeometry")    << true  << xc;
    QTest::newRow("WobblyWindows")                  << QStringLiteral("wobblywindows")     << false << xc;
    QTest::newRow("WobblyWindows-GL")               << QStringLiteral("wobblywindows")     << true  << oc;
    QTest::newRow("Zoom")                           << QStringLiteral("kwin4_effect_zoom") << true  << xc;
    QTest::newRow("Non Existing")                   << QStringLiteral("InvalidName")       << false << xc;
    QTest::newRow("Fade - Scripted")                << QStringLiteral("fade")              << false << xc;
    QTest::newRow("Fade - Scripted + kwin4_effect") << QStringLiteral("kwin4_effect_fade") << false << xc;
}

void TestBuiltInEffectLoader::testLoadEffect()
{
    QFETCH(QString, name);
    QFETCH(bool, expected);
    QFETCH(KWin::CompositingType, type);

    MockEffectsHandler mockHandler(type);
    KWin::BuiltInEffectLoader loader;
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    loader.setConfig(config);

    qRegisterMetaType<KWin::Effect*>();
    QSignalSpy spy(&loader, SIGNAL(effectLoaded(KWin::Effect*,QString)));
    // connect to signal to ensure that we delete the Effect again as the Effect doesn't have a parent
    connect(&loader, &KWin::BuiltInEffectLoader::effectLoaded,
        [&name](KWin::Effect *effect, const QString &effectName) {
            QCOMPARE(effectName, name);
            effect->deleteLater();
        }
    );
    // try to load the Effect
    QCOMPARE(loader.loadEffect(name), expected);
    // loading again should fail
    QVERIFY(!loader.loadEffect(name));
    // signal spy should have got the signal if it was expected
    QCOMPARE(spy.isEmpty(), !expected);
    if (!spy.isEmpty()) {
        QCOMPARE(spy.count(), 1);
        // if we caught a signal it should have the effect name we passed in
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.count(), 2);
        QCOMPARE(arguments.at(1).toString(), name);
    }
    spy.clear();
    QVERIFY(spy.isEmpty());

    // now if we wait for the events being processed, the effect will get deleted and it should load again
    QTest::qWait(1);
    QCOMPARE(loader.loadEffect(name), expected);
    // signal spy should have got the signal if it was expected
    QCOMPARE(spy.isEmpty(), !expected);
    if (!spy.isEmpty()) {
        QCOMPARE(spy.count(), 1);
        // if we caught a signal it should have the effect name we passed in
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.count(), 2);
        QCOMPARE(arguments.at(1).toString(), name);
    }
}

void TestBuiltInEffectLoader::testLoadBuiltInEffect_data()
{
    // TODO: this test cannot yet test the checkEnabledByDefault functionality as that requires
    // mocking enough of GL to get the blur effect to think it's supported and enabled by default
    QTest::addColumn<KWin::BuiltInEffect>("effect");
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("expected");
    QTest::addColumn<KWin::CompositingType>("type");
    QTest::addColumn<KWin::LoadEffectFlags>("loadFlags");

    const KWin::CompositingType xc = KWin::XRenderCompositing;
    const KWin::CompositingType oc = KWin::OpenGL2Compositing;

    const KWin::LoadEffectFlags checkDefault = KWin::LoadEffectFlag::Load | KWin::LoadEffectFlag::CheckDefaultFunction;
    const KWin::LoadEffectFlags forceFlags = KWin::LoadEffectFlag::Load;
    const KWin::LoadEffectFlags dontLoadFlags = KWin::LoadEffectFlags();

    // enabled by default, but not supported
    QTest::newRow("blur")                     << KWin::BuiltInEffect::Blur << QStringLiteral("blur")            << false << oc << checkDefault;
    // enabled by default
    QTest::newRow("HighlightWindow")          << KWin::BuiltInEffect::HighlightWindow << QStringLiteral("highlightwindow") << true  << xc << checkDefault;
    // supported but not enabled by default
    QTest::newRow("LookingGlass-GL")          << KWin::BuiltInEffect::LookingGlass << QStringLiteral("lookingglass")    << true << oc << checkDefault;
    // not enabled by default
    QTest::newRow("MouseClick")               << KWin::BuiltInEffect::MouseClick << QStringLiteral("mouseclick")      << true << xc << checkDefault;
    // Force an Effect which will load
    QTest::newRow("MouseClick-Force")         << KWin::BuiltInEffect::MouseClick << QStringLiteral("mouseclick")      << true  << xc << forceFlags;
    // Force an Effect which is not supported
    QTest::newRow("LookingGlass-Force")       << KWin::BuiltInEffect::LookingGlass << QStringLiteral("lookingglass")    << false << xc << forceFlags;
    // Force the Effect as supported
    QTest::newRow("LookingGlass-Force-GL")    << KWin::BuiltInEffect::LookingGlass << QStringLiteral("lookingglass")    << true  << oc << forceFlags;
    // Enforce no load of effect which is enabled by default
    QTest::newRow("HighlightWindow-DontLoad") << KWin::BuiltInEffect::HighlightWindow << QStringLiteral("highlightwindow") << false << xc << dontLoadFlags;
    // Enforce no load of effect which is not enabled by default, but enforced
    QTest::newRow("MouseClick-DontLoad")      << KWin::BuiltInEffect::MouseClick << QStringLiteral("mouseclick")      << false << xc << dontLoadFlags;
}

void TestBuiltInEffectLoader::testLoadBuiltInEffect()
{
    QFETCH(KWin::BuiltInEffect, effect);
    QFETCH(QString, name);
    QFETCH(bool, expected);
    QFETCH(KWin::CompositingType, type);
    QFETCH(KWin::LoadEffectFlags, loadFlags);

    MockEffectsHandler mockHandler(type);
    KWin::BuiltInEffectLoader loader;
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    loader.setConfig(config);

    qRegisterMetaType<KWin::Effect*>();
    QSignalSpy spy(&loader, SIGNAL(effectLoaded(KWin::Effect*,QString)));
    // connect to signal to ensure that we delete the Effect again as the Effect doesn't have a parent
    connect(&loader, &KWin::BuiltInEffectLoader::effectLoaded,
        [&name](KWin::Effect *effect, const QString &effectName) {
            QCOMPARE(effectName, name);
            effect->deleteLater();
        }
    );
    // try to load the Effect
    QCOMPARE(loader.loadEffect(effect, loadFlags), expected);
    // loading again should fail
    QVERIFY(!loader.loadEffect(effect, loadFlags));

    // signal spy should have got the signal if it was expected
    QCOMPARE(spy.isEmpty(), !expected);
    if (!spy.isEmpty()) {
        QCOMPARE(spy.count(), 1);
        // if we caught a signal it should have the effect name we passed in
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.count(), 2);
        QCOMPARE(arguments.at(1).toString(), name);
    }
    spy.clear();
    QVERIFY(spy.isEmpty());

    // now if we wait for the events being processed, the effect will get deleted and it should load again
    QTest::qWait(1);
    QCOMPARE(loader.loadEffect(effect, loadFlags), expected);
    // signal spy should have got the signal if it was expected
    QCOMPARE(spy.isEmpty(), !expected);
    if (!spy.isEmpty()) {
        QCOMPARE(spy.count(), 1);
        // if we caught a signal it should have the effect name we passed in
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.count(), 2);
        QCOMPARE(arguments.at(1).toString(), name);
    }
}

void TestBuiltInEffectLoader::testLoadAllEffects()
{
    MockEffectsHandler mockHandler(KWin::XRenderCompositing);
    KWin::BuiltInEffectLoader loader;

    KSharedConfig::Ptr config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);

    // TODO: remove once the KCM doesn't use this prefix
    const QString kwin4 = QStringLiteral("kwin4_effect_");

    // prepare the configuration to hard enable/disable the effects we want to load
    KConfigGroup plugins = config->group("Plugins");
    plugins.writeEntry(kwin4 + QStringLiteral("dashboardEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("desktopgridEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("highlightwindowEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("kscreenEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("logoutEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("minimizeanimationEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("presentwindowsEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("screenedgeEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("screenshotEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("slideEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("slidingpopupsEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("startupfeedbackEnabled"), false);
    plugins.writeEntry(kwin4 + QStringLiteral("zoomEnabled"), false);
    // enable lookingglass as it's not supported
    plugins.writeEntry(kwin4 + QStringLiteral("lookingglassEnabled"), true);
    plugins.sync();

    loader.setConfig(config);

    qRegisterMetaType<KWin::Effect*>();
    QSignalSpy spy(&loader, SIGNAL(effectLoaded(KWin::Effect*,QString)));
    // connect to signal to ensure that we delete the Effect again as the Effect doesn't have a parent
    connect(&loader, &KWin::BuiltInEffectLoader::effectLoaded,
        [](KWin::Effect *effect) {
            effect->deleteLater();
        }
    );

    // the config is prepared so that no Effect gets loaded!
    loader.queryAndLoadAll();

    // we need to wait some time because it's queued
    QVERIFY(!spy.wait(10));

    // now let's prepare a config which has one effect explicitly enabled
    plugins.writeEntry(kwin4 + QStringLiteral("mouseclickEnabled"), true);
    plugins.sync();

    loader.queryAndLoadAll();
    // should load one effect in first go
    QVERIFY(spy.wait(10));
    // and afterwards it should not load another one
    QVERIFY(!spy.wait(10));

    QCOMPARE(spy.size(), 1);
    // if we caught a signal it should have the effect name we passed in
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.count(), 2);
    QCOMPARE(arguments.at(1).toString(), QStringLiteral("mouseclick"));
    spy.clear();

    // let's delete one of the default entries
    plugins.deleteEntry(kwin4 + QStringLiteral("kscreenEnabled"));
    plugins.sync();

    QVERIFY(spy.isEmpty());
    loader.queryAndLoadAll();

    // let's use qWait as we need to wait for two signals to be emitted
    QTest::qWait(100);
    QCOMPARE(spy.size(), 2);
    QStringList loadedEffects;
    for (auto &list : spy) {
        QCOMPARE(list.size(), 2);
        loadedEffects << list.at(1).toString();
    }
    qSort(loadedEffects);
    QCOMPARE(loadedEffects.at(0), QStringLiteral("kscreen"));
    QCOMPARE(loadedEffects.at(1), QStringLiteral("mouseclick"));
}

QTEST_MAIN(TestBuiltInEffectLoader)
#include "test_builtin_effectloader.moc"
