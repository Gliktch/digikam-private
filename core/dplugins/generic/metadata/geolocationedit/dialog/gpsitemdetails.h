/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-06
 * Description : A widget to show details about item
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QModelIndex>
#include <QWidget>

// Local includes

#include "gpsdatacontainer.h"
#include "gpsundocommand.h"
#include "gpsitemmodel.h"

using namespace Digikam;

class KConfigGroup;

namespace DigikamGenericGeolocationEditPlugin
{

class GPSItemDetails : public QWidget
{
    Q_OBJECT

public:

    explicit GPSItemDetails(QWidget* const parent,
                            GPSItemModel* const imageModel);
    ~GPSItemDetails() override;

    void setUIEnabledExternal(const bool state);
    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

public Q_SLOTS:

    void slotSetCurrentImage(const QModelIndex& index);
    void slotSetActive(const bool state);

private Q_SLOTS:

    void updateUIState();
    void slotModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotApply();

protected:

    void displayGPSDataContainer(const GPSDataContainer* const gpsData);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericGeolocationEditPlugin
