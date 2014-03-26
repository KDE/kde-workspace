/***************************************************************************
 *   Copyright (C) 2010 by Dario Freddi <drf@kde.org>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "brightnesscontrol.h"

#include "brightnessosdwidget.h"

#include "brightnesscontroladaptor.h"

#include <powerdevilbackendinterface.h>
#include <powerdevilcore.h>

#include <QtGui/QDesktopWidget>

#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KConfigGroup>
#include <KDebug>
#include <KLocalizedString>

namespace PowerDevil {
namespace BundledActions {

BrightnessControl::BrightnessControl(QObject* parent)
    : Action(parent)
{
    // DBus
    new BrightnessControlAdaptor(this);

    setRequiredPolicies(PowerDevil::PolicyAgent::ChangeScreenSettings);

    connect(core()->backend(), SIGNAL(brightnessChanged(float,PowerDevil::BackendInterface::BrightnessControlType)),
            this, SLOT(onBrightnessChangedFromBackend(float,PowerDevil::BackendInterface::BrightnessControlType)));

    KActionCollection* actionCollection = new KActionCollection( this );

    KAction* globalAction = actionCollection->addAction("Increase Screen Brightness");
    globalAction->setText(i18nc("@action:inmenu Global shortcut", "Increase Screen Brightness"));
    globalAction->setGlobalShortcut(KShortcut(Qt::Key_MonBrightnessUp));
    connect(globalAction, SIGNAL(triggered(bool)), SLOT(increaseBrightness()));

    globalAction = actionCollection->addAction("Decrease Screen Brightness");
    globalAction->setText(i18nc("@action:inmenu Global shortcut", "Decrease Screen Brightness"));
    globalAction->setGlobalShortcut(KShortcut(Qt::Key_MonBrightnessDown));
    connect(globalAction, SIGNAL(triggered(bool)), SLOT(decreaseBrightness()));
}

BrightnessControl::~BrightnessControl()
{
    if (!m_brightnessOSD.isNull()) {
        m_brightnessOSD.data()->deleteLater();
    }
}

void BrightnessControl::onProfileUnload()
{
    //
}

void BrightnessControl::onWakeupFromIdle()
{
    //
}

void BrightnessControl::onIdleTimeout(int msec)
{
    Q_UNUSED(msec);
}

void BrightnessControl::onProfileLoad()
{
    // This unparsable conditional block means: if the current profile is more
    // conservative than the previous one and the current brightness is lower
    // than the new profile
    if (((m_currentProfile == "Battery" && m_lastProfile == "AC") ||
         (m_currentProfile == "LowBattery" && (m_lastProfile == "AC" || m_lastProfile == "Battery"))) &&
        m_defaultValue > brightness()) {
        // We don't want to change anything here
        kDebug() << "Not changing brightness, the current one is lower and the profile is more conservative";
    } else if (m_defaultValue >= 0) {
        QVariantMap args;
        args["Value"] = QVariant::fromValue((float)m_defaultValue);
        trigger(args);
    }
}

void BrightnessControl::triggerImpl(const QVariantMap& args)
{
    backend()->setBrightness(args["Value"].toFloat());
    if (args["Explicit"].toBool()) {
        showBrightnessOSD(backend()->brightness());
    }
}

bool BrightnessControl::isSupported()
{
    BackendInterface::BrightnessControlsList controls = backend()->brightnessControlsAvailable();
    if (controls.key(BackendInterface::Screen).isEmpty()) {
        return false;
    }

    return true;
}

bool BrightnessControl::loadAction(const KConfigGroup& config)
{
    // Handle profile changes
    m_lastProfile = m_currentProfile;
    m_currentProfile = config.parent().name();

    kDebug() << "Profiles: " << m_currentProfile << m_lastProfile;

    if (config.hasKey("value")) {
        m_defaultValue = config.readEntry<int>("value", 50);
    } else {
        m_defaultValue = -1;
    }

    return true;
}

void BrightnessControl::showBrightnessOSD(int brightness)
{
    // code adapted from KMix
    if (m_brightnessOSD.isNull()) {
        m_brightnessOSD = new BrightnessOSDWidget(BackendInterface::Screen);
    }

    m_brightnessOSD.data()->setCurrentBrightness(brightness);
    m_brightnessOSD.data()->show();
    m_brightnessOSD.data()->activateOSD(); //Enable the hide timer

    //Center the OSD
    QDesktopWidget * desktop = qApp->desktop();
    QRect rect = desktop->screenGeometry(desktop->primaryScreen());
    QSize size = m_brightnessOSD.data()->sizeHint();
    int posX = rect.x() + (rect.width() - size.width()) / 2;
    int posY = rect.y() + 4 * rect.height() / 5;
    m_brightnessOSD.data()->setGeometry(posX, posY, size.width(), size.height());
}

void BrightnessControl::onBrightnessChangedFromBackend(float brightness, PowerDevil::BackendInterface::BrightnessControlType type)
{
    if (type == BackendInterface::Screen) {
        showBrightnessOSD(brightness);
        Q_EMIT brightnessChanged(brightness);
    }
}

int BrightnessControl::brightness() const
{
    return backend()->brightness();
}

void BrightnessControl::setBrightness(int percent)
{
    QVariantMap args;
    args["Value"] = QVariant::fromValue<float>((float)percent);
    args["Explicit"] = true;
    trigger(args);
}

void BrightnessControl::increaseBrightness()
{
    backend()->brightnessKeyPressed(BackendInterface::Increase);
}

void BrightnessControl::decreaseBrightness()
{
    backend()->brightnessKeyPressed(BackendInterface::Decrease);
}

}
}
