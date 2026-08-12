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


void SetupICC::slotUrlChanged()
{
    IccSettings::instance()->setIccPath(d->defaultPathKU->fileDlgPath());
    fillCombos(true);
}

void SetupICC::slotUrlTextChanged()
{
    d->defaultPathKU->setFileDlgPath(d->defaultPathKU->fileDlgPath());
}

void SetupICC::slotToggledEnabled()
{
    bool enabled = d->enableColorManagement->isChecked();

    setWidgetsEnabled(enabled);

    if (enabled)
    {
        readSettings(true);
    }
    else
    {
        d->dlgBtnBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void SetupICC::slotClickedWork()
{
    IccProfile profile = d->workProfilesKC->currentProfile();

    if (!profile.isNull())
    {
        profileInfo(profile);
    }
}

void SetupICC::slotClickedIn()
{
    IccProfile profile = d->inProfilesKC->currentProfile();

    if (!profile.isNull())
    {
        profileInfo(profile);
    }
}

void SetupICC::slotClickedMonitor()
{
    IccProfile profile;

    if (IccSettings::instance()->monitorProfileFromSystem())
    {
        profile = IccSettings::instance()->monitorProfile();
    }
    else
    {
        profile = d->monitorProfilesKC->currentProfile();
    }

    if (!profile.isNull())
    {
        profileInfo(profile);
    }
}

void SetupICC::slotClickedProof()
{
    IccProfile profile = d->proofProfilesKC->currentProfile();

    if (!profile.isNull())
    {
        profileInfo(profile);
    }
}

void SetupICC::slotMissingToggled(bool on)
{
    if (!on)
    {
        return;
    }

    d->defaultSRGBConvert->setEnabled(d->defaultSRGBMissing->isChecked());
}

void SetupICC::slotShowDefaultSearchPaths()
{
    QStringList defaultSearchPaths = IccProfile::defaultSearchPaths();
    QString existingPaths;

    if (defaultSearchPaths.isEmpty())
    {
        existingPaths = i18nc("none of the paths", "none");
    }
    else
    {
        existingPaths = defaultSearchPaths.join(QLatin1String("</li><li>"));
    }

#ifdef Q_OS_WIN

    QString text = i18n("On Windows, the default search paths include "
                        "<ul>"
                        "<li>%1/Windows/System32/spool/drivers/color/</li>"   // For Win10 and Win7
                        "<li>%1/Windows/Spool/Drivers/Color/</li>"            // For Win2K and WinXP
                        "<li>%2/Windows/Color/</li>"                          // For Win98 and WinMe
                        "</ul>"
                        "On your system, currently these paths exist and are scanned:"
                        "<ul>"
                        "<li>%3</li>"
                        "</ul>",
                        QDir::rootPath(),
                        QDir::rootPath(),
                        existingPaths);

#elif defined (Q_OS_MACOS)

    QString text = i18n("On macOS, the default search paths include "
                        "<ul>"
                        "<li>/System/Library/ColorSync/Profiles</li>"
                        "<li>/Library/ColorSync/Profiles</li>"
                        "<li>~/Library/ColorSync/Profiles</li>"
                        "<li>/opt/local/share/color/icc</li>"
                        "<li>%1/Contents/Resources/digikam/profiles/</li>"
                        "<li>~/.local/share/color/icc/</li>"
                        "<li>~/.local/share/icc/</li>"
                        "<li>~/.color/icc/</li>"
                        "</ul>"
                        "On your system, currently these paths exist and are scanned:"
                        "<ul>"
                        "<li>%2</li>"
                        "</ul>",
                        macOSBundlePrefix(),
                        existingPaths);
#else // Linux

    QString text = i18n("On Linux, the default search paths include "
                        "<ul>"
                        "<li>/usr/share/color/icc</li>"
                        "<li>/usr/local/share/color/icc</li>"
                        "<li>~/.local/share/color/icc/</li>"
                        "<li>~/.local/share/icc/</li>"
                        "<li>~/.color/icc/</li>"
                        "</ul>"
                        "On your system, currently these paths exist and are scanned:"
                        "<ul>"
                        "<li>%1</li>"
                        "</ul>",
                        existingPaths);

#endif

    QWhatsThis::showText(d->iccFolderLabel->mapToGlobal(QPoint(0, 0)), text, d->iccFolderLabel);
}

} // namespace Digikam
