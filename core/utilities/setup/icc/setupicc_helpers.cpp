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

void SetupICC::setActiveTab(ICCTab tab)
{
    d->tab->setCurrentIndex(tab);
}

SetupICC::ICCTab SetupICC::activeTab() const
{
    return (ICCTab)d->tab->currentIndex();
}

void SetupICC::fillCombos(bool report)
{
    if (!d->enableColorManagement->isChecked())
    {
        return;
    }

    QList<IccProfile> profiles = IccSettings::instance()->allProfiles();

    if (profiles.isEmpty())
    {
        if (report)
        {
            QString message = i18n("No ICC profiles files found.");
            QMessageBox::information(this, qApp->applicationName(), message);
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "No ICC profile files found!!!";
        d->dlgBtnBox->button(QDialogButtonBox::Ok)->setEnabled(false);

        return;
    }

    d->workProfilesKC->replaceProfilesSqueezed(IccSettings::instance()->workspaceProfiles());
    d->monitorProfilesKC->replaceProfilesSqueezed(IccSettings::instance()->displayProfiles());
    d->inProfilesKC->replaceProfilesSqueezed(IccSettings::instance()->inputProfiles());
    d->proofProfilesKC->replaceProfilesSqueezed(IccSettings::instance()->outputProfiles());

    d->workProfilesKC->setNoProfileIfEmpty(i18n("No Profile Available"));
    d->monitorProfilesKC->setNoProfileIfEmpty(i18n("No Display Profile Available"));
    d->inProfilesKC->setNoProfileIfEmpty(i18n("No Input Profile Available"));
    d->proofProfilesKC->setNoProfileIfEmpty(i18n("No Output Profile Available"));

    if (d->monitorProfilesKC->count() == 0)
    {
        d->managedView->setEnabled(false);
        d->managedPreviews->setEnabled(false);
    }
    else
    {
        d->dlgBtnBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        d->managedPreviews->setEnabled(true);
    }

    if (d->workProfilesKC->count() == 0)
    {
        // If there is no workspace ICC profiles available,
        // the CM is broken and cannot be used.

        d->dlgBtnBox->button(QDialogButtonBox::Ok)->setEnabled(false);

        return;
    }

    d->dlgBtnBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void SetupICC::setWidgetsEnabled(bool enabled)
{
    d->workspaceGB->setEnabled(enabled);
    d->mismatchGB->setEnabled(enabled);
    d->missingGB->setEnabled(enabled);
    d->rawGB->setEnabled(enabled);
    d->tab->setTabEnabled(1, enabled);
    d->tab->setTabEnabled(2, enabled);
/*
    d->profilesPanel->setEnabled(enabled);
    d->advancedPanel->setEnabled(enabled);
*/
}

void SetupICC::profileInfo(const IccProfile& profile)
{
    if (profile.isNull())
    {
        QMessageBox::critical(this, i18nc("@title:window", "Profile Error"), i18n("No profile is selected."));
        return;
    }

    ICCProfileInfoDlg infoDlg(this, profile.filePath(), profile);
    dialogExec(&infoDlg);
}

bool SetupICC::iccRepositoryIsValid()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Color Management"));

    // If color management is disable, no need to check anymore.

    if (!group.readEntry(QLatin1String("EnableCM"), false))
    {
        return true;
    }

    // Can at least RawEngine profiles be opened?

    if (IccProfile::sRGB().open())
    {
        return true;
    }

    // To be valid, the ICC profiles repository must exist and be readable.

    QString extraPath = group.readEntry(QLatin1String("DefaultPath"), QString());
    QFileInfo info(extraPath);

    if (info.isDir() && info.exists() && info.isReadable())
    {
        return true;
    }

    QStringList paths = IccProfile::defaultSearchPaths();

    return (!paths.isEmpty());
}

} // namespace Digikam
