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

class DIGIKAM_GUI_EXPORT AutotagsClassifierSoftmax : public AutotagsClassifierBase
{
    Q_OBJECT

public:

    explicit AutotagsClassifierSoftmax(float _threshold, const QString& predefinedClassFile);
    virtual ~AutotagsClassifierSoftmax()                                            override    = default;

    virtual int predict(const cv::Mat& target,
                        const QList<int>& exclusionLabelList = QList<int>()) const  override;
    virtual int predict(const cv::UMat& target,
                        const QList<int>& exclusionLabelList = QList<int>()) const  override;

    virtual QList<int> predictMulti(const QList<cv::Mat>& targets)           const  override;
    virtual QList<int> predictMulti(const QList<cv::UMat>& targets)          const  override;

private:

    /// @note disabled
    AutotagsClassifierSoftmax()                                                                 = delete;
    AutotagsClassifierSoftmax(const AutotagsClassifierSoftmax&)                                 = delete;
    explicit AutotagsClassifierSoftmax(QObject*)                                                = delete;
};

} // namespace Digikam
