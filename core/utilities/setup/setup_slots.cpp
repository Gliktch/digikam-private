/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : digiKam setup dialog.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setup_p.h"

namespace Digikam
{

void Setup::slotHelp()
{
    openOnlineDocumentation(QLatin1String("setup_application"));
}

void Setup::slotOkClicked()
{
    if (!d->cameraPage->checkSettings())
    {
        showPage(CameraPage);
        return;
    }

    if (!d->miscPage->checkSettings())
    {
        showPage(MiscellaneousPage);
        return;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);

    d->cameraPage->applySettings();
    d->databasePage->applySettings();
    d->collectionsPage->applySettings();
    d->albumViewPage->applySettings();
    d->tooltipPage->applySettings();
    d->metadataPage->applySettings();
    d->templatePage->applySettings();
    d->lighttablePage->applySettings();
    d->surveyPage->applySettings();

#ifdef HAVE_GEOLOCATION

    d->geolocationPage->applySettings();

#endif

    d->editorPage->applySettings();
    d->iccPage->applySettings();
    d->pluginsPage->applySettings();
    d->miscPage->applySettings();

    ApplicationSettings::instance()->emitSetupChanged();
    ImportSettings::instance()->emitSetupChanged();

    qApp->restoreOverrideCursor();

    if (d->metadataPage->exifAutoRotateHasChanged())
    {
        LoadingCacheInterface::cleanThumbnailCache();
    }

    accept();
}

void Setup::slotCancelClicked()
{

#ifdef HAVE_GEOLOCATION

    d->geolocationPage->cancel();

#endif

}

} // namespace Digikam
