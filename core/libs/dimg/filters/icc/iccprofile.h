/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-07
 * Description : a wrapper class for an ICC color profile
 *
 * SPDX-FileCopyrightText: 2005-2006 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QByteArray>
#include <QMetaType>
#include <QSharedData>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT IccProfile
{
public:

    enum ProfileType
    {
        /// @brief Returned for a null profile or an unknown (non-standard) profile type
        InvalidType,
        /// @brief For an input device like a scanner or digital camera
        Input,
        /// @brief For an output device like a printer
        Output,
        /// @brief For a display device like a monitor
        Display,
        Abstract,
        ColorSpace,
        DeviceLink,
        NamedColor
    };

public:

    /**
     * @brief Creates a null profile
     */
    IccProfile();

    /**
     * @brief Creates a profile from the given data in memory
     */
    explicit IccProfile(const QByteArray& data);

    /**
     * @brief Creates a profile from the given file
     */
    explicit IccProfile(const QString& filePath);

    /**
     * @return the profiles available with RawEngine. You still need to call open() on them.
     */
    static IccProfile sRGB();
    static IccProfile adobeRGB();
    static IccProfile wideGamutRGB();
    static IccProfile proPhotoRGB();

    /**
     * @return a list with the profiles above
     */
    static QList<IccProfile> defaultProfiles();

    IccProfile(const IccProfile& other);
    ~IccProfile();

    IccProfile& operator=(const IccProfile& other);

    bool isNull() const;

    /**
     *  @return true if both profiles are null, if both profiles are created from the
     *  same file profile, or if the loaded profile data is identical.
     *  @note This will not ensure that the data is loaded. Use isSameProfile().
     */
    bool operator==(const IccProfile& other) const;

    bool operator!=(const IccProfile& other) const
    {
        return !operator==(other);
    }

    /**
     * @brief This method compares the actual profile data bit by bit.
     */
    bool isSameProfileAs(IccProfile& other);

    /**
     * @brief Open this profile. @return true if the operation succeeded
     * or the profile is already open. @return false if the profile is null
     * or the operation failed.
     * You need to open each profile after construction.
     */
    bool open();

    /**
     * @brief Close the profile, freeing resources. You can re-open.
     * Called automatically at destruction.
     */
    void close();

    /**
     * @return if the profile is opened.
     */
    bool isOpen()       const;

    /**
     * @return the filename that this profile was read from.
     * @return a null QString() if this profile was loaded from memory.
     */
    QString filePath()  const;

    /**
     * @brief Reads the profile description. Opens the profile if necessary.
     */
    QString description();

    ProfileType type();

    /**
     * @return the raw profile data.
     * Reads the data from disk if loaded from disk and not yet loaded.
     */
    QByteArray data();

    /**
     * @brief Writes the profile to the given file.
     */
    bool writeToFile(const QString& filePath);

    /**
     * @brief Access to the LCMS cmsHPROFILE handle.
     */
    void* handle()      const;

    operator void*()    const
    {
        return handle();
    }

    /**
     * @return the default search paths for ICC profiles.
     * This does not include any user-specified settings.
     */
    static QStringList defaultSearchPaths();

    static QList<IccProfile> scanDirectories(const QStringList& dirs);

    static void considerOriginalAdobeRGB(const QString& filePath);

private:

    IccProfile(const char* const location, const QString& relativePath);

private:

    class Private;
    QSharedDataPointer<Private> d;
};

// --------------------------------------------------------------------------------------

class DIGIKAM_EXPORT LcmsLock
{
public:

    /**
     * @brief Obtain an LcmsLock if you access not clearly thread-safe LittleCMS methods
     */
    LcmsLock();
    ~LcmsLock();

private:

    /// @note disabled
    LcmsLock(const LcmsLock&)            = delete;
    LcmsLock& operator=(const LcmsLock&) = delete;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::IccProfile)
