/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : A widget to apply Reverse Geocoding
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QWidget>

// Local includes

#include "geoifacetypes.h"

// Local includes

#include "gpsitemcontainer.h"
#include "digikam_export.h"

class QItemSelectionModel;
class QAbstractItemModel;
class KConfigGroup;

namespace Digikam
{

class GPSItemModel;
class GPSUndoCommand;

class DIGIKAM_EXPORT RGWidget : public QWidget
{
    Q_OBJECT

public:

    explicit RGWidget(GPSItemModel* const imageModel,
                      QItemSelectionModel* const selectionModel,
                      QAbstractItemModel* externTagModel,
                      QWidget* const parent = nullptr);
    ~RGWidget()                                             override;

    void setUIEnabled(const bool state);
    void readSettingsFromGroup(const KConfigGroup* const group);
    void saveSettingsToGroup(KConfigGroup* const group);

private Q_SLOTS:

    void slotButtonRGSelected();
    void slotRGReady(const QList<RGInfo>& returnedRGList);
    void slotHideOptions();
    void updateUIState();
    void slotAddCustomizedSpacer();
    void slotRemoveTag();
    void slotRemoveAllSpacers();
    void slotReaddNewTags();
    void slotRegenerateNewTags();
    void slotAddAllAddressElementsToTag();
    void slotAddSingleSpacer();
    void slotRGCanceled();

Q_SIGNALS:

    /**
     * This signal emits when containing widgets need to be enabled or disabled.
     * @param enabledState If true, the containing widgets will be enabled. Else, they will be disabled.
     */
    void signalSetUIEnabled(const bool enabledState);
    void signalSetUIEnabledWithCancel(const bool enabledState,
                                      QObject* const cancelObject,
                                      const QString& cancelSlot);

    /**
     * Update the progress bar.
     */
    void signalProgressSetup(const int maxProgress, const QString& progressText);

    /**
     * Counts how many images were processed.
     * @param currentProgress The number of processed images.
     */
    void signalProgressChanged(const int currentProgress);

    /**
     * Sends the needed data to Undo/Redo Widget.
     * @param undoCommand Holds the data that will be used for undo or redo actions
     */
    void signalUndoCommand(GPSUndoCommand* undoCommand);

protected:

    bool eventFilter(QObject* watched, QEvent* event)       override;

private:

    void createCountryCodeMap();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
