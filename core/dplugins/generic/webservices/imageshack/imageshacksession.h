/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShackSession web service
 *
 * SPDX-FileCopyrightText: 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2025 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

namespace DigikamGenericImageShackPlugin
{

class ImageShackSession
{

public:

    ImageShackSession();
    ~ImageShackSession();

public:

    bool    loggedIn()  const;
    QString username()  const;
    QString email()     const;
    QString password()  const;
    QString credits()   const;
    QString authToken() const;

    void setLoggedIn(bool b);
    void setUsername(const QString& username);
    void setEmail(const QString& email);
    void setPassword(const QString& pass);
    void setCredits(const QString& credits);
    void setAuthToken(const QString& token);

    void readSettings();
    void saveSettings();

    void logOut();

private:

    /// @note disabled
    ImageShackSession(const ImageShackSession&)            = delete;
    ImageShackSession& operator=(const ImageShackSession&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericImageShackPlugin
