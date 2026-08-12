/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide file information to the parser
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QString>

// Local includes

#include "option.h"

namespace Digikam
{

class FilePropertiesOption : public Option
{
    Q_OBJECT

public:

    FilePropertiesOption();
    ~FilePropertiesOption()                                      override = default;

protected:

    QString parseOperation(ParseSettings& settings,
                           const QRegularExpressionMatch& match) override;

private:

    /// @note disabled
    FilePropertiesOption(QObject*)                               = delete;
    FilePropertiesOption(const FilePropertiesOption&)            = delete;
    FilePropertiesOption& operator=(const FilePropertiesOption&) = delete;
};

} // namespace Digikam
