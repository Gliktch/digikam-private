/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacypassword.h"

// C++ includes

#include <utility>

namespace Digikam
{

namespace
{

constexpr int MaximumPasswordBytes = 1024;

void overwriteByteArray(QByteArray& bytes)
{
    bytes.detach();

    volatile char* const data = bytes.data();

    for (qsizetype i = 0 ; i < bytes.size() ; ++i)
    {
        data[i] = 0;
    }

    bytes.clear();
    bytes.squeeze();
}

class ByteArrayWiper
{
public:

    explicit ByteArrayWiper(QByteArray& bytes)
        : m_bytes(bytes)
    {
    }

    ~ByteArrayWiper()
    {
        overwriteByteArray(m_bytes);
    }

private:

    QByteArray& m_bytes;
};

void setError(PrivacyPasswordError* const error, PrivacyPasswordError value)
{
    if (error)
    {
        *error = value;
    }
}

} // namespace

PrivacyPassword PrivacyPassword::fromUnicode(const QString& password,
                                             PrivacyPasswordError* const error)
{
    setError(error, PrivacyPasswordError::None);

    if (password.isEmpty())
    {
        setError(error, PrivacyPasswordError::Empty);

        return PrivacyPassword();
    }

    if (password.contains(QChar::Null))
    {
        setError(error, PrivacyPasswordError::ContainsNul);

        return PrivacyPassword();
    }

    if (password.contains(QLatin1Char('\r')))
    {
        setError(error, PrivacyPasswordError::ContainsCarriageReturn);

        return PrivacyPassword();
    }

    if (password.contains(QLatin1Char('\n')))
    {
        setError(error, PrivacyPasswordError::ContainsLineFeed);

        return PrivacyPassword();
    }

    QString normalized = password.normalized(QString::NormalizationForm_C);
    QByteArray utf8     = normalized.toUtf8();
    normalized.fill(QChar::Null);
    normalized.clear();

    if (utf8.isEmpty())
    {
        overwriteByteArray(utf8);
        setError(error, PrivacyPasswordError::Empty);

        return PrivacyPassword();
    }

    if (utf8.size() > MaximumPasswordBytes)
    {
        overwriteByteArray(utf8);
        setError(error, PrivacyPasswordError::TooLong);

        return PrivacyPassword();
    }

    return PrivacyPassword(std::move(utf8));
}

QString PrivacyPassword::encodingVersion()
{
    return QLatin1String("utf8-nfc-v1");
}

PrivacyPassword::PrivacyPassword(QByteArray&& utf8Bytes)
    : m_utf8Bytes(std::move(utf8Bytes))
{
}

PrivacyPassword::PrivacyPassword(PrivacyPassword&& other) noexcept
    : m_utf8Bytes(std::move(other.m_utf8Bytes))
{
    other.m_utf8Bytes.clear();
}

PrivacyPassword& PrivacyPassword::operator=(PrivacyPassword&& other) noexcept
{
    if (this != &other)
    {
        clear();
        m_utf8Bytes = std::move(other.m_utf8Bytes);
        other.m_utf8Bytes.clear();
    }

    return *this;
}

PrivacyPassword::~PrivacyPassword()
{
    clear();
}

bool PrivacyPassword::isValid() const
{
    return !m_utf8Bytes.isEmpty();
}

int PrivacyPassword::byteCount() const
{
    return static_cast<int>(m_utf8Bytes.size());
}

bool PrivacyPassword::withStdinLine(const std::function<bool(const QByteArray&)>& consumer) const
{
    if (!isValid() || !consumer)
    {
        return false;
    }

    QByteArray line = m_utf8Bytes;
    line.detach();
    line.append('\n');
    const ByteArrayWiper wiper(line);

    return consumer(line);
}

bool PrivacyPassword::withUtf8CString(
    const std::function<bool(const char*)>& consumer) const
{
    if (!isValid() || !consumer)
    {
        return false;
    }

    QByteArray nulTerminated = m_utf8Bytes;
    nulTerminated.detach();
    nulTerminated.append('\0');
    const ByteArrayWiper wiper(nulTerminated);

    return consumer(nulTerminated.constData());
}

void PrivacyPassword::clear()
{
    overwriteByteArray(m_utf8Bytes);
}

} // namespace Digikam
