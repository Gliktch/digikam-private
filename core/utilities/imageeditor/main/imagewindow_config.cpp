/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam image editor - Configure
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagewindow_p.h"

namespace Digikam
{

void ImageWindow::slotSetup()
{
    Setup::execDialog(this);
}

void ImageWindow::slotSetupICC()
{
    Setup::execSinglePage(this, Setup::ICCPage);
}

void ImageWindow::slotSetupChanged()
{
    applyStandardSettings();

    VersionManagerSettings versionSettings = ApplicationSettings::instance()->getVersionManagerSettings();
    d->versionManager.setSettings(versionSettings);
    m_nonDestructive                       = versionSettings.enabled;
    toggleNonDestructiveActions();

    d->imageFilterModel->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    d->imageFilterModel->setSortRole((ItemSortSettings::SortRole)ApplicationSettings::instance()->getImageSortOrder());
    d->imageFilterModel->setSortOrder((ItemSortSettings::SortOrder)ApplicationSettings::instance()->getImageSorting());
    d->imageFilterModel->setVersionItemFilterSettings(VersionItemFilterSettings(versionSettings));
    d->rightSideBar->setStyle(ApplicationSettings::instance()->getSidebarTitleStyle());
}

} // namespace Digikam
