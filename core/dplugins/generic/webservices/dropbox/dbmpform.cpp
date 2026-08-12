/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "dbmpform.h"

// Qt includes

#include <QFile>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericDropBoxPlugin
{

bool DBMPForm::addFile(const QString& imgPath)
{
    QFile file(imgPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    m_buffer = file.readAll();
    file.close();

    return true;
}

QByteArray DBMPForm::formData() const
{
    return m_buffer;
}

} // namespace DigikamGenericDropBoxPlugin
