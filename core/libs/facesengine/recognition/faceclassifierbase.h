/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Base face classifier. Thin wrapper to allow for easy replacement in the future.
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "mlclassifierfoundation.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT FaceClassifierBase : public MLClassifierFoundation
{
    Q_OBJECT

public:

    FaceClassifierBase();
    ~FaceClassifierBase()                                             override = default;

    QList<int> predictMulti(const QList<cv::Mat>& targets)      const override;
    QList<int> predictMulti(const QList<cv::UMat>& targets)     const override;

public:

    static const int UNKNOWN_LABEL_ID; // Make it static const

private:

    /// @note disabled
    explicit FaceClassifierBase(QObject*)                                      = delete;
    FaceClassifierBase(const FaceClassifierBase&)                              = delete;
};

} // namespace Digikam
