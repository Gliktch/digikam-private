/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-27
 * Description : A modifier for appending a suffix number to a renaming option.
 *               This guarantees a unique string for duplicate values.
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStringList>

// Local includes

#include "modifier.h"
#include "parseresults.h"

namespace Digikam
{

class UniqueModifier : public Modifier
{
    Q_OBJECT

public:

    UniqueModifier();
    QString  parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match) override;
    void     reset()                                                                       override;

private:

    QMap<ParseResults::ResultsKey, QStringList> cache;

private:

    /// @note disabled
    explicit UniqueModifier(QObject*)                = delete;
    UniqueModifier(const UniqueModifier&)            = delete;
    UniqueModifier& operator=(const UniqueModifier&) = delete;
};

} // namespace Digikam
