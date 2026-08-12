/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a class to manipulate the results of an renaming options
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

class Modifier : public Rule
{
    Q_OBJECT

public:

    Modifier(const QString& name, const QString& description);
    Modifier(const QString& name, const QString& description, const QString& icon);
    ~Modifier()                                                  override;

protected:

    QString parseOperation(ParseSettings& settings,
                           const QRegularExpressionMatch& match) override = 0;

private:

    /// @note disabled
    Modifier(QObject*)                   = delete;
    Modifier(const Modifier&)            = delete;
    Modifier& operator=(const Modifier&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
