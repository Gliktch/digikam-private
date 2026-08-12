/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-11-02
 * Description : Autodetect hugin_executor binary program and version
 *
 * SPDX-FileCopyrightText: 2015-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

class HuginExecutorBinary : public DBinaryIface
{
    Q_OBJECT

public:

    explicit HuginExecutorBinary(QObject* const parent = nullptr);
    ~HuginExecutorBinary() override = default;
};

} // namespace DigikamGenericPanoramaPlugin
