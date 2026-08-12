/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-10-26
 * Description : a modifier for recognize and format a date
 *
 * SPDX-FileCopyrightText: 2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "modifier.h"

namespace Digikam
{

class DateFormatModifier : public Modifier
{
    Q_OBJECT

public:

    DateFormatModifier();

    QString parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match) override;

private:

    /// @note disabled
    explicit DateFormatModifier(QObject*)                    = delete;
    DateFormatModifier(const DateFormatModifier&)            = delete;
    DateFormatModifier& operator=(const DateFormatModifier&) = delete;
};

} // namespace Digikam
