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

#pragma once

// Qt includes:

#include <QObject>

// Local includes

#include "digikam_config.h"
#include "lookupaltitude.h"
#include "gpsdatacontainer.h"
#include "gpsitemlist.h"
#include "digikam_export.h"

class QUrl;

namespace Digikam
{

class GPSUndoCommand;
class GPSBookmarkOwner;

class DIGIKAM_EXPORT GPSItemListContextMenu : public QObject
{
    Q_OBJECT

public:

    explicit GPSItemListContextMenu(GPSItemList* const imagesList,
                                    GPSBookmarkOwner* const bookmarkOwner = nullptr);
    ~GPSItemListContextMenu()                           override;

    void setEnabled(const bool state);

protected:

    void setGPSDataForSelectedItems(const GPSDataContainer& gpsData, const QString& undoDescription);
    bool getCurrentItemPositionAndUrl(GPSDataContainer* const gpsInfo, QUrl* const itemUrl);
    void removeInformationFromSelectedImages(const GPSDataContainer::HasFlags flagsToClear, const QString& undoDescription);

    bool eventFilter(QObject* watched, QEvent* event)   override;

    static bool getCurrentPosition(GPSDataContainer* position, void* mydata);

private Q_SLOTS:

    void copyActionTriggered();
    void pasteSwapActionTriggered();
    void pasteActionTriggered(bool swap = false);
    void slotBookmarkSelected(const GPSDataContainer& position);
    void slotRemoveCoordinates();
    void slotRemoveAltitude();
    void slotRemoveUncertainty();
    void slotRemoveSpeed();
    void slotLookupMissingAltitudes();
    void slotAltitudeLookupReady(const QList<int>& readyRequests);
    void slotAltitudeLookupDone();
    void slotAltitudeLookupCancel();

Q_SIGNALS:

    void signalSetUIEnabled(const bool enabledState);
    void signalSetUIEnabledWithCancel(const bool enabledState, QObject* const cancelObject, const QString& cancelSlot);
    void signalProgressSetup(const int maxProgress, const QString& progressText);
    void signalProgressChanged(const int currentProgress);
    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
