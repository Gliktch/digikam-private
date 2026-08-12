/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-05-13
 * Description : ExifTool thread for the ExifTool process
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QThread>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ExifToolThread : public QThread
{
    Q_OBJECT

public:

    explicit ExifToolThread(QObject* const parent);
    ~ExifToolThread() override = default;

Q_SIGNALS:

    void exifToolProcessStarted();

protected:

    /**
     * Main thread loop.
     */
    void run() override;
};

} // namespace Digikam
