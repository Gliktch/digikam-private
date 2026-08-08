/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C++ includes

#include <functional>

// Qt includes

#include <QByteArray>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

enum class PrivacyPasswordError
{
    None,
    Empty,
    ContainsNul,
    ContainsCarriageReturn,
    ContainsLineFeed,
    TooLong
};

class DIGIKAM_DATABASE_EXPORT PrivacyPassword
{
public:

    static PrivacyPassword fromUnicode(const QString& password,
                                       PrivacyPasswordError* error = nullptr);
    static QString encodingVersion();

    PrivacyPassword(PrivacyPassword&& other) noexcept;
    PrivacyPassword& operator=(PrivacyPassword&& other) noexcept;
    ~PrivacyPassword();

    PrivacyPassword(const PrivacyPassword&)            = delete;
    PrivacyPassword& operator=(const PrivacyPassword&) = delete;

    bool isValid() const;
    int byteCount() const;

    /**
     * Supplies one temporary UTF-8 password line to a dedicated secret sink.
     * The temporary buffer is overwritten immediately after the callback.
     * Callers must not log, persist, stringify, or retain the reference.
     */
    bool withStdinLine(const std::function<bool(const QByteArray&)>& consumer) const;

    /**
     * Supplies a temporary NUL-terminated UTF-8 password to an in-process C
     * API. The pointer is overwritten and invalid as soon as the callback
     * returns. Callers must not retain, log, stringify, or persist it.
     */
    bool withUtf8CString(const std::function<bool(const char*)>& consumer) const;

private:

    PrivacyPassword() = default;
    explicit PrivacyPassword(QByteArray&& utf8Bytes);

    void clear();

private:

    QByteArray m_utf8Bytes;
};

} // namespace Digikam
