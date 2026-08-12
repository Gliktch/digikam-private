/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : Item icon view interface - Search methods.
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2013 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemiconview_p.h"
#include "progressmanager.h"
#include "facesengine.h"

namespace Digikam
{

void ItemIconView::slotNewKeywordSearch()
{
    slotLeftSideBarActivate(d->searchSideBar);
    d->searchSideBar->newKeywordSearch();
}

void ItemIconView::slotNewAdvancedSearch()
{
    slotLeftSideBarActivate(d->searchSideBar);
    d->searchSideBar->newAdvancedSearch();
}

void ItemIconView::slotNewDuplicatesSearch(const QList<PAlbum*>& albums)
{
    slotLeftSideBarActivate(d->fuzzySearchSideBar);
    d->fuzzySearchSideBar->newDuplicatesSearch(albums);
}

void ItemIconView::slotNewDuplicatesSearch(const QList<TAlbum*>& albums)
{
    slotLeftSideBarActivate(d->fuzzySearchSideBar);
    d->fuzzySearchSideBar->newDuplicatesSearch(albums);
}


void ItemIconView::slotImageFindSimilar()
{
    const ItemInfo current = currentInfo();

    if (!current.isNull())
    {
        d->fuzzySearchSideBar->newSimilarSearch(current);
        slotLeftSideBarActivate(d->fuzzySearchSideBar);
    }
}

void ItemIconView::slotImageScanForFaces()
{
    FaceScanSettings settings;
    settings.readFromConfig();

    settings.task                   = FaceScanSettings::DetectAndRecognize;
    settings.alreadyScannedHandling = FaceScanSettings::Rescan;
    settings.source                 = FaceScanSettings::ItemIconView;

    // Remove possible duplicate ItemInfos.

    const auto infs = selectedInfoList(ToolsOps);

    for (const ItemInfo& info : infs)
    {
        if (!settings.infos.contains(info))
        {
            settings.infos << info;
        }
    }

    try
    {
        FacesEngine* const tool = new FacesEngine(settings);

        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotRefreshImagePreview()));

        tool->start();
    }
    catch (...)
    {
        // do nothing. Continue gracefully
    }
}

void ItemIconView::slotImageRecognizeFaces()
{
    FaceScanSettings settings;
    settings.readFromConfig();

    // TODO Faces engine : set K-nearest config

    settings.task                   = FaceScanSettings::RecognizeMarkedFaces;
    settings.alreadyScannedHandling = FaceScanSettings::AlreadyScannedHandling::Rescan;
    settings.source                 = FaceScanSettings::ItemIconView;

    // Remove possible duplicate ItemInfos.

    const auto infs = selectedInfoList(ToolsOps);

    for (const ItemInfo& info : infs)
    {
        if (!settings.infos.contains(info))
        {
            settings.infos << info;
        }
    }

    try
    {
        FacesEngine* const tool = new FacesEngine(settings);

        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotRefreshImagePreview()));
    
        tool->start();
    }
    catch (...)
    {
        // do nothing. Continue gracefully
    }
}

void ItemIconView::slotImageRemoveAllFaces()
{
    const ItemInfoList& selected = selectedInfoList(MetadataOps);
    MetadataRemover* const tool  = new MetadataRemover(selected, MetadataRemover::Faces);
    tool->start();
}

} // namespace Digikam
