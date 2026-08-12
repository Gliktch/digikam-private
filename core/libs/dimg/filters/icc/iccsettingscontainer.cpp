/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-12-08
 * Description : ICC Settings Container.
 *
 * SPDX-FileCopyrightText: 2005-2007 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccsettingscontainer.h"

// Qt includes

#include <QStandardPaths>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "iccprofile.h"

namespace Digikam
{

void ICCSettingsContainer::readFromConfig(KConfigGroup& group)
{
    enableCM                      = group.readEntry("EnableCM", true);

    //if (!group.hasKey("OnProfileMismatch") && group.hasKey("BehaviourICC")) // legacy
    //  behavior = group.readEntry("BehaviourICC", false) ? "convert" : "ask";

    workspaceProfile              = getProfilePath(group, "WorkProfileFile");
    monitorProfile                = getProfilePath(group, "MonitorProfileFile");
    defaultInputProfile           = group.readPathEntry("InProfileFile", QString());
    defaultProofProfile           = group.readPathEntry("ProofProfileFile", QString());

    defaultMismatchBehavior       = (Behavior)group.readEntry("DefaultMismatchBehavior", (int)EmbeddedToWorkspace);
    defaultMissingProfileBehavior = (Behavior)group.readEntry("DefaultMissingProfileBehavior", (int)SRGBToWorkspace);
    defaultUncalibratedBehavior   = (Behavior)group.readEntry("DefaultUncalibratedBehavior", (int)AutoToWorkspace);

    lastMismatchBehavior          = (Behavior)group.readEntry("LastMismatchBehavior", (int)EmbeddedToWorkspace);
    lastMissingProfileBehavior    = (Behavior)group.readEntry("LastMissingProfileBehavior", (int)SRGBToWorkspace);
    lastUncalibratedBehavior      = (Behavior)group.readEntry("LastUncalibratedBehavior", (int)AutoToWorkspace);
    lastSpecifiedAssignProfile    = getProfilePath(group, "LastSpecifiedAssignProfile");
    lastSpecifiedInputProfile     = group.readEntry("LastSpecifiedInputProfile", defaultInputProfile);

    useBPC                        = group.readEntry("BPCAlgorithm", true);
    useManagedView                = group.readEntry("ManagedView", true);
    useManagedPreviews            = group.readEntry("ManagedPreviews", true);
    renderingIntent               = group.readEntry("RenderingIntent", (int)IccTransform::Perceptual);

    proofingRenderingIntent       = group.readEntry("ProofingRenderingIntent", (int)IccTransform::AbsoluteColorimetric);
    doGamutCheck                  = group.readEntry("DoGamutCheck", false);
    gamutCheckMaskColor           = group.readEntry("GamutCheckMaskColor", QColor(126, 255, 255));

    iccFolder                     = group.readEntry("DefaultPath", QString());
}

void ICCSettingsContainer::writeToConfig(KConfigGroup& group) const
{
    group.writeEntry("EnableCM", enableCM);

    if (!enableCM)
    {
        return;    // No need to write settings in this case.
    }

    group.writeEntry("DefaultMismatchBehavior",       (int)defaultMismatchBehavior);
    group.writeEntry("DefaultMissingProfileBehavior", (int)defaultMissingProfileBehavior);
    group.writeEntry("DefaultUncalibratedBehavior",   (int)defaultUncalibratedBehavior);

    group.writeEntry("LastMismatchBehavior",          (int)lastMismatchBehavior);
    group.writeEntry("LastMissingProfileBehavior",    (int)lastMissingProfileBehavior);
    group.writeEntry("LastUncalibratedBehavior",      (int)lastUncalibratedBehavior);
    group.writeEntry("LastSpecifiedAssignProfile",    lastSpecifiedAssignProfile);
    group.writeEntry("LastSpecifiedInputProfile",     lastSpecifiedInputProfile);

    group.writeEntry("BPCAlgorithm",                  useBPC);
    group.writeEntry("ManagedView",                   useManagedView);
    group.writeEntry("ManagedPreviews",               useManagedPreviews);
    group.writeEntry("RenderingIntent",               renderingIntent);

    group.writePathEntry("WorkProfileFile",           workspaceProfile);
    group.writePathEntry("MonitorProfileFile",        monitorProfile);
    group.writePathEntry("InProfileFile",             defaultInputProfile);
    group.writePathEntry("ProofProfileFile",          defaultProofProfile);

    group.writeEntry("ProofingRenderingIntent",       proofingRenderingIntent);
    group.writeEntry("DoGamutCheck",                  doGamutCheck);
    group.writeEntry("GamutCheckMaskColor",           gamutCheckMaskColor);

    group.writeEntry("DefaultPath",                   iccFolder);
}

void ICCSettingsContainer::writeManagedViewToConfig(KConfigGroup& group) const
{
    // Save Color Managed View setting in config file. For performance
    // reason, no need to flush file, it cached in memory and will be flushed
    // to disk at end of session.

    group.writeEntry("ManagedView", useManagedView);
}

void ICCSettingsContainer::writeManagedPreviewsToConfig(KConfigGroup& group) const
{
    // Save Color Managed Previews setting in config file. For performance
    // reason, no need to flush file, it cached in memory and will be flushed
    // to disk at end of session.

    group.writeEntry("ManagedPreviews", useManagedView);
}

QString ICCSettingsContainer::getProfilePath(KConfigGroup& group, const char* key) const
{
    QString sRGB         = IccProfile::sRGB().filePath();
    QString profilesPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  QLatin1String("digikam/profiles/"),
                                                  QStandardPaths::LocateDirectory);

    QString configPath   = group.readPathEntry(key, sRGB);

    if (!QFileInfo::exists(configPath))
    {
        configPath = profilesPath + QFileInfo(configPath).fileName();

        if (!QFileInfo::exists(configPath))
        {
            configPath = sRGB;
        }

        group.writePathEntry(key, configPath);
    }

    return configPath;
}

} // namespace Digikam
