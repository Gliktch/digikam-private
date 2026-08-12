/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : world map widget library - Settings Management
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Justus Schwartz <justus at gmx dot li>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mapwidget_p.h"

namespace Digikam
{

void MapWidget::saveSettingsToGroup(KConfigGroup* const group)
{
    GEOIFACE_ASSERT(group != nullptr);

    if (!group)
    {
        return;
    }

    if (!d->currentBackendName.isEmpty())
    {
        group->writeEntry("Backend", d->currentBackendName);
    }

    group->writeEntry("Center",                    getCenter().geoUrl());
    group->writeEntry("Zoom",                      getZoom());
    group->writeEntry("Preview Single Items",      s->previewSingleItems);
    group->writeEntry("Preview Grouped Items",     s->previewGroupedItems);
    group->writeEntry("Show numbers on items",     s->showNumbersOnItems);
    group->writeEntry("Thumbnail Size",            s->thumbnailSize);
    group->writeEntry("Thumbnail Grouping Radius", s->thumbnailGroupingRadius);
    group->writeEntry("Marker Grouping Radius",    s->markerGroupingRadius);
    group->writeEntry("Show Thumbnails",           s->showThumbnails);
    group->writeEntry("Mouse Mode",                int(s->currentMouseMode));

    if (d->visibleExtraActions.testFlag(ExtraActionSticky))
    {
        group->writeEntry("Sticky Mode State", d->actionStickyMode->isChecked());
    }

    for (int i = 0 ; i < d->loadedBackends.size() ; ++i)
    {
        d->loadedBackends.at(i)->saveSettingsToGroup(group);
    }
}

void MapWidget::readSettingsFromGroup(const KConfigGroup* const group)
{
    GEOIFACE_ASSERT(group != nullptr);

    if (!group)
    {
        return;
    }

#ifdef HAVE_GEOLOCATION

    setBackend(group->readEntry("Backend", "marble"));

#endif

    // Options concerning the display of markers

    d->actionPreviewSingleItems->setChecked(group->readEntry("Preview Single Items",   true));
    d->actionPreviewGroupedItems->setChecked(group->readEntry("Preview Grouped Items", true));
    d->actionShowNumbersOnItems->setChecked(group->readEntry("Show numbers on items",  true));

    setThumnailSize(group->readEntry("Thumbnail Size",                       2 * GeoIfaceMinThumbnailSize));
    setThumbnailGroupingRadius(group->readEntry("Thumbnail Grouping Radius", 2 * GeoIfaceMinThumbnailGroupingRadius));
    setMarkerGroupingRadius(group->readEntry("Edit Grouping Radius",         GeoIfaceMinMarkerGroupingRadius));
    s->showThumbnails = group->readEntry("Show Thumbnails",                  s->showThumbnails);
    d->actionShowThumbnails->setChecked(s->showThumbnails);
    d->actionStickyMode->setChecked(group->readEntry("Sticky Mode State",    d->actionStickyMode->isChecked()));

    // let the backends load their settings

    for (int i = 0 ; i < d->loadedBackends.size() ; ++i)
    {
        d->loadedBackends.at(i)->readSettingsFromGroup(group);
    }

    // current map state

    const GeoCoordinates centerDefault    = GeoCoordinates(52.0, 6.0);
    const QString centerGeoUrl            = group->readEntry("Center", centerDefault.geoUrl());
    bool centerGeoUrlValid                = false;
    const GeoCoordinates centerCoordinate = GeoCoordinates::fromGeoUrl(centerGeoUrl, &centerGeoUrlValid);
    d->cacheCenterCoordinate              = centerGeoUrlValid ? centerCoordinate : centerDefault;
    d->cacheZoom                          = group->readEntry("Zoom", d->cacheZoom);
    s->currentMouseMode                   = GeoMouseModes(group->readEntry("Mouse Mode", int(s->currentMouseMode)));

    // propagate the loaded values to the map, if appropriate

    applyCacheToBackend();
    slotUpdateActionsEnabled();
}

void MapWidget::slotItemDisplaySettingsChanged()
{
    s->previewSingleItems  = d->actionPreviewSingleItems->isChecked();
    s->previewGroupedItems = d->actionPreviewGroupedItems->isChecked();
    s->showNumbersOnItems  = d->actionShowNumbersOnItems->isChecked();

    /// @todo Update action availability?

    /// @todo We just need to update the display, no need to recluster?

    slotRequestLazyReclustering();
}

void MapWidget::slotApplySettings()
{
    GeolocationSettings::instance()->applySettingsToWidget(this);
}

} // namespace Digikam
