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

#include "option.h"

namespace Digikam
{

class Q_DECL_HIDDEN Option::Private
{
public:

    Private() = default;

    ParseResults parsedResults;
};

Option::Option(const QString& name, const QString& description)
    : Rule(name),
      d   (new Private)
{
    setDescription(description);
}

Option::Option(const QString& name, const QString& description, const QString& icon)
    : Rule(name, icon),
      d   (new Private)
{
    setDescription(description);
}

Option::~Option()
{
    delete d;
}

} // namespace Digikam

#include "moc_option.cpp"
