/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-07
 * Description : Context menu for GPS list view.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gpsitemlistcontextmenu.h"

// Qt includes:

#include <QEvent>
#include <QContextMenuEvent>
#include <QClipboard>
#include <QApplication>
#include <QDomDocument>
#include <QPointer>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "gpsundocommand.h"
#include "gpscommon.h"
#include "gpsitemcontainer.h"
#include "lookupfactory.h"
#include "gpsbookmarkowner.h"

namespace Digikam
{

class Q_DECL_HIDDEN GPSItemListContextMenu::Private
{
public:

    Private() = default;

public:

    bool                     enabled                        = true;

    QAction*                 actionBookmark                 = nullptr;
    GPSBookmarkOwner*        bookmarkOwner                  = nullptr;

    QAction*                 actionCopy                     = nullptr;
    QAction*                 actionPaste                    = nullptr;
    QAction*                 actionPasteSwap                = nullptr;
    QAction*                 actionRemoveCoordinates        = nullptr;
    QAction*                 actionRemoveAltitude           = nullptr;
    QAction*                 actionRemoveUncertainty        = nullptr;
    QAction*                 actionRemoveSpeed              = nullptr;
    QAction*                 actionLookupMissingAltitudes   = nullptr;

    GPSItemList*             imagesList                     = nullptr;

