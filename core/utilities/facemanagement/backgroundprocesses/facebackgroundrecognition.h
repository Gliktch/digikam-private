/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-03-30
 * Description : Face Background Recognition Controller
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT FaceRecognitionBackgroundController : public QObject
{
    Q_OBJECT

public:

    FaceRecognitionBackgroundController();
    ~FaceRecognitionBackgroundController() override;

    static FaceRecognitionBackgroundController* instance();

    void stop();
    bool waitForDone();

private Q_SLOTS:

    void slotSetEnabled(bool enabled);
    bool slotRescan();
    void slotScanDone();

private:

    /// @note disabled
    explicit FaceRecognitionBackgroundController(QObject*)                           = delete;
    FaceRecognitionBackgroundController(const FaceRecognitionBackgroundController&)  = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
