/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

namespace DigikamGenericPiwigoPlugin
{

class PiwigoSession
{

public:

    PiwigoSession();
    ~PiwigoSession();

public:

    QString url()      const;
    QString username() const;
    QString password() const;

    void setUrl(const QString& url);
    void setUsername(const QString& username);
    void setPassword(const QString& password);

public:

    void save();

private:

    void load();

private:

    /// @note disabled
    PiwigoSession(const PiwigoSession&)            = delete;
    PiwigoSession& operator=(const PiwigoSession&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPiwigoPlugin
