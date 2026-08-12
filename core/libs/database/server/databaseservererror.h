/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-26
 * Description : database server error reporting
 *
 * SPDX-FileCopyrightText: 2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2016 by Swati Lodha <swatilodha27 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseServerError
{
public:

    enum DatabaseServerErrorEnum
    {
        /**
         * No errors occurred while starting the database server
         */
        NoErrors = 0,

        /**
         * The requested database type is not supported.
         */
        NotSupported,

        /**
         * A error has occurred while starting the database server executable.
         */
        StartError
    };

public:

    explicit DatabaseServerError(DatabaseServerErrorEnum errorType = NoErrors,
                                 const QString& errorText = QString());
    DatabaseServerError(const DatabaseServerError& dbServerError);
    ~DatabaseServerError() = default;

    DatabaseServerErrorEnum getErrorType() const;
    void                    setErrorType(DatabaseServerErrorEnum errorType);

    QString                 getErrorText() const;
    void                    setErrorText(const QString& errorText);

private:

    QString                 m_ErrorText;
    DatabaseServerErrorEnum m_ErrorType = NoErrors;
};

} // namespace Digikam
