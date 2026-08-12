/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Integrated, multithread object detection / recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "autotagsclassifierbase.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT AutotagsClassifierMinmax : public AutotagsClassifierBase
{
    Q_OBJECT

public:

    explicit AutotagsClassifierMinmax(float _threshold, const QString& predefinedClassFile);
    virtual ~AutotagsClassifierMinmax()                                               override = default;

    virtual int predict(const cv::Mat& target,
                        const QList<int>& exclusionLabelList = QList<int>())    const override;
    virtual int predict(const cv::UMat& target,
                        const QList<int>& exclusionLabelList = QList<int>())    const override;

    virtual QList<int> predictMulti(const QList<cv::Mat>& targets)              const override;
    virtual QList<int> predictMulti(const QList<cv::UMat>& targets)             const override;

private:

    /// @note disabled
    AutotagsClassifierMinmax()                                                                 = delete;
    AutotagsClassifierMinmax(const AutotagsClassifierMinmax&)                                  = delete;
    explicit AutotagsClassifierMinmax(QObject*)                                                = delete;
};

} // namespace Digikam
