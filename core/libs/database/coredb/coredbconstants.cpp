/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-26
 * Description : Constants in the database
 *
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "coredbconstants.h"

namespace Digikam
{

QLatin1String InternalTagName::scannedForFaces()
{
    return QLatin1String("Scanned for Faces");
}

QLatin1String InternalTagName::needResolvingHistory()
{
    return QLatin1String("Need Resolving History");
}

QLatin1String InternalTagName::needTaggingHistoryGraph()
{
    return QLatin1String("Need Tagging History Graph");
}

QLatin1String InternalTagName::originalVersion()
{
    return QLatin1String("Original Version");
}

QLatin1String InternalTagName::currentVersion()
{
    return QLatin1String("Current Version");
}

QLatin1String InternalTagName::intermediateVersion()
{
    return QLatin1String("Intermediate Version");
}

QLatin1String InternalTagName::versionAlwaysVisible()
{
    return QLatin1String("Version Always Visible");
}

QLatin1String InternalTagName::colorLabelNone()
{
    return QLatin1String("Color Label None");
}

QLatin1String InternalTagName::colorLabelRed()
{
    return QLatin1String("Color Label Red");
}

QLatin1String InternalTagName::colorLabelOrange()
{
    return QLatin1String("Color Label Orange");
}

QLatin1String InternalTagName::colorLabelYellow()
{
    return QLatin1String("Color Label Yellow");
}

QLatin1String InternalTagName::colorLabelGreen()
{
    return QLatin1String("Color Label Green");
}

QLatin1String InternalTagName::colorLabelBlue()
{
    return QLatin1String("Color Label Blue");
}

QLatin1String InternalTagName::colorLabelMagenta()
{
    return QLatin1String("Color Label Magenta");
}

QLatin1String InternalTagName::colorLabelGray()
{
    return QLatin1String("Color Label Gray");
}

QLatin1String InternalTagName::colorLabelBlack()
{
    return QLatin1String("Color Label Black");
}

QLatin1String InternalTagName::colorLabelWhite()
{
    return QLatin1String("Color Label White");
}

QLatin1String InternalTagName::pickLabelNone()
{
    return QLatin1String("Pick Label None");
}

QLatin1String InternalTagName::pickLabelRejected()
{
    return QLatin1String("Pick Label Rejected");
}

QLatin1String InternalTagName::pickLabelPending()
{
    return QLatin1String("Pick Label Pending");
}

QLatin1String InternalTagName::pickLabelAccepted()
{
    return QLatin1String("Pick Label Accepted");
}

// ----------------------------------------------------------------------------------

QLatin1String TagPropertyName::person()
{
    return QLatin1String("person");
}

QLatin1String TagPropertyName::unknownPerson()
{
    return QLatin1String("unknownPerson");
}

QLatin1String TagPropertyName::unconfirmedPerson()
{
    return QLatin1String("unconfirmedPerson");
}

QLatin1String TagPropertyName::ignoredPerson()
{
    return QLatin1String("ignoredPerson");
}

QLatin1String TagPropertyName::faceEngineName()
{
    // kept at its historical value for compatibility

    return QLatin1String("faceEngineId");
}

QLatin1String TagPropertyName::tagKeyboardShortcut()
{
    return QLatin1String("tagKeyboardShortcut");
}

QLatin1String TagPropertyName::faceEngineUuid()
{
    return QLatin1String("faceEngineUuid");
}

// ----------------------------------------------------------------------------------

const QLatin1String ImageTagPropertyName::tagRegion()
{
    return QLatin1String("tagRegion");
}

const QLatin1String ImageTagPropertyName::autodetectedFace()
{
    return QLatin1String("autodetectedFace");
}

const QLatin1String ImageTagPropertyName::autodetectedPerson()
{
    return QLatin1String("autodetectedPerson");
}

const QLatin1String ImageTagPropertyName::ignoredFace()
{
    return QLatin1String("ignoredFace");
}

const QLatin1String ImageTagPropertyName::faceToTrain()
{
    return QLatin1String("faceToTrain");
}

const QLatin1String ImageTagPropertyName::faceTagExtendedData()
{
    return QLatin1String("faceTagExtendedData");
}

} // namespace DigiKam
