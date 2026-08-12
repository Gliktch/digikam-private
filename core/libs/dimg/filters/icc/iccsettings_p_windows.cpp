/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : wrapper to capture the monitor profile under Windows.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccsettings_p.h"

#ifdef Q_OS_WIN
#   include <Windows.h>
#   include <WcsPlugin.h>
#   include <winuser.h>
#   include <ICM.h>
#   include <Wingdi.h>
#endif

namespace Digikam
{

bool IccSettings::Private::profileFromWindows(QScreen* const screen,
                                              int screenNumber,
                                              IccProfile& profile)
{

#ifdef Q_OS_WIN

    Q_UNUSED(screen);

    qCDebug(DIGIKAM_DIMG_LOG) << "ICM Windows: check the monitor profile for screen"
                              << screenNumber;

    // Get the handle for the wanted screen device.

    POINT pt;
    pt.x               = screenNumber * GetSystemMetrics(SM_CXSCREEN);
    pt.y               = 0;
    HMONITOR hMonitor  = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

    MONITORINFOEX monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFOEX);

    if (!GetMonitorInfo(hMonitor, &monitorInfo))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Windows: cannot get the screen information";

        return false;
    }

    HDC hdcScreen      = CreateDC(NULL, monitorInfo.szDevice, NULL, NULL);      // krazy:exclude=null

    if (hdcScreen == NULL)        // krazy:exclude=null
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Windows: cannot get the screen handle";

        return false;
    }

    /**
     * Get the screen color profile.
     * Unlike Linux/X11, under Windows only the system path to the profile file can be handled,
     * not the binary data already loaded in the memory.
     */

    WCHAR profilePath[MAX_PATH];
    DWORD bytes = MAX_PATH * sizeof(WCHAR);

    if (
        !GetICMProfile(
                       hdcScreen,
                       &bytes,
                       profilePath
                      )
       )
    {
            qCWarning(DIGIKAM_DIMG_LOG) << "ICM Windows: cannot get the screen profile path";
            ReleaseDC(NULL, hdcScreen);      // krazy:exclude=null

            return false;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "ICM Windows: screen profile path:" << QString::fromWCharArray(profilePath);

    // Read the color profile file on disk.

    QFile profileFile(QString::fromWCharArray(profilePath));

    if (!profileFile.open(QIODevice::ReadOnly))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Windows: cannot open the screen profile file";
        ReleaseDC(NULL, hdcScreen);      // krazy:exclude=null

        return false;
    }

    QByteArray profileData = profileFile.readAll();
    profileFile.close();

    if (!profileData.isEmpty())
    {
        profile = IccProfile(profileData);

        qCDebug(DIGIKAM_DIMG_LOG) << "ICM Windows: found monitor profile for screen" << screenNumber
                                  << ":" << profile.description();
    }

    // Free the memory

    ReleaseDC(NULL, hdcScreen);      // krazy:exclude=null

    return true;

#else

    Q_UNUSED(screen);
    Q_UNUSED(screenNumber);
    Q_UNUSED(profile);

    return false;

#endif

}

} // namespace Digikam
