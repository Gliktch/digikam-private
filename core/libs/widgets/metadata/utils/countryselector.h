/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-07
 * Description : country selector combo-box.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QWidget>
#include <QComboBox>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT CountrySelector : public QComboBox
{
    Q_OBJECT

public:

    explicit CountrySelector(QWidget* const parent);
    ~CountrySelector() override;

    void setCountry(const QString& countryCode);
    bool country(QString& countryCode, QString& countryName) const;

public:

    static QString countryForCode(const QString& countryCode);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
