/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : Objective-C wrapper to capture the monitor profile under MacOS.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccsettings_p.h"

// MacOS headers

#import <CoreGraphics/CoreGraphics.h>
#import <ColorSync/ColorSync.h>

namespace Digikam
{

bool IccSettings::Private::profileFromMacos(QScreen* const screen,
                                            int screenNumber,
                                            IccProfile& profile)
{
    Q_UNUSED(screen);

    qCDebug(DIGIKAM_DIMG_LOG) << "ICM macOS: check the monitor profile for screen"
                              << screenNumber;

    // Get the number of active screens.

    uint32_t displayCount = 0;
    CGError error         = CGGetActiveDisplayList(0, nullptr, &displayCount);

    if (error != CGDisplayNoErr)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM macOS: cannot get the number of screens";

        return false;
    }

    if (screenNumber >= (int)displayCount)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM macOS: screen number is out of range";

        return false;
    }

    // Allocate an array to store the screen IDs

    std::vector<CGDirectDisplayID> displayIDs(displayCount);
    error = CGGetActiveDisplayList(displayCount, displayIDs.data(), &displayCount);

    if (error != CGDisplayNoErr)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM macOS: cannot get the list of screens";

        return false;
    }

    // Screens profile extraction.

    CGDirectDisplayID displayID = displayIDs[screenNumber];
    CGColorSpaceRef csprof      = CGDisplayCopyColorSpace(displayID);

    if (!csprof)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM macOS: cannot get the monitor color profile handle" << screenNumber;

        return false;
    }

    // Get the monitor color profile binary data.

    CFDataRef profileData = CGColorSpaceCopyICCData(csprof);

    if (!profileData)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM macOS: cannot get the monitor color profile binary data" << screenNumber;
        CFRelease(csprof);

        return false;
    }

    // Convert CFData to QByteArray

    QByteArray profileBytes
    (
        reinterpret_cast<const char*>(CFDataGetBytePtr(profileData)),
        static_cast<int>(CFDataGetLength(profileData))
    );

    // Populate the profile from the byte-array

    if (!profileBytes.isEmpty())
    {
        profile = IccProfile(profileBytes);

        qCDebug(DIGIKAM_DIMG_LOG) << "ICM macOS: found monitor profile for screen" << screenNumber
                                  << ":" << profile.description();
    }

    // Free memory

    CFRelease(profileData);
    CFRelease(csprof);

    return true;
}

} // namespace Digikam
