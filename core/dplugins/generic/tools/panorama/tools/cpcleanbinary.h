/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetects cpclean binary program and version
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class CPCleanBinary : public DBinaryIface
{
    Q_OBJECT

public:

    explicit CPCleanBinary(QObject* const parent = nullptr);
    ~CPCleanBinary() override = default;
};

} // namespace DigikamGenericPanoramaPlugin
