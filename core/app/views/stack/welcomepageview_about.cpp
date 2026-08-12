/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "welcomepageview_p.h"

namespace Digikam
{

QString WelcomePageView::aboutTabContent() const
{
    QString tabContent =
       i18n(
       "<h3>"
            "digiKam is an advanced open-source digital photo management application that runs on Linux, Windows, and MacOS."
       "</h3>"

       "<p>"
            "The application provides A comprehensive set of tools for importing, managing, editing, "
            "and sharing media taken with your camera. "
            "You can use digiKam’s import capabilities to easily transfer photos, raw files, and videos directly "
            "from your camera and external storage devices (SD cards, USB disks, etc.). "
            "The application allows you to configure import settings and rules that process and organize "
            "imported items on-the-fly."
       "</p>"

       "<p>"
            "To resume, with digiKam you can import, organize, enhance, search, export your collections of photo "
            "and video, and much more..."
       "</p>"

       "<p>"
            "Currently, you are in the Album view mode of digiKam. "
            "Albums are the places where your files are stored, and are identical to the folders on your hard disk."
       "</p>"

       "<p>"
            "digiKam has many powerful features which are described in the "
            "<b><a href=\"https://docs.digikam.org/en/index.html\">documentation</a></b>"
       "</p>"

       "<p>"
            "The <b><a href=\"https://www.digikam.org\">digiKam homepage</a></b> provides "
            "information about new versions of digiKam."
       "</p>"

       "<p>"
            "To contact the digiKam users community or to support the development team, follow the links "
            "provided in the application <b>Help</b> menu entry. "
       "</p>"

       "<h3>"
            "Thanks to support and use the digiKam project."
       "</h3>"
    );

    return tabContent;
}

} // namespace Digikam
