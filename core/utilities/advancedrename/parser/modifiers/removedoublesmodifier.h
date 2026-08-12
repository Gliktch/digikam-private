/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : a modifier for deleting duplicate words
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "modifier.h"
#include "ruledialog.h"

namespace Digikam
{

class RemoveDoublesModifier : public Modifier
{
    Q_OBJECT

public:

    RemoveDoublesModifier();
    QString parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match) override;

private:

    /// @note disabled
    explicit RemoveDoublesModifier(QObject*)                       = delete;
    RemoveDoublesModifier(const RemoveDoublesModifier&)            = delete;
    RemoveDoublesModifier& operator=(const RemoveDoublesModifier&) = delete;
};

} // namespace Digikam
