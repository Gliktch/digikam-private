/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2016-06-20
 * Description : Autodetects Mysql initializer binary program and version
 *
 * SPDX-FileCopyrightText: 2016-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dbinaryiface.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT MysqlInitBinary : public DBinaryIface
{
    Q_OBJECT

public:

    MysqlInitBinary();
    ~MysqlInitBinary() override = default;

private:

    /// @note disabled
    explicit MysqlInitBinary(QObject*) = delete;
};

} // namespace Digikam
