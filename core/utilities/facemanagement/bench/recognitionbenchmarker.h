/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Face recognition benchmarker
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
#include "facialrecognition_wrapper.h"

namespace Digikam
{

class Q_DECL_HIDDEN RecognitionBenchmarker : public WorkerObject
{
    Q_OBJECT

public:

    explicit RecognitionBenchmarker(FacePipeline::Private* const dd);
    QString result() const;

public Q_SLOTS:

    void process(const FacePipelineExtendedPackage::Ptr& package);

Q_SIGNALS:

    void processed(const FacePipelineExtendedPackage::Ptr& package);

protected:

    class Q_DECL_HIDDEN Statistics
    {
    public:

        Statistics() = default;

    public:

        int knownFaces          = 0;
        int correctlyRecognized = 0;
    };

protected:

    QMap<int, Statistics>        results;
    FacialRecognitionWrapper     recognizer;

    FacePipeline::Private* const d = nullptr;
};

} // namespace Digikam
