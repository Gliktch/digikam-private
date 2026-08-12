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

QString WelcomePageView::featuresTabContent() const
{
    QStringList newFeatures;
    newFeatures << i18n("More than <a href=\"https://bugs.kde.org/buglist.cgi?f1=cf_versionfixedin&limit=0&o1=equals&order=bug_id&product=digikam&v1=%1\">400 user-reports</a> closed on Bugzilla.", QLatin1String(digikam_version_short));
    newFeatures << i18n("Internal QtAVPlayer has been updated to the latest upstream 2026-04-05 which brings with it a variety of bug fixes and performance improvements (Qt5 only).");
    newFeatures << i18n("Add support of customizable date format to show everywhere in the program.");
    newFeatures << i18n("Color label names are now customizable in the Labels tab from left sidebar by double clicking on text. Label strings size are limited to 32 characters.");
    newFeatures << i18n("Add regular expression wildcard support for ignored directories settings.");
    newFeatures << i18n("Split filters view into 3 separated tabs for better usability: Properties, Tags, and People.");
    newFeatures << i18n("Load and save Trash-view column states between sessions.");
    newFeatures << i18n("Clicking on GPS icon overlay opens the Geolocation tab from the left sidebar.");
    newFeatures << i18n("Add customizable columns to show Grouped and Versioned properties.");
    newFeatures << i18n("New Advanced Rename modifier Date Format.");
    newFeatures << i18n("Add support to merge existing template entries assigned in items with a new template.");
    newFeatures << i18n("Add new Slideshow option to customize the color background instead pure black in case of images transparency usage.");
    newFeatures << i18n("Add new Presentation option to customize the color background instead pure black in case of images transparency usage.");
    newFeatures << i18n("Complete rewrite the File-Transfert tool GUI for a better usability.");
    newFeatures << i18n("Complete rewrite the File-Copy tool GUI for a better usability.");
    newFeatures << i18n("Add marching-ants effect to the canvas selection rubber band.");
    newFeatures << i18n("Add pick color histogram indicators for the Curves, Levels, White Balance, and Black & White Sepia tools.");
    newFeatures << i18n("Rewrite the welcome page in more modern style.");
    newFeatures << i18n("Huge web site update about contents and design eg. screenshots, description, history, features, support, download, etc.");

    newFeatures << i18n("Sort Item improvements:")
                + appendSubList(QStringList()
                    << i18n("Add support of Album Icon-View sort items by Pick and Color Labels.")
                    << i18n("Add support of Import Icon-View sort items by Pick and Color Labels.")
                    << i18n("Add support of Album Icon-View sort items by Orientation.")
                    << i18n("Add support of Album Icon-View sort items by GPS location.")
                    << i18n("Add support of Album Icon-View sort items by File Format.")
                    << i18n("Add support of Import Icon-View sort items by File Format.")
                );

    newFeatures << i18n("Item Property improvements:")
                + appendSubList(QStringList()
                    << i18n("Add new button to copy all properties as text in clipboard.")
                    << i18n("Add new button to open current item in file manager.")
                    << i18n("If no icon-view item selected, show current album statistics.")
                );

    newFeatures << i18n("Grouped Item improvements:")
                + appendSubList(QStringList()
                    << i18n("Show items from an open group highlighted for a better usability with Icon-View.")
                    << i18n("Usability improvements of the grouped icon-view items overlay.")
                    << i18n("Thumbbar improvements to support grouped items visibility as with Icon-View.")
                );

    newFeatures << i18n("Preview Mode improvements:")
                + appendSubList(QStringList()
                    << i18n("A new Survey tool have been introduced to preview items in a separated windows and synchronized with the icon-view. It support all features available in preview mode. I can be used in fullscreen on a separated monitor to quickly review items during you workflow.")
                    << i18n("Extend Image and Video preview toolbar overlay with Labels editor.")
                    << i18n("Add new Preview toolbar buttons: exposure indicators, color management, OSD, magnifier.")
                    << i18n("Add context-menu to the video player.")
                    << i18n("Add load progress indicator on the status bar (Icon-view and LightTable).")
                    << i18n("Add trackpad gesture events support to zoom the contents, pan the canvas, and change to previous/next items.")
                    << i18n("Add a magnifier glass tool to zoom and explore a small area of the preview canvas. Tool Size and zoom factor are customizable.")
                    << i18n("Add under and over exposure indicators for the preview canvas.")
                    << i18n("Railway over the canvas is displayed as an overlay when the scrollbar are visible.")
                );

    newFeatures << i18n("Advanced Search improvements:")
                + appendSubList(QStringList()
                    << i18n("New options to find empty properties registered in the database.")
                    << i18n("Add support to search Creator and Author fields to be drop down lists populated with the values already presents in the database.")
                    << i18n("Add support to search recently modified items in the database.")
                );

    newFeatures << i18n("Update internal RAW engine to last Libraw snapshot 2026-04-06.");
    newFeatures << i18n("New RAW camera supported:")
                + appendSubList(QStringList()
                    << i18n("Canon R50 V, Canon EOS R1, EOS R5 Mark II, EOS R5 C, EOS R6 Mark II, EOS R8, EOS R50, EOS R100, EOS Ra Canon EOS Kiss M2;")
                    << i18n("Fujifilm X-T50, GFX 100S II, GFX100-II, X-T5, X-S20, X-H2, X-H2S ;")
                    << i18n("Hasselblad  CFV-50c, CFV-100c, X2D-100c ;")
                    << i18n("Leica Q3 43, D-Lux8, SL3, Q3, M11 Monochrom ;")
                    << i18n("Nikon (standard compression only): Z6-III, Z f, Z30, Z8 ;")
                    << i18n("Olympus/OM System OM-1 Mark II, TG-7, OM-5 ;")
                    << i18n("Panasonic GH7, S9, DC-G9 II, DC-ZS200D / ZS220D, DC-TZ200D / TZ202D / TZ220D, DC-S5-II, DC-GH6 ;")
                    << i18n("Pentax KF, K III Monochrome ;")
                    << i18n("Sony Sony ARW6 CRAW HQ, Sony ILCE-7M, VZV-E10M2, UMC-R10C, A9-III, ILX-LR1, A7C-II, A7CR, ILCE-6700,  ZV-1M2, ZV-E1, ILCE-7RM5 (A7R-V), ILME-FX30, A1 ;")
                    << i18n("Multiple DJI and Skydio drones ;")
                    << i18n("Multiple smartphones with DNG format recorded.")
                );

    // Add new features here...

    newFeatures << i18n("...and much more.");

    QString featureItems;

    for (int i = 0 ; i < newFeatures.count() ; ++i)
    {
        featureItems += i18n("<li>%1</li>", newFeatures.at(i));
    }

    QString tabContent = i18n("<h3>Some of the new features in this release of digiKam include (compared to digiKam version 8):</h3><ul>%1</ul>",
                         featureItems);

    return tabContent;
}

} // namespace Digikam
