/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-23
 * Description : setup image editor output files settings.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT SetupIOFiles : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupIOFiles(QWidget* const parent = nullptr);
    ~SetupIOFiles() override;

    void applySettings();

private:

    void readSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
