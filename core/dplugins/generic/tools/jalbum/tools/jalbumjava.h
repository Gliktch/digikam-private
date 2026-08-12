/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect Java binary program
 *
 * SPDX-FileCopyrightText: 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericJAlbumPlugin
{

class JalbumJava : public DBinaryIface
{
    Q_OBJECT

public:

    explicit JalbumJava(QObject* const parent = nullptr);
    ~JalbumJava() override = default;
};

} // namespace DigikamGenericJAlbumPlugin
