/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide camera information to the parser
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

class CameraNameOption : public Option
{
    Q_OBJECT

public:

    CameraNameOption();
    ~CameraNameOption()                         override = default;

protected:

    QString parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match) override;

private:

    /// @note disabled
    CameraNameOption(QObject*)                           = delete;
    CameraNameOption(const CameraNameOption&)            = delete;
    CameraNameOption& operator=(const CameraNameOption&) = delete;
};

} // namespace Digikam
