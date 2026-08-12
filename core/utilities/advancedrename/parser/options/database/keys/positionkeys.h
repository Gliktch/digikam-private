/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-23
 * Description : position information keys
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dbkeyscollection.h"
#include "parsesettings.h"

namespace Digikam
{

class PositionKeys : public DbKeysCollection
{

public:

    PositionKeys();
    ~PositionKeys() override = default;

protected:

    QString getDbValue(const QString& key, ParseSettings& settings) override;

private:

    /// @note disabled
    PositionKeys(const PositionKeys&)            = delete;
    PositionKeys& operator=(const PositionKeys&) = delete;
};

} // namespace Digikam