    /// Altitude lookup
    QPointer<LookupAltitude> altitudeLookup;
    GPSUndoCommand*          altitudeUndoCommand            = nullptr;
    int                      altitudeRequestedCount         = 0;
    int                      altitudeReceivedCount          = 0;
};

GPSItemListContextMenu::GPSItemListContextMenu(GPSItemList* const imagesList,
                                               GPSBookmarkOwner* const bookmarkOwner)
    : QObject(imagesList),
      d      (new Private)
{
    d->imagesList                   = imagesList;

    d->actionCopy                   = new QAction(i18n("Copy coordinates"),                this);
    d->actionCopy->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
    d->actionPaste                  = new QAction(i18n("Paste coordinates"),               this);
    d->actionPaste->setIcon(QIcon::fromTheme(QLatin1String("edit-paste")));
    d->actionPasteSwap              = new QAction(i18n("Paste coordinates swapped"),       this);
    d->actionPasteSwap->setIcon(QIcon::fromTheme(QLatin1String("edit-paste")));
    d->actionRemoveCoordinates      = new QAction(i18n("Remove coordinates"),              this);
    d->actionRemoveAltitude         = new QAction(i18n("Remove altitude"),                 this);
    d->actionRemoveUncertainty      = new QAction(i18n("Remove uncertainty"),              this);
    d->actionRemoveSpeed            = new QAction(i18n("Remove speed"),                    this);
    d->actionLookupMissingAltitudes = new QAction(i18n("Look up missing altitude values"), this);

    connect(d->actionCopy, SIGNAL(triggered()),
            this, SLOT(copyActionTriggered()));

    connect(d->actionPaste, SIGNAL(triggered()),
            this, SLOT(pasteActionTriggered()));

    connect(d->actionPasteSwap, SIGNAL(triggered()),
            this, SLOT(pasteSwapActionTriggered()));

    connect(d->actionRemoveCoordinates, SIGNAL(triggered()),
            this, SLOT(slotRemoveCoordinates()));

    connect(d->actionRemoveAltitude, SIGNAL(triggered()),
            this, SLOT(slotRemoveAltitude()));

    connect(d->actionRemoveUncertainty, SIGNAL(triggered()),
            this, SLOT(slotRemoveUncertainty()));

    connect(d->actionRemoveSpeed, SIGNAL(triggered()),
            this, SLOT(slotRemoveSpeed()));

    connect(d->actionLookupMissingAltitudes, SIGNAL(triggered()),
            this, SLOT(slotLookupMissingAltitudes()));

    if (bookmarkOwner)
    {
        d->bookmarkOwner   = bookmarkOwner;
        d->actionBookmark  = new QAction(i18n("Bookmarks"), this);
        d->actionBookmark->setMenu(d->bookmarkOwner->getMenu());

        connect(d->bookmarkOwner, SIGNAL(positionSelected(GPSDataContainer)),
                this, SLOT(slotBookmarkSelected(GPSDataContainer)));
    }

    d->imagesList->installEventFilter(this);
}

GPSItemListContextMenu::~GPSItemListContextMenu()
{
    delete d->altitudeUndoCommand;
    delete d;
}

bool GPSItemListContextMenu::eventFilter(QObject* watched, QEvent* event)
{
    // We are only interested in context-menu events.

    if ((event->type() == QEvent::ContextMenu) && d->enabled)
    {
        // Enable or disable the actions.

        const GPSItemModel* const imageModel      = d->imagesList->getModel();
        QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
        const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
        const int nSelected                       = selectedIndices.size();

        // "copy" are only available for one selected image with geo data.

        bool copyAvailable                        = (nSelected == 1);
        bool removeAltitudeAvailable              = false;
        bool removeCoordinatesAvailable           = false;
        bool removeUncertaintyAvailable           = false;
        bool removeSpeedAvailable                 = false;
        bool lookupMissingAltitudesAvailable      = false;

        for (int i = 0 ; i < nSelected ; ++i)
        {
            const GPSItemContainer* const gpsItem = imageModel->itemFromIndex(selectedIndices.at(i));

            if (gpsItem)
            {
                const GPSDataContainer gpsData   = gpsItem->gpsData();
                const bool itemHasCoordinates    = gpsData.getCoordinates().hasCoordinates();
                copyAvailable                   &= itemHasCoordinates;
                removeCoordinatesAvailable      |= itemHasCoordinates;
                removeAltitudeAvailable         |= gpsData.getCoordinates().hasAltitude();
                removeUncertaintyAvailable      |= gpsData.hasNSatellites() || gpsData.hasDop() || gpsData.hasFixType();
                removeSpeedAvailable            |= gpsData.hasSpeed();
                lookupMissingAltitudesAvailable |= itemHasCoordinates && !gpsData.getCoordinates().hasAltitude();
            }
        }

        d->actionCopy->setEnabled(copyAvailable);
        d->actionRemoveAltitude->setEnabled(removeAltitudeAvailable);
        d->actionRemoveCoordinates->setEnabled(removeCoordinatesAvailable);
        d->actionRemoveUncertainty->setEnabled(removeUncertaintyAvailable);
        d->actionRemoveSpeed->setEnabled(removeSpeedAvailable);
        d->actionLookupMissingAltitudes->setEnabled(lookupMissingAltitudesAvailable);

        if (d->bookmarkOwner)
        {
            d->bookmarkOwner->changeAddBookmark(copyAvailable);
            GPSDataContainer position;
            QUrl             itemUrl;
            getCurrentItemPositionAndUrl(&position, &itemUrl);
            const QString itemFileName = itemUrl.fileName();
            d->bookmarkOwner->setPositionAndTitle(position.getCoordinates(), itemFileName);
        }

        // "paste" is only available if there is geo data in the clipboard
        // and at least one photo is selected.

        bool pasteAvailable     = (nSelected >= 1);
        bool pasteSwapAvailable = true;

        if (pasteAvailable)
        {
            QClipboard* const clipboard = QApplication::clipboard();
            const QMimeData* mimedata   = clipboard->mimeData();
            bool hasXmlFormat           = mimedata->hasFormat(QLatin1String("application/gpx+xml"));
            pasteAvailable              = hasXmlFormat || mimedata->hasText();
            pasteSwapAvailable          = !hasXmlFormat && mimedata->hasText();
        }

        d->actionPaste->setEnabled(pasteAvailable);
        d->actionPasteSwap->setEnabled(pasteSwapAvailable);

        // Construct the context-menu.

        QMenu* const menu = new QMenu(d->imagesList);
        menu->addAction(d->actionCopy);
        menu->addAction(d->actionPaste);
        menu->addAction(d->actionPasteSwap);
        menu->addSeparator();
        menu->addAction(d->actionRemoveCoordinates);
        menu->addAction(d->actionRemoveAltitude);
        menu->addAction(d->actionRemoveUncertainty);
        menu->addAction(d->actionRemoveSpeed);
        menu->addAction(d->actionLookupMissingAltitudes);

        if (d->actionBookmark)
        {
            menu->addSeparator();
            menu->addAction(d->actionBookmark);
            d->actionBookmark->setEnabled(nSelected >= 1);
        }

        QContextMenuEvent* const e = static_cast<QContextMenuEvent*>(event);
        menu->exec(e->globalPos());

        delete menu;

        return true;
    }
    else
    {
        return QObject::eventFilter(watched, event);
    }
}

bool GPSItemListContextMenu::getCurrentItemPositionAndUrl(GPSDataContainer* const gpsInfo,
                                                          QUrl* const itemUrl)
{
    // NOTE: currentIndex does not seem to work any more since we use KLinkItemSelectionModel.

    const GPSItemModel* const imageModel      = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();

    if (selectedIndices.size() == 0)
    {
        return false;
    }

    const QModelIndex firstIndex              = selectedIndices.first();
    const GPSItemContainer* const firstItem   = imageModel->itemFromIndex(firstIndex);

    if (firstItem)
    {
        for (int i = 1 ; i < selectedIndices.size() ; ++i)
        {
            if (selectedIndices.at(i).isValid())
            {
                const GPSItemContainer* const gpsItem = imageModel->itemFromIndex(selectedIndices.at(i));

                if (!gpsItem || (firstItem->gpsData() != gpsItem->gpsData()))
                {
                    if (itemUrl)
                    {
                        *itemUrl = QUrl(i18n("Items have different coordinates."));
                    }

                    return false;
                }
            }
        }

        if (gpsInfo)
        {
            *gpsInfo = firstItem->gpsData();
        }

        if (itemUrl)
        {
            *itemUrl = firstItem->url();
        }

        return true;
    }

    return false;
}

void GPSItemListContextMenu::copyActionTriggered()
{
    GPSDataContainer gpsInfo;
    QUrl itemUrl;

    if (!getCurrentItemPositionAndUrl(&gpsInfo, &itemUrl))
    {
        return;
    }

    coordinatesToClipboard(gpsInfo.getCoordinates(), itemUrl, QString());
}

void GPSItemListContextMenu::pasteSwapActionTriggered()
{
    pasteActionTriggered(true);
}

void GPSItemListContextMenu::pasteActionTriggered(bool swap)
{
    // Extract the coordinates from the clipboard.

    QClipboard* const clipboard = QApplication::clipboard();
    const QMimeData* mimedata   = clipboard->mimeData();

    GPSDataContainer gpsData;
    bool foundData              = false;

    if (mimedata->hasFormat(QLatin1String("application/gpx+xml")))
    {
        const QByteArray data = mimedata->data(QLatin1String("application/gpx+xml"));
        bool xmlOkay          = true;
        bool foundDoubleData  = false;

        // Code adapted from gpsdataparser.cpp.

        QDomDocument gpxDoc(QLatin1String("gpx"));

        if (!gpxDoc.setContent(data))
        {
            xmlOkay = false;
        }

        if (xmlOkay)
        {
            const QDomElement gpxDocElem = gpxDoc.documentElement();

            if (gpxDocElem.tagName() != QLatin1String("gpx"))
            {
                xmlOkay = false;
            }

            if (xmlOkay)
            {
                for (QDomNode nWpt = gpxDocElem.firstChild() ; !nWpt.isNull() ; nWpt = nWpt.nextSibling())
                {
                    const QDomElement wptElem = nWpt.toElement();

                    if (wptElem.isNull())
                    {
                        continue;
                    }

                    if (wptElem.tagName() != QLatin1String("wpt"))
                    {
                        continue;
                    }

                    double ptAltitude  = 0.0;
                    double ptLatitude  = 0.0;
                    double ptLongitude = 0.0;
                    bool haveAltitude  = false;

                    // Get GPS position. If not available continue to next point.

                    const QString lat  = wptElem.attribute(QLatin1String("lat"));
                    const QString lon  = wptElem.attribute(QLatin1String("lon"));

                    if (lat.isEmpty() || lon.isEmpty())
                    {
                        continue;
                    }

                    ptLatitude         = lat.toDouble();
                    ptLongitude        = lon.toDouble();

                    if (foundData)
                    {
                        foundDoubleData = true;
                        break;
                    }

                    // Get metadata of way point (altitude and time stamp).

                    for (QDomNode nWptMeta = wptElem.firstChild() ; !nWptMeta.isNull() ; nWptMeta = nWptMeta.nextSibling())
                    {
                        const QDomElement wptMetaElem = nWptMeta.toElement();

                        if (wptMetaElem.isNull())
                        {
                            continue;
                        }

                        if (wptMetaElem.tagName() == QLatin1String("ele"))
                        {
                            // Get GPS point altitude. If not available continue to next point.

                            QString ele = wptMetaElem.text();

                            if (!ele.isEmpty())
                            {
                                ptAltitude = ele.toDouble(&haveAltitude);
                                break;
                            }
                        }
                    }

                    foundData = true;
                    GeoCoordinates coordinates(ptLatitude, ptLongitude);

                    if (haveAltitude)
                    {
                        coordinates.setAlt(ptAltitude);
                    }

                    gpsData.setCoordinates(coordinates);
                }
            }
        }

        if (foundDoubleData)
        {
            QMessageBox::information(d->imagesList,
                                     i18nc("@title:window", "Geolocation Editor"),
                                     i18n("Found more than one point on the clipboard - can only assign one point at a time."));
        }
    }

    if (!foundData && mimedata->hasText())
    {
        const QString textdata         = mimedata->text();
        bool foundGeoUrl               = false;
        GeoCoordinates testCoordinates = GeoCoordinates::fromGeoUrl(textdata, &foundGeoUrl);

        if (foundGeoUrl)
        {
            gpsData.setCoordinates(testCoordinates);
            foundData = true;
        }
        else
        {
            /// @todo this is legacy code from before we used geo-url.

            QLocale locale;
            QString cordText = textdata.trimmed();
            QStringList separators({QLatin1String(","),
                                    QLatin1String("/"),
                                    QLatin1String(":"),
                                    QLatin1String(";"),
                                    QLatin1String(" ")});
            int firstPoint   =  cordText.indexOf(QLatin1Char('.'));

            if ((firstPoint == -1) || (firstPoint > 4))
            {
                locale = QLocale(QLocale::French);

                if (cordText.count(QLatin1Char(',')) > 1)
                {
                    separators.removeOne(QLatin1String(","));
                }
            }
            else
            {
                locale = QLocale(QLocale::C);
            }

            QStringList parts;
            bool foundParts = false;

            while (!separators.isEmpty())
            {
                parts = cordText.split(separators.takeFirst(), Qt::SkipEmptyParts);

                if ((parts.size() == 3) || (parts.size() == 2))
                {
                    foundParts = true;
                    break;
                }
            }

            if (foundParts)
            {
                double ptLongitude = 0.0;
                double ptAltitude  = 0.0;
                bool haveAltitude  = false;
                bool okay          = true;
                double ptLatitude  = locale.toDouble(parts[0], &okay);

                if (okay)
                {
                    ptLongitude = locale.toDouble(parts[1], &okay);
                }

                if (okay && (parts.size() == 3))
                {
                    ptAltitude   = locale.toDouble(parts[2], &okay);
                    haveAltitude = okay;
                }

                foundData = okay;

                if (okay)
                {
                    if (swap)
                    {
                        std::swap(ptLatitude, ptLongitude);
                    }

                    GeoCoordinates coordinates(ptLatitude, ptLongitude);

                    if (haveAltitude)
                    {
                        coordinates.setAlt(ptAltitude);
                    }

                    gpsData.setCoordinates(coordinates);
                }
            }
        }
    }

    if (!foundData)
    {
        QMessageBox::information(d->imagesList,
                                 i18nc("@title:window", "Geolocation Editor"),
                                 i18n("Could not find any coordinates on the clipboard."));
        return;
    }

    setGPSDataForSelectedItems(gpsData, i18n("Coordinates pasted"));
}

void GPSItemListContextMenu::setGPSDataForSelectedItems(const GPSDataContainer& gpsData,
                                                        const QString& undoDescription)
{
    GPSItemModel* const imageModel            = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
    const int nSelected                       = selectedIndices.size();
    GPSUndoCommand* const undoCommand         = new GPSUndoCommand();

    for (int i = 0 ; i < nSelected ; ++i)
    {
        const QModelIndex itemIndex     = selectedIndices.at(i);
        GPSItemContainer* const gpsItem = imageModel->itemFromIndex(itemIndex);

        GPSUndoCommand::UndoInfo undoInfo(itemIndex);
        undoInfo.readOldDataFromItem(gpsItem);

        gpsItem->setGPSData(gpsData);
        undoInfo.readNewDataFromItem(gpsItem);

        undoCommand->addUndoInfo(undoInfo);
    }

    undoCommand->setText(undoDescription);

    Q_EMIT signalUndoCommand(undoCommand);
}

void GPSItemListContextMenu::slotBookmarkSelected(const GPSDataContainer& position)
{
    setGPSDataForSelectedItems(position, i18n("Bookmark selected"));
}

bool GPSItemListContextMenu::getCurrentPosition(GPSDataContainer* position, void* mydata)
{
    if (!position || !mydata)
    {
        return false;
    }

    GPSItemListContextMenu* const me = reinterpret_cast<GPSItemListContextMenu*>(mydata);

    return me->getCurrentItemPositionAndUrl(position, nullptr);
}

void GPSItemListContextMenu::removeInformationFromSelectedImages(const GPSDataContainer::HasFlags flagsToClear, const QString& undoDescription)
{
    // Enable or disable the actions.

    GPSItemModel* const imageModel            = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
    const int nSelected                       = selectedIndices.size();
    GPSUndoCommand* const undoCommand         = new GPSUndoCommand();

    for (int i = 0 ; i < nSelected ; ++i)
    {
        const QModelIndex itemIndex     = selectedIndices.at(i);
        GPSItemContainer* const gpsItem = imageModel->itemFromIndex(itemIndex);

        GPSUndoCommand::UndoInfo undoInfo(itemIndex);
        undoInfo.readOldDataFromItem(gpsItem);

        GPSDataContainer newGPSData     = gpsItem->gpsData();
        bool didSomething               = false;

        if (flagsToClear.testFlag(GPSDataContainer::HasCoordinates))
        {
            if (newGPSData.hasCoordinates())
            {
                didSomething = true;
                newGPSData.clear();
            }
        }

        if (flagsToClear.testFlag(GPSDataContainer::HasAltitude))
        {
            if (newGPSData.hasAltitude())
            {
                didSomething = true;
                newGPSData.clearAltitude();
            }
        }

        if (flagsToClear.testFlag(GPSDataContainer::HasNSatellites))
        {
            if (newGPSData.hasNSatellites())
            {
                didSomething = true;
                newGPSData.clearNSatellites();
            }
        }

        if (flagsToClear.testFlag(GPSDataContainer::HasDop))
        {
            if (newGPSData.hasDop())
            {
                didSomething = true;
                newGPSData.clearDop();
            }
        }

        if (flagsToClear.testFlag(GPSDataContainer::HasFixType))
        {
            if (newGPSData.hasFixType())
            {
                didSomething = true;
                newGPSData.clearFixType();
            }
        }

        if (flagsToClear.testFlag(GPSDataContainer::HasSpeed))
        {
            if (newGPSData.hasSpeed())
            {
                didSomething = true;
                newGPSData.clearSpeed();
            }
        }

        if (didSomething)
        {
            gpsItem->setGPSData(newGPSData);
            undoInfo.readNewDataFromItem(gpsItem);
            undoCommand->addUndoInfo(undoInfo);
        }
    }

    if (undoCommand->affectedItemCount() > 0)
    {
        undoCommand->setText(undoDescription);

        Q_EMIT signalUndoCommand(undoCommand);
    }
    else
    {
        delete undoCommand;
    }
}

void GPSItemListContextMenu::slotRemoveCoordinates()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasCoordinates, i18n("Remove coordinates information"));
}

