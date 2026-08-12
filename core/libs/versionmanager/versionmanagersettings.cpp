/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-18
 * Description : settings container for versioning
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "versionmanagersettings.h"

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN VersionManagerSettingsConfig
{
public:

    static const QString configEnabled;
    static const QString configIntermediateAfterEachSession;
    static const QString configIntermediateAfterRawConversion;
    static const QString configIntermediateWhenNotReproducible;
    static const QString configViewShowIntermediates;
    static const QString configViewShowOriginal;
    static const QString configAutoSaveWhenClosingEditor;
    static const QString configVersionStorageFormat;
};

const QString VersionManagerSettingsConfig::configEnabled(QLatin1String("Non-Destructive Editing Enabled"));
const QString VersionManagerSettingsConfig::configIntermediateAfterEachSession(QLatin1String("Save Intermediate After Each Session"));
const QString VersionManagerSettingsConfig::configIntermediateAfterRawConversion(QLatin1String("Save Intermediate After Raw Conversion"));
const QString VersionManagerSettingsConfig::configIntermediateWhenNotReproducible(QLatin1String("Save Intermediate When Not Reproducible"));
const QString VersionManagerSettingsConfig::configViewShowIntermediates(QLatin1String("Show Intermediates in View"));
const QString VersionManagerSettingsConfig::configViewShowOriginal(QLatin1String("Show Original in View"));
const QString VersionManagerSettingsConfig::configAutoSaveWhenClosingEditor(QLatin1String("Auto-Save When Closing Editor"));
const QString VersionManagerSettingsConfig::configVersionStorageFormat(QLatin1String("Saving Format for Versions"));

void VersionManagerSettings::readFromConfig(const KConfigGroup& group)
{
    enabled                     = group.readEntry(VersionManagerSettingsConfig::configEnabled, true);
    saveIntermediateVersions    = NoIntermediates;

    if (group.readEntry(VersionManagerSettingsConfig::configIntermediateAfterEachSession, false))
    {
        saveIntermediateVersions |= AfterEachSession;
    }

    if (group.readEntry(VersionManagerSettingsConfig::configIntermediateAfterRawConversion, false))
    {
        saveIntermediateVersions |= AfterRawConversion;
    }

    if (group.readEntry(VersionManagerSettingsConfig::configIntermediateWhenNotReproducible, false))
    {
        saveIntermediateVersions |= WhenNotReproducible;
    }

    showInViewFlags             = OnlyShowCurrent;

    if (group.readEntry(VersionManagerSettingsConfig::configViewShowOriginal, true))
    {
        showInViewFlags |= ShowOriginal;
    }

    if (group.readEntry(VersionManagerSettingsConfig::configViewShowIntermediates, true))
    {
        showInViewFlags |= ShowIntermediates;
    }

    bool autoSave               = group.readEntry(VersionManagerSettingsConfig::configAutoSaveWhenClosingEditor, false);
    editorClosingMode           = autoSave ? AutoSave : AlwaysAsk;

    format                      = group.readEntry(VersionManagerSettingsConfig::configVersionStorageFormat,
                                                  QString::fromLatin1("JPG")).toUpper();
}

void VersionManagerSettings::writeToConfig(KConfigGroup& group) const
{
    group.writeEntry(VersionManagerSettingsConfig::configEnabled,                         enabled);
    group.writeEntry(VersionManagerSettingsConfig::configIntermediateAfterEachSession,    bool(saveIntermediateVersions & AfterEachSession));
    group.writeEntry(VersionManagerSettingsConfig::configIntermediateAfterRawConversion,  bool(saveIntermediateVersions & AfterRawConversion));
    group.writeEntry(VersionManagerSettingsConfig::configIntermediateWhenNotReproducible, bool(saveIntermediateVersions & WhenNotReproducible));
    group.writeEntry(VersionManagerSettingsConfig::configViewShowIntermediates,           bool(showInViewFlags & ShowIntermediates));
    group.writeEntry(VersionManagerSettingsConfig::configViewShowOriginal,                bool(showInViewFlags & ShowOriginal));
    group.writeEntry(VersionManagerSettingsConfig::configAutoSaveWhenClosingEditor,       bool(editorClosingMode == AutoSave));
    group.writeEntry(VersionManagerSettingsConfig::configVersionStorageFormat,            format);
}

} // namespace Digikam
