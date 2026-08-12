/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : base autotags classifier. Thin wrapper to allow for easy replacement in the future
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "mlclassifierfoundation.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT AutotagsClassifierBase : public MLClassifierFoundation
{
    Q_OBJECT

public:

    explicit AutotagsClassifierBase(float _threshold, const QString& _predefinedClassFile);
    ~AutotagsClassifierBase()                                           override = default;

    QList<int> predictMulti(const QList<cv::Mat>& targets)        const override
    {
        Q_UNUSED(targets);

        return QList<int>();
    }

    QList<int> predictMulti(const QList<cv::UMat>& targets)       const override
    {
        Q_UNUSED(targets);

        return QList<int>();
    }

    bool retrain()                                                      override
    {
        return false;
    }

    QList<QString> getClassStrings(const QList<int>& labelList)   const;

protected:

    QString         predefinedClassFile;
    QList<QString>  predefinedClasses;

protected:

    bool loadTrainingData()                                             override;

private:

    /// @note disabled
    explicit AutotagsClassifierBase(QObject*)                                    = delete;
    AutotagsClassifierBase()                                                     = delete;
    AutotagsClassifierBase(const AutotagsClassifierBase&)                        = delete;
};

} // namespace Digikam
