/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : A label with an active url
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QLabel>
#include <QUrl>
#include <QString>
#include <QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * A widget to host an image into a label with an active url which can be
 * open to default web browser using simple mouse click.
 */
class DIGIKAM_EXPORT DActiveLabel : public QLabel
{
    Q_OBJECT

public:

    explicit DActiveLabel(const QUrl& url = QUrl(),
                          const QString& imgPath = QString(),
                          QWidget* const parent = nullptr);
    ~DActiveLabel() override = default;

    void updateData(const QUrl& url, const QImage& img);
};

} // namespace Digikam
