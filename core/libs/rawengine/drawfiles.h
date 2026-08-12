/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-11-06
 * Description : list of RAW file extensions supported by libraw
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QMap>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

DIGIKAM_EXPORT QMap<QString, QString> s_rawFileExtensionsdWithDesc();

DIGIKAM_EXPORT QString                s_rawFileExtensions();

DIGIKAM_EXPORT int                    s_rawFileExtensionsVersion();

} // namespace Digikam
