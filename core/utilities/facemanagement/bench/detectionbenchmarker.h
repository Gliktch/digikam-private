/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Face detection benchmarker
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "facepipeline_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN DetectionBenchmarker : public WorkerObject
{
    Q_OBJECT

public:

    explicit DetectionBenchmarker(FacePipeline::Private* const d);
    QString result() const;

public Q_SLOTS:

    void process(const FacePipelineExtendedPackage::Ptr& package);

Q_SIGNALS:

    void processed(const FacePipelineExtendedPackage::Ptr& package);

protected:

    int                          totalImages            = 0;
    int                          faces                  = 0;
    double                       totalPixels            = 0.0;
    double                       facePixels             = 0.0;

    int                          trueNegativeImages     = 0;
    int                          falsePositiveImages    = 0;

    int                          truePositiveFaces      = 0;
    int                          falseNegativeFaces     = 0;
    int                          falsePositiveFaces     = 0;

    FacePipeline::Private* const d                      = nullptr;
};

} // namespace Digikam
