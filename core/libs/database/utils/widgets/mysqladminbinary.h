/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-05-30
 * Description : Autodetects Mysql server admin program and version
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

class DIGIKAM_GUI_EXPORT MysqlAdminBinary : public DBinaryIface
{
    Q_OBJECT

public:

    MysqlAdminBinary();
    ~MysqlAdminBinary() override = default;

private:

    /// @note disabledd
    explicit MysqlAdminBinary(QObject*) = delete;
};

} // namespace Digikam
