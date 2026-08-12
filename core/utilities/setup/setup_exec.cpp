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

bool Setup::execDialog(Page page)
{
    return execDialog(nullptr, page);
}

bool Setup::execDialog(QWidget* const parent, Page page)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(page);

    return openSetup(setup);
}

bool Setup::execSinglePage(Page page)
{
    return execSinglePage(nullptr, page);
}

bool Setup::execSinglePage(QWidget* const parent, Page page)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(page);
    setup->setFaceType(Plain);

    return openSetup(setup);
}

bool Setup::execTemplateEditor(QWidget* const parent, const Template& t)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(TemplatePage);
    setup->setFaceType(Plain);
    setup->setTemplate(t);

    return openSetup(setup);
}

#ifdef HAVE_GEOLOCATION

bool Setup::execGeolocation(QWidget* const parent, int tab)
{
    QPointer<Setup> setup        = new Setup(parent);
    setup->showPage(GeolocationPage);
    setup->setFaceType(Plain);

    DConfigDlgWdgItem* const cur   = setup->currentPage();

    if (!cur)
    {
        return false;
    }

    SetupGeolocation* const widget = dynamic_cast<SetupGeolocation*>(cur->widget());

    if (!widget)
    {
        return false;
    }

    widget->setActiveTab((SetupGeolocation::GeolocationTab)tab);

    return openSetup(setup);
}

#endif

bool Setup::execMetadataFilters(QWidget* const parent, int tab)
{
    QPointer<Setup> setup        = new Setup(parent);
    setup->showPage(MetadataPage);
    setup->setFaceType(Plain);

    DConfigDlgWdgItem* const cur = setup->currentPage();

    if (!cur)
    {
        return false;
    }

    SetupMetadata* const widget  = dynamic_cast<SetupMetadata*>(cur->widget());

    if (!widget)
    {
        return false;
    }

    widget->setActiveTab(SetupMetadata::Display);
    widget->setActiveSubTab((SetupMetadata::MetadataSubTab)tab);

    return openSetup(setup);
}

bool Setup::execExifTool(QWidget* const parent)
{
    QPointer<Setup> setup        = new Setup(parent);
    setup->showPage(MetadataPage);
    setup->setFaceType(Plain);

    DConfigDlgWdgItem* const cur = setup->currentPage();

    if (!cur)
    {
        return false;
    }

    SetupMetadata* const widget  = dynamic_cast<SetupMetadata*>(cur->widget());

    if (!widget)
    {
        return false;
    }

    widget->setActiveTab(SetupMetadata::ExifTool);

    return openSetup(setup);
}

bool Setup::execLocalize(QWidget* const parent)
{
    QPointer<Setup> setup        = new Setup(parent);
    setup->showPage(MiscellaneousPage);
    setup->setFaceType(Plain);

    DConfigDlgWdgItem* const cur = setup->currentPage();

    if (!cur)
    {
        return false;
    }

    SetupMisc* const widget  = dynamic_cast<SetupMisc*>(cur->widget());

    if (!widget)
    {
        return false;
    }

    widget->setActiveTab(SetupMisc::Localize);

    return openSetup(setup);
}

} // namespace Digikam
