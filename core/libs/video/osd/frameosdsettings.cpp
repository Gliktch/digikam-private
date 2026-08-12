/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : on screen display settings container.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "frameosdsettings.h"

// KDE includes

#include <kconfiggroup.h>

namespace Digikam
{

void FrameOsdSettings::readSettings(const KConfigGroup& group)
{
    printName            = group.readEntry("FrameOsdPrintNameEntry",            true);
    printDate            = group.readEntry("FrameOsdPrintDateEntry",            true);
    printApertureFocal   = group.readEntry("FrameOsdPrintApertureFocalEntry",   false);
    printExpoSensitivity = group.readEntry("FrameOsdPrintExpoSensitivityEntry", false);
    printMakeModel       = group.readEntry("FrameOsdPrintMakeModelEntry",       false);
    printLensModel       = group.readEntry("FrameOsdPrintLensModelEntry",       false);
    printComment         = group.readEntry("FrameOsdPrintCommentEntry",         false);
    printTitle           = group.readEntry("FrameOsdPrintTitleEntry",           false);
    printCapIfNoTitle    = group.readEntry("FrameOsdPrintCapIfNoTitleEntry",    false);
    printTags            = group.readEntry("FrameOsdPrintTagsEntry",            false);
    printRating          = group.readEntry("FrameOsdPrintRatingEntry",          false);
    osdFont              = group.readEntry("FrameOsdCaptionFontEntry",          QFontDatabase::systemFont(QFontDatabase::GeneralFont));
}

void FrameOsdSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry("FrameOsdPrintNameEntry",            printName);
    group.writeEntry("FrameOsdPrintDateEntry",            printDate);
    group.writeEntry("FrameOsdPrintApertureFocalEntry",   printApertureFocal);
    group.writeEntry("FrameOsdPrintExpoSensitivityEntry", printExpoSensitivity);
    group.writeEntry("FrameOsdPrintMakeModelEntry",       printMakeModel);
    group.writeEntry("FrameOsdPrintLensModelEntry",       printLensModel);
    group.writeEntry("FrameOsdPrintCommentEntry",         printComment);
    group.writeEntry("FrameOsdPrintTitleEntry",           printTitle);
    group.writeEntry("FrameOsdPrintCapIfNoTitleEntry",    printCapIfNoTitle);
    group.writeEntry("FrameOsdPrintTagsEntry",            printTags);
    group.writeEntry("FrameOsdPrintRatingEntry",          printRating);
    group.writeEntry("FrameOsdCaptionFontEntry",          osdFont);
}

} // namespace Digikam
