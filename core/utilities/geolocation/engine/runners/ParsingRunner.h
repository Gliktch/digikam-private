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

// Qt includes

#include <QObject>

// Local includes

#include "GeoDataDocument.h"
#include "digikam_export.h"

namespace Marble
{

class DIGIKAM_EXPORT ParsingRunner : public QObject
{
    Q_OBJECT

public:

    explicit ParsingRunner(QObject* parent = nullptr);

    /**
      * Start a file parsing.
      * If implemented in a plugin, make sure to include Parsing in the
      * plugin capabilities, otherwise MarbleRunnerManager will ignore the plugin
      */
    virtual GeoDataDocument* parseFile(const QString& fileName, DocumentRole role, QString& error) = 0;
};

} // namespace Marble
