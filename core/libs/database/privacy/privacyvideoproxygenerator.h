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

class PrivacyProcessRunner;

enum class PrivacyVideoProxyPresentation
{
    Generic = 1,
    Blurred = 2
};

enum class PrivacyVideoProxyOutcome
{
    Failed                   = 0,
    GeneratedSameContainer   = 1,
    GeneratedGenericFallback = 2
};

enum class PrivacyVideoProxyFallbackReason
{
    None                    = 0,
    SourceProbeFailed       = 1,
    SourceContainerMismatch = 2,
    NoUsableKeyframe        = 3,
    KeyframeEncodeFailed    = 4
};

enum class PrivacyVideoProxyError
{
    None                 = 0,
    InvalidRequest       = 1,
    UnsupportedContainer = 2,
    ToolUnavailable      = 3,
    EncodeFailed         = 4,
    EncodedOutputInvalid = 5
};

struct DIGIKAM_DATABASE_EXPORT PrivacyVideoToolPaths
{
    QString ffmpeg;
    QString ffprobe;

    bool isValid() const;
    static PrivacyVideoToolPaths discover();
};

class DIGIKAM_DATABASE_EXPORT PrivacyVideoProxyRequest
{
public:

    QString                       sourcePath;
    QString                       publicFileName;
    PrivacyVideoProxyPresentation presentation = PrivacyVideoProxyPresentation::Generic;
};

class DIGIKAM_DATABASE_EXPORT PrivacyVideoProxyResult
{
public:

    bool isValid() const;

public:

    PrivacyVideoProxyOutcome        outcome = PrivacyVideoProxyOutcome::Failed;
    PrivacyVideoProxyFallbackReason fallbackReason = PrivacyVideoProxyFallbackReason::None;
    PrivacyVideoProxyError          error = PrivacyVideoProxyError::InvalidRequest;
    PrivacyVideoProxyPresentation   renderedPresentation = PrivacyVideoProxyPresentation::Generic;
    QByteArray                      encodedBytes;
    QByteArray                      containerFormat;
    QByteArray                      sha256;
    QSize                           pixelSize;
    double                          selectedKeyframeSeconds = -1.0;
    bool                            sourcePixelsUsed = false;
};

/**
 * Builds a metadata-free, one-frame video proxy in the public filename's
 * container. Privacy badges and borders remain UI adornments.
 *
 * The caller owns staging, journalling, durable publication and cache
 * transition. This class never writes to a public collection path.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyVideoProxyGenerator
{
public:

    PrivacyVideoProxyGenerator(PrivacyProcessRunner& runner,
                               PrivacyVideoToolPaths toolPaths);

    static QSize fixedPixelSize();
    static QByteArray canonicalContainerForFileName(const QString& fileName);
    static bool isSameContainerCandidate(const QString& fileName);

    PrivacyVideoProxyResult generate(const PrivacyVideoProxyRequest& request) const;

private:

    PrivacyProcessRunner& m_runner;
    PrivacyVideoToolPaths m_toolPaths;
};

} // namespace Digikam
