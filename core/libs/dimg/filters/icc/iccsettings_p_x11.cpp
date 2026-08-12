/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : wrapper to capture the monitor profile under Linux/X11.
 *
 * SPDX-FileCopyrightText: 2000      by Matthias Elter <elter at kde dot org>
 * SPDX-FileCopyrightText: 2001      by John Califf <jcaliff at compuzone dot net>
 * SPDX-FileCopyrightText: 2004      by Boudewijn Rempt <boud at valdyas dot org>
 * SPDX-FileCopyrightText: 2007      by Thomas Zander <zander at kde dot org>
 * SPDX-FileCopyrightText: 2007      by Adrian Page <adrian at pagenet dot plus dot com>
 * SPDX-FileCopyrightText: 2005-2006 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccsettings_p.h"

#if defined(Q_CC_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wvariadic-macros"
#endif

// Note must be after all other to prevent broken compilation
#ifdef HAVE_X11
#   include <climits>
#   include <X11/Xlib.h>
#   include <X11/Xatom.h>
#endif // HAVE_X11

#if defined(Q_CC_CLANG)
#    pragma clang diagnostic pop
#endif

namespace Digikam
{

bool IccSettings::Private::profileFromX11(QScreen* const screen,
                                          int screenNumber,
                                          IccProfile& profile)
{

#ifdef HAVE_X11

    qCDebug(DIGIKAM_DIMG_LOG) << "ICM X11: check the monitor profile for screen" << screenNumber;

    // Get the Display instance and the screen number.

    Display* const disp = XOpenDisplay(nullptr);

    if (!disp)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ICM X11: cannot open X11 display";

        return false;
    }

    // Get the root window and the atom name.

    Window appRootWindow;
    QString atomName;

    if (screen->virtualSiblings().size() > 1)
    {
        appRootWindow = RootWindow(disp, DefaultScreen(disp));
        atomName = QString::fromLatin1("_ICC_PROFILE_%1").arg(screenNumber);
    }
    else
    {
        appRootWindow = RootWindow(disp, screenNumber);
        atomName      = QLatin1String("_ICC_PROFILE");
    }

    // Get the ICC profil.

    Atom icc_atom = XInternAtom(disp, atomName.toLatin1().constData(), True);

    if (icc_atom == None)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "ICM X11: no ICC profile atom found for screen" << screenNumber;
        XCloseDisplay(disp);

        return false;
    }

    Atom type;
    int format;
    unsigned long nitems      = 0;
    unsigned long bytes_after = 0;
    unsigned char* str        = nullptr;

    int result = XGetWindowProperty(
        disp, appRootWindow, icc_atom,
        0, INT_MAX, False, XA_CARDINAL,
        &type, &format, &nitems, &bytes_after, &str
    );

    if ((result != Success) || !str || (nitems == 0))
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "ICM X11: no monitor profile installed for screen" << screenNumber;

        if (str)
        {
            XFree(str);
        }

        XCloseDisplay(disp);

        return false;
    }

    QByteArray bytes(reinterpret_cast<char*>(str), nitems);

    if (!bytes.isEmpty())
    {
        profile = IccProfile(bytes);
        qCDebug(DIGIKAM_DIMG_LOG) << "ICM X11: found monitor profile for screen"
                                  << screenNumber << ":" << profile.description();
    }

    XFree(str);
    XCloseDisplay(disp);

    return true;

#else

    Q_UNUSED(screen);
    Q_UNUSED(screenNumber);
    Q_UNUSED(profile);

    return false;

#endif

}

} // namespace Digikam
