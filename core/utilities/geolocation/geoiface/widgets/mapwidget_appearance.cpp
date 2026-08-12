/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : world map widget library - Appearance management
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

void MapWidget::stopThumbnailTimer()
{
    d->currentBackend->updateMarkers();
    d->thumbnailTimerCount++;

    if (d->thumbnailTimerCount == 10)
    {
        d->thumbnailTimer->stop();
        d->thumbnailsHaveBeenLoaded = true;
    }
}

void MapWidget::slotZoomIn()
{
    if (!currentBackendReady())
    {
        return;
    }

    d->currentBackend->zoomIn();
}

void MapWidget::slotZoomOut()
{
    if (!currentBackendReady())
    {
        return;
    }

    d->currentBackend->zoomOut();
}

void MapWidget::getColorInfos(const int clusterIndex,
                              QColor* fillColor,
                              QColor* strokeColor,
                              Qt::PenStyle* strokeStyle,
                              QString* labelText,
                              QColor* labelColor,
                              const GeoGroupState* const overrideSelection,
                              const int* const overrideCount) const
{
    /// @todo Call the new getColorInfos function!

    const GeoIfaceCluster& cluster = s->clusterList.at(clusterIndex);

    /// @todo Check that this number is already valid!

    const int nMarkers             = overrideCount ? *overrideCount : cluster.markerCount;

    getColorInfos((overrideSelection ? *overrideSelection : cluster.groupState),
                  nMarkers,
                  fillColor, strokeColor, strokeStyle, labelText, labelColor);
}

void MapWidget::getColorInfos(const GeoGroupState groupState,
                              const int nMarkers,
                              QColor* fillColor, QColor* strokeColor,
                              Qt::PenStyle* strokeStyle, QString* labelText,
                              QColor* labelColor) const
{
    if      (nMarkers < 1000)
    {
        *labelText = QString::number(nMarkers);
    }
    else if ((nMarkers >= 1000) && (nMarkers <= 1950))      // cppcheck-suppress knownConditionTrueFalse
    {
        *labelText = QString::fromLatin1("%L1k").arg(qreal(nMarkers) / 1000.0, 0, 'f', 1);
    }
    else if ((nMarkers >= 1951) && (nMarkers < 19500))      // cppcheck-suppress knownConditionTrueFalse
    {
        *labelText = QString::fromLatin1("%L1k").arg(qreal(nMarkers) / 1000.0, 0, 'f', 0);
    }
    else
    {
        // convert to "1E5" notation for numbers >=20k:

        qreal exponent           = floor(log((qreal)nMarkers) / log((qreal)10));
        qreal nMarkersFirstDigit = round(qreal(nMarkers) / pow(10, exponent));

        if (nMarkersFirstDigit >= 10)
        {
            nMarkersFirstDigit = round(nMarkersFirstDigit / 10.0);
            exponent++;
        }

        *labelText = QString::fromLatin1("%1E%2").arg(int(nMarkersFirstDigit)).arg(int(exponent));
    }

    *labelColor  = QColor(Qt::black);
    *strokeStyle = Qt::NoPen;

    /// @todo On my system, digikam uses QColor(67, 172, 232) as the selection color. Or should we just use blue?

    switch (groupState & SelectedMask)
    {
        case SelectedNone:
        {
            *strokeStyle = Qt::SolidLine;
            *strokeColor = QColor(Qt::black);
            break;
        }

        case SelectedSome:
        {
            *strokeStyle = Qt::DotLine;
            *strokeColor = QColor(Qt::blue); // 67, 172, 232);
            break;
        }

        case SelectedAll:
        {
            *strokeStyle = Qt::SolidLine;
            *strokeColor = QColor(Qt::blue); // 67, 172, 232);
            break;
        }
    }

    /**
     * @todo These are the fill colors for the circles, for cases in which only some or all of the images
     * are positively filtered. Filtering is implemented in GeoIface, but the code here has not been adapted yet.
     */
    QColor fillAll, fillSome, fillNone;

    if      (nMarkers >= 100)
    {
        fillAll  = QColor(255, 0,   0);
        fillSome = QColor(255, 188, 125);
        fillNone = QColor(255, 185, 185);
    }
    else if (nMarkers >= 50)
    {
        fillAll  = QColor(255, 127, 0);
        fillSome = QColor(255, 190, 125);
        fillNone = QColor(255, 220, 185);
    }
    else if (nMarkers >= 10)
    {
        fillAll  = QColor(255, 255, 0);
        fillSome = QColor(255, 255, 105);
        fillNone = QColor(255, 255, 185);
    }
    else if (nMarkers >= 2)
    {
        fillAll  = QColor(0,   255, 0);
        fillSome = QColor(125, 255, 125);
        fillNone = QColor(185, 255, 255);
    }
    else
    {
        fillAll  = QColor(0,   255, 255);
        fillSome = QColor(125, 255, 255);
        fillNone = QColor(185, 255, 255);
    }

    *fillColor = fillAll;
/*
    switch (groupState)
    {
        case PartialAll:
        {
            *fillColor = fillAll;
            break;
        }

        case PartialSome:
        {
            *fillColor = fillSome;
            break;
        }

        case PartialNone:
        {
            if (haveAnySolo)
            {
                *fillColor = fillNone;
            }
            else
            {
                *fillColor = fillAll;
            }

            break;
        }
    }
*/
}

