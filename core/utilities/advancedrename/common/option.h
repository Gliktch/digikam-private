/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an abstract option class
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "rule.h"

namespace Digikam
{

class Option : public Rule
{
    Q_OBJECT

public:

    Option(const QString& name, const QString& description);
    Option(const QString& name, const QString& description, const QString& icon);
    ~Option()                                                    override;

protected:

    QString parseOperation(ParseSettings& settings,
                           const QRegularExpressionMatch& match) override = 0;

private:

    /// @note disabled
    Option(QObject*)                 = delete;
    Option(const Option&)            = delete;
    Option& operator=(const Option&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
