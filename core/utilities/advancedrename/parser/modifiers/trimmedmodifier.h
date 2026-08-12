/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : trimmed token modifier
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "modifier.h"

namespace Digikam
{

class TrimmedModifier : public Modifier
{
    Q_OBJECT

public:

    TrimmedModifier();
    QString parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match) override;

private:

    /// @note disabled
    explicit TrimmedModifier(QObject*)                 = delete;
    TrimmedModifier(const TrimmedModifier&)            = delete;
    TrimmedModifier& operator=(const TrimmedModifier&) = delete;
};

} // namespace Digikam
