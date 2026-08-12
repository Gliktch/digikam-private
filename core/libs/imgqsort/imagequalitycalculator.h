/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Calculor
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_globals.h"

// Qt includes

#include <QString>
#include <QList>

namespace Digikam
{

class ImageQualityCalculator
{
public:

    struct ResultDetection
    {
        QString detetionType;
        float   weight = 0.0F;
        float   score  = 0.0F;
    };

public:

    explicit ImageQualityCalculator();
    ~ImageQualityCalculator();

    float calculateQuality()                    const;
    void addDetectionResult(const QString& name,
                            const float score,
                            const float weight) const;

private:

    void normalizeWeight()                      const;
    void adjustWeightByQualityLevel()           const;
    int  numberDetectors()                      const;

    /// @note disabled
    ImageQualityCalculator(const ImageQualityCalculator&)            = delete;
    ImageQualityCalculator& operator=(const ImageQualityCalculator&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
