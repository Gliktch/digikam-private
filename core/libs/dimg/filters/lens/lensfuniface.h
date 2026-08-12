/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * SPDX-FileCopyrightText: 2008      by Adrian Schroeter <adrian at suse dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// LensFun includes

/*
 * Pragma directives to reduce warnings from Lensfun header files.
 *
 */
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmismatched-tags"
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <lensfun.h>

/**
 * Make an integer version number of Lensfun for comparison from a major, minor and a patch version number.
 */
#define LENSFUN_MAKE_VERSION(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))

/**
 * The Lensfun version number of the library used at compile-time as an integer number for easy comparison.
 */
#define LENSFUN_VERSION LENSFUN_MAKE_VERSION(LF_VERSION_MAJOR, LF_VERSION_MINOR, LF_VERSION_MICRO)

/**
 * The Lensfun version test macro to branch codes depending of API.
 */
#define LENSFUN_TEST_VERSION(major,minor,patch) ( LENSFUN_VERSION >= LENSFUN_MAKE_VERSION(major,minor,patch) )

// Restore warnings
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "dmetadata.h"
#include "digikam_export.h"
#include "lensfunfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT LensFunIface
{
public:

    typedef const lfCamera* DevicePtr;
    typedef const lfLens*   LensPtr;
    typedef QList<LensPtr>  LensList;

public:

    enum MetadataMatch
    {
        MetadataUnavailable  = -2,
        MetadataNoMatch      = -1,
        MetadataPartialMatch = 0,
        MetadataExactMatch   = 1
    };

public:

    LensFunIface();
    ~LensFunIface();

    void setFilterSettings(const LensFunContainer& other);

    void             setSettings(const LensFunContainer& other);
    LensFunContainer settings()                                     const;

    MetadataMatch findFromMetadata(const DMetadata* const meta);

    bool supportsDistortion()                                       const;
    bool supportsCCA()                                              const;
    bool supportsVig()                                              const;
    bool supportsGeometry()                                         const;


    /**
     * Return Camera maker string description found in metadata
     */
    QString makeDescription()                                       const;

    /**
     *Return Camera model string description found in metadata
     */
    QString modelDescription()                                      const;

    /**
     * Return Lens string description found in metadata
     */
    QString lensDescription()                                       const;

    static QString lensFunVersion();

public:

    // Lensfun API based methods

    LensPtr usedLens()                                              const;
    void    setUsedLens(LensPtr lens);

    DevicePtr usedCamera()                                          const;
    void      setUsedCamera(DevicePtr cam);

    lfDatabase*            lensFunDataBase()                        const;
    const lfCamera* const* lensFunCameras()                         const;

    DevicePtr findCamera(const QString& make, const QString& model) const;
    LensPtr   findLens(const QString& model)                        const;

private:

    /// @note disabled
    LensFunIface(const LensFunIface&)            = delete;
    LensFunIface& operator=(const LensFunIface&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::LensFunIface::DevicePtr)
Q_DECLARE_METATYPE(Digikam::LensFunIface::LensPtr)