QString MapWidget::convertZoomToBackendZoom(const QString& someZoom,
                                            const QString& targetBackend) const
{
    const QStringList zoomParts = someZoom.split(QLatin1Char(':'));

    GEOIFACE_ASSERT(zoomParts.count() == 2);

    const QString sourceBackend = zoomParts.first();

    if (sourceBackend == targetBackend)
    {
        return someZoom;
    }

    const int sourceZoom = zoomParts.last().toInt();
    int targetZoom       = -1;

#ifdef HAVE_GEOLOCATION

    // all of these values were found experimentally!

    if (targetBackend == QLatin1String("marble" ))
    {
        if      (sourceZoom == 0) { targetZoom =  900; }
        else if (sourceZoom == 1) { targetZoom =  970; }
        else if (sourceZoom == 2) { targetZoom = 1108; }
        else if (sourceZoom == 3) { targetZoom = 1250; }
        else if (sourceZoom == 4) { targetZoom = 1384; }
        else if (sourceZoom == 5) { targetZoom = 1520; }
        else if (sourceZoom == 6) { targetZoom = 1665; }
        else if (sourceZoom == 7) { targetZoom = 1800; }
        else if (sourceZoom == 8) { targetZoom = 1940; }
        else if (sourceZoom == 9) { targetZoom = 2070; }
        else if (sourceZoom ==10) { targetZoom = 2220; }
        else if (sourceZoom ==11) { targetZoom = 2357; }
        else if (sourceZoom ==12) { targetZoom = 2510; }
        else if (sourceZoom ==13) { targetZoom = 2635; }
        else if (sourceZoom ==14) { targetZoom = 2775; }
        else if (sourceZoom ==15) { targetZoom = 2900; }
        else if (sourceZoom ==16) { targetZoom = 3051; }
        else if (sourceZoom ==17) { targetZoom = 3180; }
        else if (sourceZoom ==18) { targetZoom = 3295; }
        else if (sourceZoom ==19) { targetZoom = 3450; }
        else                      { targetZoom = 3500; } /// @todo Find values for level 20 and up
    }

#endif

    if (targetBackend == QLatin1String("googlemaps" ))
    {
        if      (sourceZoom <= 900) { targetZoom =  0; }
        else if (sourceZoom <= 970) { targetZoom =  1; }
        else if (sourceZoom <=1108) { targetZoom =  2; }
        else if (sourceZoom <=1250) { targetZoom =  3; }
        else if (sourceZoom <=1384) { targetZoom =  4; }
        else if (sourceZoom <=1520) { targetZoom =  5; }
        else if (sourceZoom <=1665) { targetZoom =  6; }
        else if (sourceZoom <=1800) { targetZoom =  7; }
        else if (sourceZoom <=1940) { targetZoom =  8; }
        else if (sourceZoom <=2070) { targetZoom =  9; }
        else if (sourceZoom <=2220) { targetZoom = 10; }
        else if (sourceZoom <=2357) { targetZoom = 11; }
        else if (sourceZoom <=2510) { targetZoom = 12; }
        else if (sourceZoom <=2635) { targetZoom = 13; }
        else if (sourceZoom <=2775) { targetZoom = 14; }
        else if (sourceZoom <=2900) { targetZoom = 15; }
        else if (sourceZoom <=3051) { targetZoom = 16; }
        else if (sourceZoom <=3180) { targetZoom = 17; }
        else if (sourceZoom <=3295) { targetZoom = 18; }
        else if (sourceZoom <=3450) { targetZoom = 19; }
        else                        { targetZoom = 20; } /// @todo Find values for level 20 and up
    }

    GEOIFACE_ASSERT(targetZoom >= 0);

    return QString::fromLatin1("%1:%2").arg(targetBackend).arg(targetZoom);
}


