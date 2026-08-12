/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QByteArray>
#include <QString>

// Local insludes

#include "wstoolutils.h"

using namespace Digikam;

namespace DigikamGenericGoogleServicesPlugin
{

class GDMPForm
{
public:

    GDMPForm();
    ~GDMPForm() = default;

    void finish();
    void reset();

    void addPair(const QString& name,
                 const QString& description,
                 const QString& mimetype,
                 const QString& id);

    bool addFile(const QString& path);

    QString contentType() const;
    QByteArray formData() const;
    QString boundary()    const;
    QString getFileSize() const;

private:

    QByteArray m_buffer;
    QByteArray m_boundary = WSToolUtils::randomString(42 + 13).toLatin1();

    QString    m_file_size;
};

} // namespace DigikamGenericGoogleServicesPlugin
