/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Foundation for all machine learning pipelines
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QIcon>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MLPipelinePackageFoundation
{
public:

    MLPipelinePackageFoundation()                                              = default;
    virtual ~MLPipelinePackageFoundation()                                     = default;

public:

    quint64 size            = 0;
    QIcon   thumbnailIcon;

private:

    /// @note disabled
    MLPipelinePackageFoundation(const MLPipelinePackageFoundation&)            = delete;
    MLPipelinePackageFoundation& operator=(const MLPipelinePackageFoundation&) = delete;
};

} // namespace Digikam