void MapWidget::setShowThumbnails(const bool state)
{
    s->showThumbnails = state;

    rebuildConfigurationMenu();
    slotUpdateActionsEnabled();
    slotRequestLazyReclustering();
}

void MapWidget::slotShowThumbnailsChanged()
{
    setShowThumbnails(d->actionShowThumbnails->isChecked());
}

void MapWidget::setSortKey(const int sortKey)
{
    s->sortKey = sortKey;

    // this is probably faster than writing a function that changes all the clusters icons...
    /// @todo We just need to update the display, no need to recluster?

    slotRequestLazyReclustering();
}

void MapWidget::setThumnailSize(const int newThumbnailSize)
{
    s->thumbnailSize = qMax(GeoIfaceMinThumbnailSize, newThumbnailSize);

    // make sure the grouping radius is larger than the thumbnail size

    if (2 * s->thumbnailGroupingRadius < newThumbnailSize)
    {
        /// @todo more straightforward way for this?

        s->thumbnailGroupingRadius = newThumbnailSize / 2 + newThumbnailSize % 2;
    }

    if (s->showThumbnails)
    {
        slotRequestLazyReclustering();
    }

    slotUpdateActionsEnabled();
}

void MapWidget::setThumbnailGroupingRadius(const int newGroupingRadius)
{
    s->thumbnailGroupingRadius = qMax(GeoIfaceMinThumbnailGroupingRadius, newGroupingRadius);

    // make sure the thumbnails are smaller than the grouping radius

    if ((2 * s->thumbnailGroupingRadius) < s->thumbnailSize)
    {
        s->thumbnailSize = 2 * newGroupingRadius;
    }

    if (s->showThumbnails)
    {
        slotRequestLazyReclustering();
    }

    slotUpdateActionsEnabled();
}

void MapWidget::setMarkerGroupingRadius(const int newGroupingRadius)
{
    s->markerGroupingRadius = qMax(GeoIfaceMinMarkerGroupingRadius, newGroupingRadius);

    if (!s->showThumbnails)
    {
        slotRequestLazyReclustering();
    }

    slotUpdateActionsEnabled();
}

void MapWidget::slotDecreaseThumbnailSize()
{
    if (!s->showThumbnails)
    {
        return;
    }

    if (s->thumbnailSize > GeoIfaceMinThumbnailSize)
    {
        const int newThumbnailSize = qMax(GeoIfaceMinThumbnailSize, s->thumbnailSize - 5);

        // make sure the grouping radius is also decreased
        // this will automatically decrease the thumbnail size as well

        setThumbnailGroupingRadius(newThumbnailSize / 2);
    }
}

void MapWidget::slotIncreaseThumbnailSize()
{
    if (!s->showThumbnails)
    {
        return;
    }

    setThumnailSize(s->thumbnailSize+5);
}

int MapWidget::getThumbnailSize() const
{
    return s->thumbnailSize;
}

int MapWidget::getUndecoratedThumbnailSize() const
{
    return (s->thumbnailSize - 2);
}

} // namespace Digikam
