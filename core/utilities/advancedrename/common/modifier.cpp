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

#include "modifier.h"

namespace Digikam
{

class Q_DECL_HIDDEN Modifier::Private
{
public:

    Private() = default;

    ParseResults parsedResults;
};

Modifier::Modifier(const QString& name, const QString& description)
    : Rule(name),
      d   (new Private)
{
    setDescription(description);
}

Modifier::Modifier(const QString& name, const QString& description, const QString& icon)
    : Rule(name, icon),
      d   (new Private)
{
    setDescription(description);
}

Modifier::~Modifier()
{
    delete d;
}

} // namespace Digikam

#include "moc_modifier.cpp"
