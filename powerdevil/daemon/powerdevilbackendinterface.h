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


#ifndef POWERDEVIL_BACKENDINTERFACE_H
#define POWERDEVIL_BACKENDINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QHash>

#include <kdemacros.h>

class KJob;

namespace PowerDevil {

class KDE_EXPORT BackendInterface : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BackendInterface)

public:
    explicit BackendInterface(QObject* parent = 0);
    virtual ~BackendInterface();

    /**
     * This enum type defines the different states of the system battery.
     *
     * - NoBatteryState: No battery available
     * - Normal: The battery is at its normal charge level
     * - Warning: The battery is at its warning charge level
     * - Low: The battery is at its low charge level
     * - Critical: The battery is at its critical charge level
     */
    enum BatteryState{ NoBatteryState, Normal, Warning, Low, Critical };

    /**
     * This enum type defines the different states of the AC adapter.
     *
     * - UnknownAcAdapterState: The AC adapter has an unknown state
     * - Plugged: The AC adapter is plugged
     * - Unplugged: The AC adapter is unplugged
     */
    enum AcAdapterState{ UnknownAcAdapterState, Plugged, Unplugged };

    /**
     * This enum type defines the types of system button events.
     *
     * - UnknownButtonType: An unknown button
     * - PowerButton: A power button pressed event, generally used to turn on or off the system
     * - SleepButton: A sleep button pressed event, generally used to make the system asleep
     * - LidOpen: A laptop lid open event
     * - LidClose: A laptop lid close event
     */
    enum ButtonType{ UnknownButtonType, PowerButton, SleepButton, LidOpen, LidClose, HibernateButton };

    /**
     * This enum type defines the different suspend methods.
     *
     * - UnknownSuspendMethod: The name says it all
     * - Standby: Processes are stopped, some hardware is deactivated (ACPI S1)
     * - ToRam: Most devices are deactivated, only RAM is powered (ACPI S3)
     * - ToDisk: State of the machine is saved to disk, and it's powered down (ACPI S4)
     */
    enum SuspendMethod{ UnknownSuspendMethod = 0, Standby = 1, ToRam = 2, ToDisk = 4, HybridSuspend = 8 };

    /**
     * This type stores an OR combination of SuspendMethod values.
     */
    Q_DECLARE_FLAGS(SuspendMethods, SuspendMethod)

    /**
     * This enum defines the different types of brightness controls.
     *
     * - UnknownBrightnessControl: Unknown
     * - Screen: Brightness control for a monitor or laptop panel
     * - Keyboard: Brightness control for a keyboard backlight
     */
    enum BrightnessControlType{ UnknownBrightnessControl = 0, Screen = 1, Keyboard = 2 };

    typedef QHash<QString, BrightnessControlType> BrightnessControlsList;

    /**
     * This enum defines the different types brightness keys.
     *
     * - Increase: Key to increase brightness (Qt::Key_MonBrightnessUp or Qt::Key_KeyboardBrightnessUp)
     * - Decrease: Key to decrease brightness (Qt::Key_MonBrightnessDown or Qt::Key_KeyboardBrightnessDown)
     * - Toggle: Key to toggle backlight (Qt::Key_KeyboardBacklightOnOff)
     */
    enum BrightnessKeyType{ Increase, Decrease, Toggle };

    /**
     * This enum defines capabilities of the backend
     *
     * - SignalResumeFromSuspend: The backend is able to stream the @c resumeFromSuspend signal accurately
     */
    enum Capability { NoCapabilities = 0, SignalResumeFromSuspend = 1 };

    Q_DECLARE_FLAGS(Capabilities, Capability)

    /**
     * This struct contains information for a recall notice from the vendor
     */
    struct RecallNotice {
        /** The battery uuid */
        QString batteryId;
        /** The vendor's name */
        QString vendor;
        /** The vendor's website */
        QString url;
    };

    /**
     * Initializes the backend. This function @b MUST be called before the backend is usable. Using
     * any method in BackendInterface without initializing it might lead to undefined behavior. The signal
     * @c backendReady or @c backendError will be streamed upon completion.
     *
     * @note Backend implementations @b MUST reimplement this function
     */
    virtual void init() = 0;

    /**
     * @returns the capabilities of the backend
     * @see PowerDevil::BackendInterface::Capability
     */
    Capabilities capabilities() const;

    /**
     * Retrieves the current state of the system battery.
     *
     * @return the current battery state
     * @see PowerDevil::BackendInterface::BatteryState
     */
    BatteryState batteryState() const;

    /**
     * Retrieves the current estimated remaining time of the system batteries
     *
     * @return the current global estimated remaining time in milliseconds
     */
    qulonglong batteryRemainingTime() const;

    /**
     * Retrieves the current state of the system AC adapter.
     *
     * @return the current AC adapter state
     * @see PowerDevil::BackendInterface::AcAdapterState
     */
    AcAdapterState acAdapterState() const;


    /**
     * Retrieves the set of suspend methods supported by the system.
     *
     * @return the suspend methods supported by this system
     * @see PowerDevil::BackendInterface::SuspendMethod
     * @see PowerDevil::BackendInterface::SuspendMethods
     */
    SuspendMethods supportedSuspendMethods() const;

    /**
     * Requests a suspend of the system.
     *
     * @param method the suspend method to use
     * @return the job handling the operation
     */
    virtual KJob *suspend(SuspendMethod method) = 0;

    /**
     * Checks if brightness controls are enabled on this system.
     *
     * @return a list of the devices available to control
     */
    BrightnessControlsList brightnessControlsAvailable() const;

    /**
     * Gets the screen brightness.
     *
     * @param device the name of the device that you would like to control
     * @return the brightness of the device, as a percentage
     */
    virtual float brightness(BrightnessControlType type = Screen) const;

    /**
     * @returns whether the lid is closed or not.
     */
    bool isLidClosed() const;

    /**
     * Sets the screen brightness.
     *
     * @param brightness the desired screen brightness, as a percentage
     * @param device the name of the device that you would like to control
     * @return true if the brightness change succeeded, false otherwise
     */
    virtual bool setBrightness(float brightness, BrightnessControlType type = Screen) = 0;

    /**
     * Should be called when the user presses a brightness key.
     *
     * @param type the type of the brightness key press
     * @see PowerDevil::BackendInterface::BrightnessKeyType
     */
    virtual void brightnessKeyPressed(BrightnessKeyType type, BrightnessControlType controlType = Screen) = 0;

    /**
     * Retrieves the capacities of the installed batteries in percentage.
     *
     * @returns A dictionary with the battery's capacity percentage mapped to the battery uuid.
     */
    QHash< QString, uint > capacities() const;

    /**
     * Returns a list of recall notices, if available
     *
     * @return a list of recall notices
     * @see PowerDevil::BackendInterface::RecallNotice
     */
    QList< RecallNotice > recallNotices() const;

