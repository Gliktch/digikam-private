/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect jAlbum jar archive
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

class JalbumJar : public DBinaryIface
{
    Q_OBJECT

public:

    explicit JalbumJar(QObject* const parent = nullptr);
    ~JalbumJar() override = default;

    bool checkDirForPath(const QString& possibleDir) override;
};

} // namespace DigikamGenericJAlbumPlugin
