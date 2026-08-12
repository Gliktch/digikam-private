/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-11-24
 * Description : Color management setup tab.
 *
 * SPDX-FileCopyrightText: 2005-2007 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupicc_p.h"

namespace Digikam
{

void SetupICC::applySettings()
{
    ICCSettingsContainer settings;
    settings.enableCM = d->enableColorManagement->isChecked();

    if      (d->defaultAskMismatch->isChecked())
    {
        settings.defaultMismatchBehavior = ICCSettingsContainer::AskUser;
    }
    else if (d->defaultConvertMismatch->isChecked())
    {
        settings.defaultMismatchBehavior = ICCSettingsContainer::EmbeddedToWorkspace;
    }

    if      (d->defaultAskMissing->isChecked())
    {
        settings.defaultMissingProfileBehavior = ICCSettingsContainer::AskUser;
    }
    else if (d->defaultSRGBMissing->isChecked())
    {
        settings.defaultMissingProfileBehavior = ICCSettingsContainer::UseSRGB;

        if (d->defaultSRGBConvert->isChecked())
        {
            settings.defaultMissingProfileBehavior |= ICCSettingsContainer::ConvertToWorkspace;
        }
        else
        {
            settings.defaultMissingProfileBehavior |= ICCSettingsContainer::KeepProfile;
        }
    }
    else if (d->defaultWSMissing->isChecked())
    {
        settings.defaultMissingProfileBehavior = ICCSettingsContainer::UseWorkspace | ICCSettingsContainer::KeepProfile;
    }
    else if (d->defaultInputMissing->isChecked())
    {
        settings.defaultMissingProfileBehavior = ICCSettingsContainer::InputToWorkspace;
    }

    if      (d->defaultAskRaw->isChecked())
    {
        settings.defaultUncalibratedBehavior = ICCSettingsContainer::AskUser;
    }
    else if (d->defaultInputRaw->isChecked())
    {
        settings.defaultUncalibratedBehavior = ICCSettingsContainer::InputToWorkspace;
    }
    else if (d->defaultGuessRaw->isChecked())
    {
        settings.defaultUncalibratedBehavior = ICCSettingsContainer::AutomaticColors | ICCSettingsContainer::ConvertToWorkspace;
    }

    settings.iccFolder           = d->defaultPathKU->fileDlgPath();
    settings.useBPC              = d->bpcAlgorithm->isChecked();
    settings.renderingIntent     = d->renderingIntentKC->intent();
    settings.useManagedView      = d->managedView->isChecked();
    settings.useManagedPreviews  = d->managedPreviews->isChecked();

    settings.defaultInputProfile = d->inProfilesKC->currentProfile().filePath();
    settings.workspaceProfile    = d->workProfilesKC->currentProfile().filePath();
    settings.defaultProofProfile = d->proofProfilesKC->currentProfile().filePath();

    if (!IccSettings::instance()->monitorProfileFromSystem())
    {
        settings.monitorProfile = d->monitorProfilesKC->currentProfile().filePath();
    }

    IccSettings::instance()->setSettings(settings);
}

void SetupICC::readSettings(bool restore)
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    if (!restore)
    {
        d->enableColorManagement->setChecked(settings.enableCM);
    }

    d->bpcAlgorithm->setChecked(settings.useBPC);
    d->renderingIntentKC->setIntent(settings.renderingIntent);
    d->managedView->setChecked(settings.useManagedView);
    d->managedPreviews->setChecked(settings.useManagedPreviews);

    if      (settings.defaultMismatchBehavior & ICCSettingsContainer::AskUser)
    {
        d->defaultAskMismatch->setChecked(true);
    }
    else if (settings.defaultMismatchBehavior & ICCSettingsContainer::ConvertToWorkspace)
    {
        d->defaultConvertMismatch->setChecked(true);
    }

    if (settings.defaultMissingProfileBehavior & ICCSettingsContainer::AskUser)
    {
        d->defaultAskMissing->setChecked(true);
    }
    else
    {
        if      (settings.defaultMissingProfileBehavior & ICCSettingsContainer::UseSRGB)
        {
            d->defaultSRGBMissing->setChecked(true);
            d->defaultSRGBConvert->setChecked(settings.defaultMissingProfileBehavior & ICCSettingsContainer::ConvertToWorkspace);
        }
        else if (settings.defaultMissingProfileBehavior & ICCSettingsContainer::UseWorkspace)
        {
            d->defaultWSMissing->setChecked(true);
        }
        else if (settings.defaultMissingProfileBehavior & ICCSettingsContainer::UseDefaultInputProfile)
        {
            d->defaultInputMissing->setChecked(true);
        }
    }

    if      (settings.defaultUncalibratedBehavior & ICCSettingsContainer::AskUser)
    {
        d->defaultAskRaw->setChecked(true);
    }
    else if (settings.defaultUncalibratedBehavior & ICCSettingsContainer::UseDefaultInputProfile)
    {
        d->defaultInputRaw->setChecked(true);
    }
    else if (settings.defaultUncalibratedBehavior & ICCSettingsContainer::AutomaticColors)
    {
        d->defaultGuessRaw->setChecked(true);
    }

    d->defaultPathKU->setFileDlgPath(settings.iccFolder);
    fillCombos(false);

    d->workProfilesKC->setCurrentProfile(IccProfile(settings.workspaceProfile));
    d->inProfilesKC->setCurrentProfile(IccProfile(settings.defaultInputProfile));
    d->proofProfilesKC->setCurrentProfile(IccProfile(settings.defaultProofProfile));

    if (IccSettings::instance()->monitorProfileFromSystem())
    {
        d->monitorProfilesKC->clear();
        d->monitorProfilesKC->setNoProfileIfEmpty(i18n("Monitor Profile From System Settings"));
    }
    else
    {
        d->monitorProfilesKC->setCurrentProfile(IccProfile(settings.monitorProfile));
    }
}

} // namespace Digikam
