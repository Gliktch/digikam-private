/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Writer of focus points to exiftool data
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QVariant>
#include <QStringList>

// Local includes

#include "digikam_export.h"
#include "focuspoint.h"

namespace Digikam
{

class DIGIKAM_EXPORT FocusPointsWriter : public QObject
{
    Q_OBJECT

public:

    explicit FocusPointsWriter(QObject* const parent, const QString& path);
    ~FocusPointsWriter();

public:

    void writeFocusPoint(const FocusPoint& point);
    void writeFocusPoint(const QRectF& rectF);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
