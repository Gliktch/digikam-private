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

#include <QByteArray>
#include <QString>

// Local includes

#include "wstoolutils.h"

using namespace Digikam;

namespace DigikamGenericImageShackPlugin
{

class ImageShackMPForm
{

public:

    ImageShackMPForm();
    ~ImageShackMPForm() = default;

    void finish();
    void reset();

    void addPair(const QString& name, const QString& value);
    bool addFile(const QString& name, const QString& path);

    QString    contentType() const;
    QByteArray formData()    const;
    QString    boundary()    const;

private:

    QByteArray m_buffer;
    QByteArray m_boundary = WSToolUtils::randomString(42 + 13).toLatin1();

};

} // namespace DigikamGenericImageShackPlugin
