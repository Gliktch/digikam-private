/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "ParseRunnerPlugin.h"

// Qt includes

#include <QIcon>

namespace Marble
{

class Q_DECL_HIDDEN ParseRunnerPlugin::Private
{
public:

    Private();
};

ParseRunnerPlugin::Private::Private()
{
    // nothing to do
}

ParseRunnerPlugin::ParseRunnerPlugin(QObject* parent) :
    QObject(parent),
    d(new Private)
{
}

ParseRunnerPlugin::~ParseRunnerPlugin()
{
    delete d;
}

QIcon ParseRunnerPlugin::icon() const
{
    return QIcon();
}

} // namespace Marble

#include "moc_ParseRunnerPlugin.cpp"
