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

#include "databaseservererror.h"

namespace Digikam
{

DatabaseServerError::DatabaseServerError(DatabaseServerErrorEnum errorType, const QString& errorText)
    : m_ErrorText(errorText),
      m_ErrorType(errorType)
{
}

DatabaseServerError::DatabaseServerError(const DatabaseServerError& dbServerError)
    : m_ErrorText(dbServerError.m_ErrorText),
      m_ErrorType(dbServerError.m_ErrorType)
{
}

DatabaseServerError::DatabaseServerErrorEnum DatabaseServerError::getErrorType() const
{
    return m_ErrorType;
}

void DatabaseServerError::setErrorType(DatabaseServerErrorEnum errorType)
{
    m_ErrorType = errorType;
}

QString DatabaseServerError::getErrorText() const
{
    return m_ErrorText;
}
void DatabaseServerError::setErrorText(const QString& errorText)
{
    m_ErrorText = errorText;
}

} // namespace Digikam
