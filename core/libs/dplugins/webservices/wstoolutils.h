/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : Web Service tool utils methods
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QSettings>
#include <QString>
#include <QDir>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT WSToolUtils
{
public:

    WSToolUtils()  = default;
    ~WSToolUtils() = default;

    static QString decodeKey(const QString& key);

    /**
     * Generates random string.
     */
    static QString randomString(const int& length);

    static QDir makeTemporaryDir(const char* prefix);
    static void removeTemporaryDir(const char* prefix);

    static QSettings* getOauthSettings(QObject* const parent);

    static void saveToken(const QString& name, const QString& token);

    static QString readToken(const QString& name);

    static void clearToken(const QString& name);

private:

    static QString oauthConfigFile();
    static QString possibleCharacters();
};

} // namespace Digikam
