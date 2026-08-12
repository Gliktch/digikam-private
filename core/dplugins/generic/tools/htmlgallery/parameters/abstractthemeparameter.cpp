/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "abstractthemeparameter.h"

// KDE includes

#include <kconfiggroup.h>

static const char* NAME_KEY          = "Name";
static const char* DEFAULT_VALUE_KEY = "Default";

namespace DigikamGenericHtmlGalleryPlugin
{

class Q_DECL_HIDDEN AbstractThemeParameter::Private
{
public:

    Private() = default;

    QByteArray internalName;
    QString    name;
    QString    defaultValue;
};

AbstractThemeParameter::AbstractThemeParameter()
    : d(new Private)
{
}

AbstractThemeParameter::~AbstractThemeParameter()
{
    delete d;
}

void AbstractThemeParameter::init(const QByteArray& internalName, const KConfigGroup* group)
{
    d->internalName = internalName;
    d->name         = group->readEntry(NAME_KEY);
    d->defaultValue = group->readEntry(DEFAULT_VALUE_KEY);
}

QByteArray AbstractThemeParameter::internalName() const
{
    return d->internalName;
}

QString AbstractThemeParameter::name() const
{
    return d->name;
}

QString AbstractThemeParameter::defaultValue() const
{
    return d->defaultValue;
}

} // namespace DigikamGenericHtmlGalleryPlugin
