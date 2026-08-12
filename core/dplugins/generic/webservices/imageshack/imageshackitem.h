/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
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

class ImageShackGallery
{

public:

    ImageShackGallery() = default;

    QString m_user;
    QString m_server;
    QString m_name;
    QString m_title;
    QString m_url;
};

// -------------------------------------------------------------------

class ImageShackPhoto
{

public:

    ImageShackPhoto() = default;

    QString m_server;
    QString m_name;
    QString m_bucket;
};

} // namespace DigikamGenericImageShackPlugin
