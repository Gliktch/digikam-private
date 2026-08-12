/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-02
 * Description : an option to provide directory information to the parser
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "option.h"

class QString;

namespace Digikam
{

class DirectoryNameOption : public Option
{
    Q_OBJECT

public:

    DirectoryNameOption();
    ~DirectoryNameOption()                                       override = default;

protected:

    QString parseOperation(ParseSettings& settings,
                           const QRegularExpressionMatch& match) override;

private:

    /// @note disabled
    DirectoryNameOption(QObject*)                              = delete;
    DirectoryNameOption(const DirectoryNameOption&)            = delete;
    DirectoryNameOption& operator=(const DirectoryNameOption&) = delete;
};

} // namespace Digikam
