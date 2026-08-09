/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-03
 * Description : Loading parameters for multithreaded loading
 *
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "loadingdescription.h"

// Local includes

#include "icctransform.h"
#include "thumbnailinfo.h"
#include "thumbnailsize.h"

namespace Digikam
{

bool LoadingDescription::PreviewParameters::operator==(const PreviewParameters& other) const
{
    return (
            (type             == other.type)            &&
            (size             == other.size)            &&
            (flags            == other.flags)           &&
            (previewSettings  == other.previewSettings) &&
            (extraParameter   == other.extraParameter)  &&
            (storageReference == other.storageReference)
           );
}

bool LoadingDescription::PreviewParameters::onlyPregenerate() const
{
    return (flags & OnlyPregenerate);
}

bool LoadingDescription::PreviewParameters::onlyFromStorage() const
{
    return (flags & OnlyFromStorage);
}

bool LoadingDescription::PostProcessingParameters::operator==(const PostProcessingParameters& other) const
{
    return (colorManagement == other.colorManagement);
}

bool LoadingDescription::PostProcessingParameters::needsProcessing() const
{
    return (colorManagement != NoColorConversion);
}

void LoadingDescription::PostProcessingParameters::setTransform(const IccTransform& transform)
{
    iccData = QVariant::fromValue<IccTransform>(transform);
}

bool LoadingDescription::PostProcessingParameters::hasTransform() const
{
    return (!iccData.isNull() && iccData.canConvert<IccTransform>());
}

IccTransform LoadingDescription::PostProcessingParameters::transform() const
{
    return iccData.value<IccTransform>();
}

void LoadingDescription::PostProcessingParameters::setProfile(const IccProfile& profile)
{
    iccData = QVariant::fromValue<IccProfile>(profile);
}

bool LoadingDescription::PostProcessingParameters::hasProfile() const
{
    return (!iccData.isNull() && iccData.canConvert<IccProfile>());
}

IccProfile LoadingDescription::PostProcessingParameters::profile() const
{
    return iccData.value<IccProfile>();
}

// ----------------------------------------------------------------------------

LoadingDescription::LoadingDescription()
    : filePath                (QString()),
      rawDecodingSettings     (DRawDecoding()),
      previewParameters       (PreviewParameters()),
      postProcessingParameters(PostProcessingParameters())
{
}

LoadingDescription::LoadingDescription(const QString& filePath,
                                       ColorManagementSettings cm)
    : filePath           (filePath),
      rawDecodingSettings(DRawDecoding()),
      previewParameters  (PreviewParameters())
{
      postProcessingParameters.colorManagement = cm;
}

LoadingDescription::LoadingDescription(const QString& filePath,
                                       const DRawDecoding& settings,
                                       RawDecodingHint hint,
                                       ColorManagementSettings cm)
    : filePath           (filePath),
      rawDecodingSettings(settings),
      rawDecodingHint    (hint),
      previewParameters  (PreviewParameters())
{
      postProcessingParameters.colorManagement = cm;
}

LoadingDescription::LoadingDescription(const QString& filePath,
                                       const PreviewSettings& previewSettings,
                                       int size,
                                       ColorManagementSettings cm,
                                       LoadingDescription::PreviewParameters::PreviewType type)
    : filePath           (filePath),
      rawDecodingSettings(DRawDecoding())
{
    previewParameters.type                   = type;
    previewParameters.size                   = size;
    previewParameters.previewSettings        = previewSettings;
    postProcessingParameters.colorManagement = cm;
}

QString LoadingDescription::cacheKey() const
{
    // Here we have the knowledge which LoadingDescriptions / RawFileDecodingSettings
    // must be cached separately.

    // Thumbnail loading. This one is easy.

    if      (previewParameters.type == PreviewParameters::Thumbnail)
    {
        QString fileRef = filePath.isEmpty() ? (QLatin1String("id:/") + previewParameters.storageReference.toString())
                                             : filePath;

        return namespacedCacheKey(fileRef + QLatin1String("-thumbnail-") +
                                  QString::number(previewParameters.size));
    }
    else if (previewParameters.type == PreviewParameters::DetailThumbnail)
    {
        QString fileRef    = filePath.isEmpty() ? (QLatin1String("id:/") + previewParameters.storageReference.toString())
                                                : filePath;
        QRect rect         =  previewParameters.extraParameter.toRect();
        QString rectString = QString::fromLatin1("%1,%2-%3x%4-")
                             .arg(rect.x())
                             .arg(rect.y())
                             .arg(rect.width())
                             .arg(rect.height());

        return namespacedCacheKey(fileRef + QLatin1String("-thumbnail-") + rectString +
                                  QString::number(previewParameters.size));
    }

    // DImg loading

    if (previewParameters.type == PreviewParameters::NoPreview)
    {
        // Assumption: Full loading. For Raw images, we need to check all parameters here.
        // Non-raw images will always be loaded full-size.
        // NOTE: do not identify these by cache key only, check the settings!

        if      (rawDecodingHint == RawDecodingGlobalSettings)
        {
            return namespacedCacheKey(filePath + QLatin1String("-globalraw"));
        }
        else if (rawDecodingHint == RawDecodingCustomSettings)
        {
            return namespacedCacheKey(filePath + QLatin1String("-customraw"));
        }
    }
    else
    {
        // Assumption: Size-limited previews are always eight bit and do not care for raw settings.

        if (previewParameters.size)
        {
            return namespacedCacheKey(filePath + QLatin1String("-previewImage-") +
                                      QString::number(previewParameters.size));
        }
        else
        {
            return namespacedCacheKey(filePath + QLatin1String("-previewImage"));
        }
    }

    QString suffix;

    // Assumption: Time-optimized loading is used for previews and non-previews

    if (rawDecodingHint == RawDecodingTimeOptimized)
    {
        // Assumption: With time-optimized, we can have 8 or 16bit and halfSize or demosaiced.

        suffix += QLatin1String("-timeoptimized");

        if (!rawDecodingSettings.rawPrm.sixteenBitsImage)
        {
            suffix += QLatin1String("-8");
        }

        if (rawDecodingSettings.rawPrm.halfSizeColorImage)
        {
            suffix += QLatin1String("-halfSize");
        }
    }

    return namespacedCacheKey(filePath + suffix);
}

QStringList LoadingDescription::lookupCacheKeys() const
{
    // Build a hierarchy which cache entries may be used for this LoadingDescription.

    // Thumbnail loading. No other cache key included!

    if ((previewParameters.type == PreviewParameters::Thumbnail) ||
        (previewParameters.type == PreviewParameters::DetailThumbnail))
    {
        return QStringList() << cacheKey();
    }

    // DImg loading.
    // Typically, the first is the best. An actual loading operation may use a
    // lower-quality loading and will effectively only add the last entry of the
    // list to the cache, although it can accept the first if already available.
    // Hierarchy:
    //  Raw with GlobalSettings and CustomSettings
    //  Raw with optimized loading, 8 or 16bit
    //      full size
    //      halfSize
    //  "Normal" image (default raw parameters)
    //  Preview image
    //      full size
    //      reduced size

    QStringList cacheKeys;

    if (previewParameters.type != PreviewParameters::NoPreview)
    {
        if (previewParameters.size)
        {
            cacheKeys << filePath + QLatin1String("-previewImage-") + QString::number(previewParameters.size);
        }

        // full size preview

        cacheKeys << filePath + QLatin1String("-previewImage");
    }

    if (rawDecodingHint == RawDecodingDefaultSettings)
    {
        cacheKeys << filePath;
    }

    if (rawDecodingHint == RawDecodingTimeOptimized)
    {
        if (rawDecodingSettings.rawPrm.sixteenBitsImage)
        {
            cacheKeys << filePath + QLatin1String("-timeoptimized");

            if (rawDecodingSettings.rawPrm.halfSizeColorImage)
            {
                cacheKeys << filePath + QLatin1String("-timeoptimized-halfSize");
            }
        }
        else
        {
            cacheKeys << filePath + QLatin1String("-timeoptimized-8");

            if (rawDecodingSettings.rawPrm.halfSizeColorImage)
            {
                cacheKeys << filePath + QLatin1String("-timeoptimized-8-halfSize");
            }
        }
    }

    if      (rawDecodingHint == RawDecodingGlobalSettings)
    {
        cacheKeys << filePath + QLatin1String("-globalraw");
    }
    else if (rawDecodingHint == RawDecodingCustomSettings)
    {
        cacheKeys << filePath + QLatin1String("-customraw");
    }

    return namespacedCacheKeys(cacheKeys);
}

bool LoadingDescription::needCheckRawDecoding() const
{
    return ((rawDecodingHint == RawDecodingGlobalSettings) ||
            (rawDecodingHint == RawDecodingCustomSettings));
}

bool LoadingDescription::isReducedVersion() const
{
    // return true if this loads anything but the full version

    return (rawDecodingSettings.rawPrm.halfSizeColorImage ||
            (previewParameters.type != PreviewParameters::NoPreview));
}

bool LoadingDescription::operator==(const LoadingDescription& other) const
{
    return (equalsIgnoringSourceResolution(other)                          &&
            (m_sourceDisposition      == other.m_sourceDisposition)        &&
            (m_sourceCachePolicy      == other.m_sourceCachePolicy)        &&
            ((!m_sourceResolutionApplied || !other.m_sourceResolutionApplied) ||
             (m_sourceResolverGeneration == other.m_sourceResolverGeneration)) &&
            (m_sourceFilePath         == other.m_sourceFilePath)           &&
            (m_sourceEncodedBytes     == other.m_sourceEncodedBytes)       &&
            (m_privacyCacheNamespace  == other.m_privacyCacheNamespace));
}

bool LoadingDescription::operator!=(const LoadingDescription& other) const
{
    return (!operator==(other));
}

bool LoadingDescription::equalsIgnoreReducedVersion(const LoadingDescription& other) const
{
    return ((filePath                == other.filePath)                &&
            (m_sourceDisposition     == other.m_sourceDisposition)     &&
            (m_sourceCachePolicy     == other.m_sourceCachePolicy)     &&
            ((!m_sourceResolutionApplied || !other.m_sourceResolutionApplied) ||
             (m_sourceResolverGeneration == other.m_sourceResolverGeneration)) &&
            (m_sourceFilePath        == other.m_sourceFilePath)        &&
            (m_sourceEncodedBytes    == other.m_sourceEncodedBytes)    &&
            (m_privacyCacheNamespace == other.m_privacyCacheNamespace));
}

bool LoadingDescription::equalsIgnoringSourceResolution(const LoadingDescription& other) const
{
    return ((filePath                 == other.filePath)                   &&
            (rawDecodingSettings      == other.rawDecodingSettings)        &&
            (previewParameters        == other.previewParameters)          &&
            (postProcessingParameters == other.postProcessingParameters));
}

bool LoadingDescription::equalsOrBetterThan(const LoadingDescription& other) const
{
    // This method is similar to operator==. But it returns true as well if this
    // loads a "better" version than <other>.
    // Preview parameters must have the same size, or other has no size restriction.
    // Comparing raw decoding settings is complicated. We allow to be loaded with optimizeTimeLoading().

    DRawDecoding fast = rawDecodingSettings;
    fast.optimizeTimeLoading();

    return (
            (filePath                == other.filePath)                &&
            (m_sourceDisposition     == other.m_sourceDisposition)     &&
            (m_sourceCachePolicy     == other.m_sourceCachePolicy)     &&
            ((!m_sourceResolutionApplied || !other.m_sourceResolutionApplied) ||
             (m_sourceResolverGeneration == other.m_sourceResolverGeneration)) &&
            (m_sourceFilePath        == other.m_sourceFilePath)        &&
            (m_sourceEncodedBytes    == other.m_sourceEncodedBytes)    &&
            (m_privacyCacheNamespace == other.m_privacyCacheNamespace) &&
            (
                (rawDecodingSettings == other.rawDecodingSettings) ||
                (fast                == other.rawDecodingSettings)
            ) &&
            (
                (previewParameters.size == other.previewParameters.size) ||
                other.previewParameters.size
            )
           );
}

bool LoadingDescription::isThumbnail() const
{
    return ((previewParameters.type == PreviewParameters::Thumbnail) ||
            (previewParameters.type == PreviewParameters::DetailThumbnail));
}

bool LoadingDescription::isPreviewImage() const
{
    return (previewParameters.type == PreviewParameters::PreviewImage);
}

ThumbnailIdentifier LoadingDescription::thumbnailIdentifier() const
{
    ThumbnailIdentifier id;

    if (!isThumbnail())
    {
        return id;
    }

    id.filePath = filePath;
    id.id       = previewParameters.storageReference.toLongLong();
    id.sourceFilePath      = m_sourceFilePath;
    id.sourceEncodedBytes  = m_sourceEncodedBytes;
    id.cacheNamespace      = m_privacyCacheNamespace;
    id.sourceResolverGeneration = m_sourceResolverGeneration;
    id.sourceResolutionApplied = m_sourceResolutionApplied;
    id.sourceAccessDenied  = isSourceDenied();
    id.detailThumbnail     = (previewParameters.type ==
                              PreviewParameters::DetailThumbnail);
    // Detail/crop identifiers include an arbitrary rectangle. There is no
    // reverse logical-owner index in either persistent thumbnail backend, so
    // old privacy generations could not be enumerated safely for deletion.
    // Keep handled detail derivatives memory-only; locked primary proxies may
    // still use the persistent cache and are addressable by their prior id.

    id.persistentCacheAllowed = persistentCacheAllowed() &&
                                ((previewParameters.type != PreviewParameters::DetailThumbnail) ||
                                 m_privacyCacheNamespace.isEmpty());

    return id;
}

void LoadingDescription::resolveSource()
{
    if (m_sourceResolutionApplied)
    {
        return;
    }

    PrivacySourceRequest request;
    request.logicalFilePath = filePath;
    request.itemReference   = previewParameters.storageReference;

    if      (isThumbnail())
    {
        request.consumer = PrivacySourceRequest::Thumbnail;
        request.detailThumbnail = (previewParameters.type ==
                                   PreviewParameters::DetailThumbnail);
    }
    else if (isPreviewImage())
    {
        request.consumer = PrivacySourceRequest::Preview;
    }
    else
    {
        request.consumer = PrivacySourceRequest::Image;
    }

    const PrivacySourceResult result = PrivacySourceResolver::resolve(request);

    m_sourceResolutionApplied = true;
    m_sourceDisposition       = result.disposition;
    m_sourceCachePolicy       = result.cachePolicy;
    m_sourceResolverGeneration = result.resolverGeneration;
    m_sourceFilePath          = result.physicalFilePath;
    m_sourceEncodedBytes      = result.encodedBytes;
    m_privacyCacheNamespace   = result.cacheNamespace;
}

void LoadingDescription::resetSourceResolution()
{
    m_sourceResolutionApplied = false;
    m_sourceDisposition       = PrivacySourceResult::NotHandled;
    m_sourceCachePolicy       = PrivacySourceResult::Persistent;
    m_sourceResolverGeneration = 0;
    m_sourceFilePath.clear();
    m_sourceEncodedBytes.clear();
    m_privacyCacheNamespace.clear();
}

QString LoadingDescription::effectiveFilePath() const
{
    if (isSourceDenied())
    {
        return QString();
    }

    if ((m_sourceDisposition == PrivacySourceResult::Resolved) &&
        !m_sourceFilePath.isEmpty())
    {
        return m_sourceFilePath;
    }

    return filePath;
}

bool LoadingDescription::isSourceDenied() const
{
    return (m_sourceDisposition == PrivacySourceResult::Denied);
}

bool LoadingDescription::sourceResolutionApplied() const
{
    return m_sourceResolutionApplied;
}

bool LoadingDescription::sourceResolutionIsCurrent() const
{
    if (!m_sourceResolutionApplied)
    {
        return false;
    }

    PrivacySourceRequest request;
    request.logicalFilePath = filePath;
    request.itemReference   = previewParameters.storageReference;

    if      (isThumbnail())
    {
        request.consumer = PrivacySourceRequest::Thumbnail;
        request.detailThumbnail = (previewParameters.type ==
                                   PreviewParameters::DetailThumbnail);
    }
    else if (isPreviewImage())
    {
        request.consumer = PrivacySourceRequest::Preview;
    }
    else
    {
        request.consumer = PrivacySourceRequest::Image;
    }

    const PrivacySourceResult current = PrivacySourceResolver::resolve(request);

    return ((m_sourceDisposition      == current.disposition)      &&
            (m_sourceCachePolicy      == current.cachePolicy)      &&
            (m_sourceResolverGeneration == current.resolverGeneration) &&
            (m_sourceFilePath         == current.physicalFilePath) &&
            (m_sourceEncodedBytes     == current.encodedBytes) &&
            (m_privacyCacheNamespace  == current.cacheNamespace));
}

bool LoadingDescription::persistentCacheAllowed() const
{
    return (m_sourceCachePolicy == PrivacySourceResult::Persistent);
}

QString LoadingDescription::privacyCacheNamespace() const
{
    return m_privacyCacheNamespace;
}

quint64 LoadingDescription::sourceResolverGeneration() const
{
    return m_sourceResolverGeneration;
}

QString LoadingDescription::namespacedCacheKey(const QString& legacyKey) const
{
    if (m_privacyCacheNamespace.isEmpty())
    {
        return legacyKey;
    }

    return (QLatin1String("digikam-private-cache:/v1/") +
            PrivacySourceResolver::cacheNamespaceDigest(
                m_privacyCacheNamespace + QLatin1Char('\0') +
                QString::number(m_sourceResolverGeneration)) +
            QLatin1Char('/') + legacyKey);
}

QStringList LoadingDescription::namespacedCacheKeys(const QStringList& legacyKeys) const
{
    if (m_privacyCacheNamespace.isEmpty())
    {
        return legacyKeys;
    }

    QStringList keys;
    keys.reserve(legacyKeys.size());

    for (const QString& key : legacyKeys)
    {
        keys << namespacedCacheKey(key);
    }

    return keys;
}

QStringList LoadingDescription::possibleCacheKeys(const QString& filePath)
{
    QStringList keys;
    keys << filePath ;
    keys << filePath + QLatin1String("-timeoptimized-8-halfSize");
    keys << filePath + QLatin1String("-timeoptimized-8");
    keys << filePath + QLatin1String("-timeoptimized-halfSize");
    keys << filePath + QLatin1String("-timeoptimized");
    keys << filePath + QLatin1String("-customraw");
    keys << filePath + QLatin1String("-globalraw");

    for (int i = 1 ; i <= ThumbnailSize::HD ; ++i)
    {
        keys << filePath + QLatin1String("-previewImage-") + QString::number(i);
    }

    return keys;
}

QStringList LoadingDescription::possibleThumbnailCacheKeys(const QString& filePath)
{
    // FIXME: With details, there is an endless number of possible cache keys. Need different approach.

    QStringList keys;

    // there are (ThumbnailSize::HD) possible keys...

    QString path = filePath + QLatin1String("-thumbnail-");

    for (int i = 1 ; i <= ThumbnailSize::HD ; ++i)
    {
        keys << path + QString::number(i);
    }

    return keys;
}

} // namespace Digikam
