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

#pragma once

// Qt includes

#include <QByteArray>

namespace DigikamGenericDropBoxPlugin
{

class DBMPForm
{

public:

    DBMPForm()  = default;
    ~DBMPForm() = default;

    bool addFile(const QString& imgPath);
    QByteArray formData() const;

private:

    QByteArray m_buffer;
};

} // namespace DigikamGenericDropBoxPlugin
