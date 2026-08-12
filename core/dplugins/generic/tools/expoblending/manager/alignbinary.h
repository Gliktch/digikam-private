/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : Autodetect align_image_stack binary program and version
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{

class AlignBinary : public DBinaryIface
{
    Q_OBJECT

public:

    AlignBinary();
    ~AlignBinary() override = default;

private:

    /// @note disabled
    explicit AlignBinary(QObject*) = delete;
};

} // namespace DigikamGenericExpoBlendingPlugin
