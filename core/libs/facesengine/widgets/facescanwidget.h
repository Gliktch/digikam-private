/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : Widget to choose options for face scanning
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTabWidget>

// Local includes

#include "statesavingobject.h"
#include "facescansettings.h"

namespace Digikam
{

class FaceScanWidget : public QTabWidget,
                       public StateSavingObject
{
    Q_OBJECT

public:

    explicit FaceScanWidget(QWidget* const parent = nullptr);
    ~FaceScanWidget()                 override;

    bool settingsConflicted()   const;
    FaceScanSettings settings() const;

protected:

    void doLoadState()                override;
    void doSaveState()                override;

private:

    void setupUi();
    void setupConnections();

private Q_SLOTS:

    void slotSettingsChanged();
    void slotPrepareForDetect(bool status);
    void slotPrepareForRecognize(bool status);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