void GPSItemListContextMenu::slotRemoveAltitude()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasAltitude, i18n("Remove altitude information"));
}

void GPSItemListContextMenu::slotRemoveUncertainty()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasNSatellites|GPSDataContainer::HasDop|GPSDataContainer::HasFixType,
                                        i18n("Remove uncertainty information"));
}

void GPSItemListContextMenu::setEnabled(const bool state)
{
    d->enabled = state;
}

void GPSItemListContextMenu::slotRemoveSpeed()
{
    removeInformationFromSelectedImages(GPSDataContainer::HasSpeed, i18n("Remove speed"));
}

void GPSItemListContextMenu::slotLookupMissingAltitudes()
{
    const GPSItemModel* const imageModel      = d->imagesList->getModel();
    QItemSelectionModel* const selectionModel = d->imagesList->getSelectionModel();
    const QList<QModelIndex> selectedIndices  = selectionModel->selectedRows();
/*
    const int nSelected                       = selectedIndices.size();
*/
    // Find the indices which have coordinates but no altitude.

    LookupAltitude::Request::List altitudeQueries;

    for (const QModelIndex& currentIndex : std::as_const(selectedIndices))
    {
        const GPSItemContainer* const gpsItem = imageModel->itemFromIndex(currentIndex);

        if (!gpsItem)
        {
            continue;
        }

        const GPSDataContainer gpsData   = gpsItem->gpsData();
        const GeoCoordinates coordinates = gpsData.getCoordinates();

        if ((!coordinates.hasCoordinates()) || coordinates.hasAltitude())
        {
            continue;
        }

        // The item has coordinates but no altitude, create a query.

        LookupAltitude::Request myLookup;
        myLookup.coordinates = coordinates;
        myLookup.data        = QVariant::fromValue(QPersistentModelIndex(currentIndex));

        altitudeQueries << myLookup;
    }

    if (altitudeQueries.isEmpty())
    {
        return;
    }

    d->altitudeLookup = LookupFactory::getAltitudeLookup(QLatin1String("geonames"), this);

    connect(d->altitudeLookup, SIGNAL(signalRequestsReady(QList<int>)),
            this, SLOT(slotAltitudeLookupReady(QList<int>)));

    connect(d->altitudeLookup, SIGNAL(signalDone()),
            this, SLOT(slotAltitudeLookupDone()));

    Q_EMIT signalSetUIEnabledWithCancel(false, this, QString::fromUtf8(SLOT(slotAltitudeLookupCancel())));

    Q_EMIT signalProgressSetup(altitudeQueries.count(), i18n("Looking up altitudes"));

    d->altitudeUndoCommand    = new GPSUndoCommand();
    d->altitudeRequestedCount = altitudeQueries.count();
    d->altitudeReceivedCount  = 0;
    d->altitudeLookup->addRequests(altitudeQueries);
    d->altitudeLookup->startLookup();
}

void GPSItemListContextMenu::slotAltitudeLookupReady(const QList<int>& readyRequests)
{
    GPSItemModel* const imageModel = d->imagesList->getModel();

    for (const int requestIndex : std::as_const(readyRequests))
    {
        const LookupAltitude::Request myLookup  = d->altitudeLookup->getRequest(requestIndex);
        const QPersistentModelIndex markerIndex = myLookup.data.value<QPersistentModelIndex>();

        if (!markerIndex.isValid())
        {
            continue;
        }

        GPSItemContainer* const gpsItem         = imageModel->itemFromIndex(markerIndex);

        if (!gpsItem)
        {
            continue;
        }

        GPSUndoCommand::UndoInfo undoInfo(markerIndex);
        undoInfo.readOldDataFromItem(gpsItem);

        GPSDataContainer gpsData                = gpsItem->gpsData();
        gpsData.setCoordinates(myLookup.coordinates);
        gpsItem->setGPSData(gpsData);
        undoInfo.readNewDataFromItem(gpsItem);

        d->altitudeUndoCommand->addUndoInfo(undoInfo);
        d->altitudeReceivedCount++;
    }

    Q_EMIT signalProgressChanged(d->altitudeReceivedCount);
}

void GPSItemListContextMenu::slotAltitudeLookupDone()
{
    LookupAltitude::StatusAltitude requestStatus = d->altitudeLookup->getStatus();

    if (requestStatus == LookupAltitude::StatusError)
    {
        const QString errorMessage = i18n("Altitude lookup failed:\n%1", d->altitudeLookup->errorMessage());
        QMessageBox::information(d->imagesList, i18nc("@title:window", "Geolocation Editor"), errorMessage);
    }

    if (d->altitudeReceivedCount > 0)
    {
        // At least some queries returned a result, save the undo command.

        d->altitudeUndoCommand->setText(i18n("Altitude looked up"));

        Q_EMIT signalUndoCommand(d->altitudeUndoCommand);
    }
    else
    {
        delete d->altitudeUndoCommand;
    }

    d->altitudeUndoCommand = nullptr;
    d->altitudeLookup->deleteLater();

    Q_EMIT signalSetUIEnabled(true);
}

void GPSItemListContextMenu::slotAltitudeLookupCancel()
{
    if (d->altitudeLookup)
    {
        d->altitudeLookup->cancel();
    }
}

} // namespace Digikam

#include "moc_gpsitemlistcontextmenu.cpp"
