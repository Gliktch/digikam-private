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

#pragma once

// Local includes

#include "ParsingRunner.h"

namespace Marble
{

class GpxRunner : public ParsingRunner
{
    Q_OBJECT

public:

    explicit GpxRunner(QObject* parent = nullptr);
    ~GpxRunner() override;

    GeoDataDocument* parseFile(const QString& fileName, DocumentRole role, QString& error) override;
};

} // namespace Marble
