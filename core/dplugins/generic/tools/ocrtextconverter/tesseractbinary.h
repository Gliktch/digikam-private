/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect Tesseract binary program
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStringList>

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class TesseractBinary : public DBinaryIface
{
    Q_OBJECT

public:

    explicit TesseractBinary(QObject* const parent = nullptr);
    ~TesseractBinary()                     override = default;

    QStringList tesseractLanguages() const;
};

} // namespace DigikamGenericTextConverterPlugin
