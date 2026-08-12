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

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Marble
{

class GeoDataLatLonBox;

class DIGIKAM_EXPORT LatLonBoxWidget: public QWidget
{
    Q_OBJECT

public:

    explicit LatLonBoxWidget(QWidget* const parent = nullptr, Qt::WindowFlags const f = Qt::WindowFlags());
    ~LatLonBoxWidget() override;
    GeoDataLatLonBox latLonBox() const;
    void setLatLonBox(GeoDataLatLonBox const&);

Q_SIGNALS:

    void valueChanged();

private Q_SLOTS:

    void updateLatSingleStep();
    void updateLonSingleStep();

private:

    Q_DISABLE_COPY(LatLonBoxWidget)

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Marble
