/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

namespace DigikamGenericGoogleServicesPlugin
{

class GPMPForm
{

public:

    GPMPForm()  = default;
    ~GPMPForm() = default;

    void finish();
    void reset();

    bool addPair(const QString& name,
                 const QString& value,
                 const QString& contentType = QString());

    bool addFile(const QString& name,
                 const QString& path);

    QString    contentType() const;
    QByteArray formData()    const;
    QString    boundary()    const;

private:

    QByteArray m_buffer;
    QByteArray m_boundary = QByteArray("----------") +
                            WSToolUtils::randomString(42 + 13).toLatin1();
};

} // namespace DigikamGenericGoogleServicesPlugin
