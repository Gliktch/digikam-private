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

// Qt includes

#include <QByteArray>
#include <QSize>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

enum class PrivacyStillProxyPresentation
{
    Generic = 1,
    Blurred = 2
};

enum class PrivacyStillProxyOutcome
{
    Failed                   = 0,
    GeneratedSameFormat     = 1,
    GeneratedGenericFallback = 2
};

enum class PrivacyStillProxyFallbackReason
{
    None                         = 0,
    UnsupportedPublicFormat      = 1,
    SameFormatEncoderUnavailable = 2,
    SourceFormatMismatch         = 3,
    SourceDecodeFailed           = 4,
    SourceSafetyLimitExceeded    = 5
};

enum class PrivacyStillProxyError
{
    None                   = 0,
    InvalidRequest         = 1,
    GenericEncoderMissing  = 2,
    EncodeFailed           = 3,
    EncodedOutputInvalid   = 4
};

class DIGIKAM_DATABASE_EXPORT PrivacyStillProxyRequest
{
public:

    QString                       sourcePath;
    QString                       publicFileName;
    PrivacyStillProxyPresentation presentation = PrivacyStillProxyPresentation::Generic;
};

class DIGIKAM_DATABASE_EXPORT PrivacyStillProxyResult
{
public:

    bool isValid() const;

public:

    PrivacyStillProxyOutcome        outcome = PrivacyStillProxyOutcome::Failed;
    PrivacyStillProxyFallbackReason fallbackReason = PrivacyStillProxyFallbackReason::None;
    PrivacyStillProxyError          error = PrivacyStillProxyError::InvalidRequest;
    PrivacyStillProxyPresentation   renderedPresentation = PrivacyStillProxyPresentation::Generic;
    QByteArray                      encodedBytes;
    QByteArray                      encodedFormat;
    QByteArray                      sha256;
    QSize                           pixelSize;
    bool                            sourcePixelsUsed = false;
};

/**
 * Builds metadata-free still-image proxy bytes. Privacy badges and borders are
 * UI adornments and are deliberately never rasterized by this class.
 *
 * The caller owns staging, journalling, durable publication and cache
 * transition. This class never writes to a public collection path.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyStillProxyGenerator
{
public:

    static QSize fixedPixelSize();
    static QByteArray canonicalFormatForFileName(const QString& fileName);
    static bool isSameFormatCandidate(const QString& fileName);

    PrivacyStillProxyResult generate(const PrivacyStillProxyRequest& request) const;
};

} // namespace Digikam