Q_SIGNALS:
    /**
     * This signal is emitted when the AC adapter is plugged or unplugged.
     *
     * @param newState the new state of the AC adapter, it's one of the
     * type @see PowerDevil::BackendInterface::AcAdapterState
     */
    void acAdapterStateChanged(PowerDevil::BackendInterface::AcAdapterState newState);

    /**
     * This signal is emitted when the system battery state changed.
     *
     * @param newState the new state of the system battery, it's one of the
     * type @see PowerDevil::BackendInterface::BatteryState
     */
    void batteryStateChanged(PowerDevil::BackendInterface::BatteryState newState);

    /**
     * This signal is emitted when a button has been pressed.
     *
     * @param buttonType the pressed button type, it's one of the
     * type @see PowerDevil::BackendInterface::ButtonType
     */
    void buttonPressed(PowerDevil::BackendInterface::ButtonType buttonType);

    /**
     * This signal is emitted when the brightness changes.
     *
     * @param brightness the new brightness level
     */
    void brightnessChanged(float brightness, PowerDevil::BackendInterface::BrightnessControlType type);

    /**
     * This signal is emitted when the estimated battery remaining time changes.
     *
     * @param time the new remaining time
     */
    void batteryRemainingTimeChanged(qulonglong time);

    /**
     * This signal is emitted when the backend is ready to be used
     *
     * @see init
     */
    void backendReady();

    /**
     * This signal is emitted if the backend could not be initialized
     *
     * @param error Details about the error occurred
     * @see init
     */
    void backendError(const QString &error);

    /**
     * This signal is emitted when the PC is resuming from suspension
     */
    void resumeFromSuspend();

protected:
    void setCapabilities(Capabilities capabilities);

    void onBrightnessChanged(BrightnessControlType device, float brightness);
    void setBatteryRemainingTime(qulonglong time);
    void setButtonPressed(PowerDevil::BackendInterface::ButtonType type);
    void setBatteryState(PowerDevil::BackendInterface::BatteryState state);
    void setAcAdapterState(PowerDevil::BackendInterface::AcAdapterState state);

    void setCapacityForBattery(const QString &batteryId, uint percent);
    void setRecallNotices(const QList< RecallNotice > &notices);

    void setBackendIsReady(BrightnessControlsList availableBrightnessControls, SuspendMethods supportedSuspendMethods);
    void setBackendHasError(const QString &errorDetails);

protected slots:
    // This function is actually here due to HAL
    void setResumeFromSuspend();

private:
    class Private;
    Private * const d;

    friend class Core;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(PowerDevil::BackendInterface::Capabilities)
Q_DECLARE_OPERATORS_FOR_FLAGS(PowerDevil::BackendInterface::SuspendMethods)

#endif // POWERDEVIL_BACKENDINTERFACE_H
