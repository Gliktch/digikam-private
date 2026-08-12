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

// Local includes

#include "faceclassifierbase.h"

namespace Digikam
{

const int FaceClassifierBase::UNKNOWN_LABEL_ID = -1;

FaceClassifierBase::FaceClassifierBase()
    : MLClassifierFoundation()
{
}

QList<int> FaceClassifierBase::predictMulti(const QList<cv::Mat>& targets) const
{
    Q_UNUSED(targets);

    return QList<int>();
}

QList<int> FaceClassifierBase::predictMulti(const QList<cv::UMat>& targets) const
{
    Q_UNUSED(targets);

    return QList<int>();
}

} // namespace Digikam

#include "moc_faceclassifierbase.cpp"
