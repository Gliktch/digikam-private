/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : route to the right wrapper to capture the monitor profile from the desktop.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccsettings_p.h"

namespace Digikam
{

IccProfile IccSettings::Private::profileFromDesktop(QWidget* const widget)
{
    QScreen* const screen = qApp->primaryScreen();

    if (!screen)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM Desktop: no screen instance available for the application";

        return IccProfile();
    }

    QScreen* widgetScreen = screen;

    if (widget)
    {
        QWindow* winHandle = widget->windowHandle();

        if (!winHandle)
        {
            if (QWidget* const nativeParent = widget->nativeParentWidget())
            {
                winHandle = nativeParent->windowHandle();
            }
        }

        if (winHandle)
        {
            widgetScreen = winHandle->screen();
        }
    }

    int screenNumber = qMax(qApp->screens().indexOf(widgetScreen), 0);

    // Look in the hash-table cache to see if profile do not exists yet.

    IccProfile profile;
    {
        QMutexLocker lock(&mutex);

        if (screenProfiles.contains(screenNumber))
        {
            profile = screenProfiles.value(screenNumber);

            return profile;
        }
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "ICM Desktop: looking for the monitor color profile...";

#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)

    if      (qApp->platformName() != QLatin1String("wayland"))
    {

#   ifdef HAVE_X11

        if (!profileFromX11(screen, screenNumber, profile))
        {
            return IccProfile();
        }

#   endif

    }
    else if (qApp->platformName() == QLatin1String("wayland"))
    {

#   ifdef HAVE_DBUS

        if (!profileFromWayland(screen, screenNumber, profile))
        {
            return IccProfile();
        }

#   endif

    }

#elif defined Q_OS_WIN

    if (!profileFromWindows(screen, screenNumber, profile))
    {
        return IccProfile();
    }

#elif defined Q_OS_DARWIN

    if (!profileFromMacos(screen, screenNumber, profile))
    {
        return IccProfile();
    }

#else

    // Unsupported platform

    qCWarning(DIGIKAM_DIMG_LOG) << "ICM Desktop: the color management from this screen platform is not supported";

    return IccProfile();

#endif

    // Insert the profile to cache even if null.

    {
        QMutexLocker lock(&mutex);
        screenProfiles.insert(screenNumber, profile);
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "ICM Desktop: screen platform is :" << qApp->platformName();
    qCDebug(DIGIKAM_DIMG_LOG) << "ICM Desktop: screen number      :" << screenNumber;
    QScreen* const actualScreen = qApp->screens().value(screenNumber);
    qCDebug(DIGIKAM_DIMG_LOG) << "ICM Desktop: screen description :" << actualScreen->name() << "|" << actualScreen->model();
    qCDebug(DIGIKAM_DIMG_LOG) << "ICM Desktop: prolile description:" << profile.description();

    return profile;
}

} // namespace Digikam
