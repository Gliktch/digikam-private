/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect gimp binary program
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericPrintCreatorPlugin
{

class GimpBinary : public DBinaryIface
{
    Q_OBJECT

public:

    explicit GimpBinary(QObject* const parent = nullptr);
    ~GimpBinary() override = default;
};

} // namespace DigikamGenericPrintCreatorPlugin
