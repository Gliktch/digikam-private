/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : auto-detect ExifTool program and version.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dbinaryiface.h"

using namespace Digikam;

namespace Digikam
{

class DIGIKAM_EXPORT ExifToolBinary : public DBinaryIface
{
    Q_OBJECT

public:

    explicit ExifToolBinary(QObject* const parent = nullptr);
    ~ExifToolBinary() override = default;
};

} // namespace Digikam
