/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2008 Cédric Borgese <cedric.borgese@gmail.com>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_WOBBLYWINDOWS_H
#define KWIN_WOBBLYWINDOWS_H

// Include with base class for effects.
#include <kwineffects.h>

namespace KWin
{

struct ParameterSet;

/**
 * Effect which wobble windows
 **/
class WobblyWindowsEffect : public Effect
{
    Q_OBJECT
public:

    WobblyWindowsEffect();
    virtual ~WobblyWindowsEffect();

    virtual void reconfigure(ReconfigureFlags);
    virtual void prePaintScreen(ScreenPrePaintData& data, int time);
    virtual void prePaintWindow(EffectWindow* w, WindowPrePaintData& data, int time);
    virtual void paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data);
    virtual void postPaintScreen();
    virtual bool isActive() const;

    // Wobbly model parameters
    void setStiffness(qreal stiffness);
    void setDrag(qreal drag);
    void setVelocityThreshold(qreal velocityThreshold);
    void setMoveFactor(qreal factor);

    struct Pair {
        qreal x;
        qreal y;
    };

    enum WindowStatus {
        Free,
        Moving,
        Openning,
        Closing
    };

    static bool supported();

public Q_SLOTS:
    void slotWindowAdded(EffectWindow *w);
    void slotWindowClosed(EffectWindow *w);
    void slotWindowStartUserMovedResized(EffectWindow *w);
    void slotWindowStepUserMovedResized(EffectWindow *w, const QRect &geometry);
    void slotWindowFinishUserMovedResized(EffectWindow *w);
    void slotWindowMaximizeStateChanged(EffectWindow *w, bool horizontal, bool vertical);

private:

    void startMovedResized(EffectWindow* w);
    void stepMovedResized(EffectWindow* w);
    bool updateWindowWobblyDatas(EffectWindow* w, qreal time);

    struct WindowWobblyInfos {
        Pair* origin;
        Pair* position;
        Pair* velocity;
        Pair* acceleration;
        Pair* buffer;

        // if true, the physics system moves this point based only on it "normal" destination
        // given by the window position, ignoring neighbour points.
        bool* constraint;

        unsigned int width;
        unsigned int height;
        unsigned int count;

        Pair* bezierSurface;
        unsigned int bezierWidth;
        unsigned int bezierHeight;
        unsigned int bezierCount;

        WindowStatus status;

        // for closing
        QRectF closeRect;

        // for resizing. Only sides that have moved will wobble
        bool can_wobble_top, can_wobble_left, can_wobble_right, can_wobble_bottom;
        QRect resize_original_rect;
    };

    QHash< const EffectWindow*,  WindowWobblyInfos > windows;

    QRegion m_updateRegion;

    qreal m_stiffness;
    qreal m_drag;
    qreal m_move_factor;

    // the default tesselation for windows
    // use qreal instead of int as I really often need
    // these values as real to do divisions.
    qreal m_xTesselation;
    qreal m_yTesselation;

    qreal m_minVelocity;
    qreal m_maxVelocity;
    qreal m_stopVelocity;
    qreal m_minAcceleration;
    qreal m_maxAcceleration;
    qreal m_stopAcceleration;

    bool m_moveEffectEnabled;
    bool m_openEffectEnabled;
    bool m_closeEffectEnabled;

    bool m_moveWobble; // Expands m_moveEffectEnabled
    bool m_resizeWobble;

    void initWobblyInfo(WindowWobblyInfos& wwi, QRect geometry) const;
    void freeWobblyInfo(WindowWobblyInfos& wwi) const;
    void wobblyOpenInit(WindowWobblyInfos& wwi) const;
    void wobblyCloseInit(WindowWobblyInfos& wwi, EffectWindow* w) const;

    WobblyWindowsEffect::Pair computeBezierPoint(const WindowWobblyInfos& wwi, Pair point) const;

    static void heightRingLinearMean(Pair** data_pointer, WindowWobblyInfos& wwi);

    void setParameterSet(const ParameterSet& pset);
};

} // namespace KWin

#endif // WOBBLYWINDOWS_H
