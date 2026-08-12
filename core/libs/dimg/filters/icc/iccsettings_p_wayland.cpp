/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : wrapper to capture the monitor profile under Linux/Wayland.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccsettings_p.h"

namespace Digikam
{

bool IccSettings::Private::profileFromWayland(QScreen* const screen,
                                              int screenNumber,
                                              IccProfile& profile)
{
    Q_UNUSED(screen);

#ifdef HAVE_DBUS

    qCDebug(DIGIKAM_DIMG_LOG) << "ICM Wayland: check the monitor profile for screen"
                              << screenNumber;

    // Connecting to D-Bus system.

    QDBusConnection bus = QDBusConnection::systemBus();

    if (!bus.isConnected())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Wayland: cannot open a connection to D-Bus system.";

        return false;
    }

    // D-Bus colord interface.

    QDBusInterface colordInterface
    (
        QLatin1String("org.freedesktop.ColorManager"),
        QLatin1String("/org/freedesktop/ColorManager"),
        QLatin1String("org.freedesktop.ColorManager"),
        bus
    );

    if (!colordInterface.isValid())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Wayland: cannot dial with the colord service:"
                                    << bus.lastError().message();

        return false;
    }

    // Get the screens list

    QDBusReply<QList<QDBusObjectPath>> devicesReply = colordInterface.call(QLatin1String("GetDevices"));

    if (!devicesReply.isValid())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Wayland: error while to get the screens list:"
                                    << devicesReply.error().message();

        return false;
    }

    // Check if the screen number is valid.

    if ((screenNumber < 0) || (screenNumber >= devicesReply.value().size()))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Wayland: invalid screen number:"
                                    << screenNumber;

        return false;
    }

    // Get the device corresponding to the screen number.

    QDBusObjectPath devicePath = devicesReply.value().at(screenNumber);

    QDBusInterface deviceInterface
    (
        QLatin1String("org.freedesktop.ColorManager"),
        devicePath.path(),
        QLatin1String("org.freedesktop.ColorManager.Device"),
        bus
    );

    if (!deviceInterface.isValid())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Wayland: device interface invalid:"
                                    << devicePath.path();

        return false;
    }

    // Check if the device is a monitor.

    QVariant kindVariant = deviceInterface.property("Kind");

    if (!kindVariant.isValid() || (kindVariant.toString() != QLatin1String("display")))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Wayland: the device is not a screen:"
                                    << devicePath.path();

        return false;
    }

    // Get the default profile.

    QDBusReply<QDBusObjectPath> profilePathReply = deviceInterface.call(QLatin1String("GetProfile"));

    if (!profilePathReply.isValid())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Wayland: cannot get the profil data for"
                                    << devicePath.path();

        return false;
    }

    QDBusInterface profileInterface
    (
        QLatin1String("org.freedesktop.ColorManager"),
        profilePathReply.value().path(),
        QLatin1String("org.freedesktop.ColorManager.Profile"),
        bus
    );

    if (!profileInterface.isValid())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Wayland: invalid interface for the profil:"
                                    << profilePathReply.value().path();

        return false;
    }

    // Get the profile data.

    QDBusReply<QByteArray> iccDataReply = profileInterface.call(QLatin1String("GetData"));

    if (!iccDataReply.isValid())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Wayland: cannot get the profile"
                                    << profilePathReply.value().path();

        return false;
    }

    if (!iccDataReply.value().isEmpty())
    {
        profile = IccProfile(iccDataReply.value());

        qCDebug(DIGIKAM_DIMG_LOG) << "ICM Wayland: found monitor profile for screen"
                                  << screenNumber << ":" << profile.description();
    }

    return true;

#else

    Q_UNUSED(screen);
    Q_UNUSED(screenNumber);
    Q_UNUSED(profile);

    return false;

#endif

}

} // namespace Digikam
