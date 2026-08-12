/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QByteArray>
#include <QUrl>

// Local includes

#include "digikam_config.h"
#include "digikam_export.h"
#include "metaengine.h"
#include "metaengine_data.h"
#include "metaenginesettingscontainer.h"
#include "metadatainfo.h"
#include "captionvalues.h"
#include "photoinfocontainer.h"
#include "videoinfocontainer.h"
#include "dmetadatasettings.h"

namespace Digikam
{

class Template;
class IccProfile;

class DIGIKAM_EXPORT DMetadata : public MetaEngine
{

public:

    /**
     * @brief Video color model reported by FFMPEG following XMP DM Spec from Adobe.
     * These values are stored in DB as Image color model properties (extension of DImg::ColorModel)
     */
    enum VIDEOCOLORMODEL
    {
        VIDEOCOLORMODEL_UNKNOWN = 1000,
        VIDEOCOLORMODEL_OTHER,
        VIDEOCOLORMODEL_SRGB,
        VIDEOCOLORMODEL_BT709,
        VIDEOCOLORMODEL_BT601
    };

public:

    typedef QMap<QString, QString> CountryCodeMap;

public:

    DMetadata();
    explicit DMetadata(const QString& filePath);
    explicit DMetadata(const MetaEngineData& data);
    ~DMetadata()                                                                                                              override = default;

public: // Settings helpers

    void registerMetadataSettings();
    void setSettings(const MetaEngineSettingsContainer& settings);

public: // File I/O helpers

    /**
     * @brief Re-implemented from MetaEngine to use libraw identify, libheif,
     * ffmpeg probe, and ImageMAgick identify methods if Exiv2 failed.
     * If backend is non null, return the backend used to populate metadata (Exiv2).
     * See MetaEngine::Backend enum for details.
     */
    bool load(const QString& filePath, bool videoAll = false, Backend* backend = nullptr);
    bool save(const QString& filePath, bool setVersion = false)                                                         const override;
    bool applyChanges(bool setVersion = false)                                                                          const override;

    /**
     * @brief Try to extract metadata using Raw Engine identify method (libraw).
     */
    bool loadUsingRawEngine(const QString& filePath);

public: // History helpers

    QString getItemHistory()                                                                                            const;
    bool    setItemHistory(const QString& imageHistoryXml)                                                              const;
    bool    hasItemHistoryTag()                                                                                         const;

    QString getItemUniqueId()                                                                                           const;
    bool    setItemUniqueId(const QString& uuid)                                                                        const;

public: // Faces helpers

    /**
     * @brief Get Images Face Map based on tags stored in Picassa/Metadatagroup
     * format.
     */
    bool getItemFacesMap(QMultiMap<QString, QVariant>& facesPath)                                                       const;

    /**
     * @brief Set Images Face Map tags in Picassa/Metadatagroup format.
     *
     * @param facesPath The face  map to register in metadata based on tags stored in Picassa/Metadatagroup
     * @param write If true all faces will be written, else update mode
     *              search if at least a face tag exist and write if true.
     * @param size The size of the area grouping all faces in image.
     */
    bool setItemFacesMap(const QMultiMap<QString, QVariant>& facesPath, bool write, const QSize& size = QSize())        const;

    /**
     * @brief Remove Images Face Map tags from Picassa/Metadatagroup format.
     */
    bool removeItemFacesMap()                                                                                           const;

public: // Tags helpers

    bool getItemTagsPath(QStringList& tagsPath,
                         const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings())        const;
    bool setItemTagsPath(const QStringList& tagsPath,
                         const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings())        const;

    bool getACDSeeTagsPath(QStringList& tagsPath)                                                                       const;

    bool setACDSeeTagsPath(const QStringList& tagsPath)                                                                 const;

public: // Comments helpers

    CaptionsMap getItemComments(const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings()) const;
    bool setItemComments(const CaptionsMap& comments,
                         const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings())        const;

    CaptionsMap getItemTitles(const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings())   const;
    bool setItemTitles(const CaptionsMap& title,
                       const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings())          const;

    static MetaEngine::AltLangMap toAltLangMap(const QVariant& var);

public: // Labels helpers

    int  getItemRating(const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings())          const;
    bool setItemRating(int rating,
                       const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings())          const;

    int  getItemPickLabel(const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings())       const;
    bool setItemPickLabel(int pickId,
                          const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings())       const;

    int  getItemColorLabel(const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings())      const;
    bool setItemColorLabel(int colorId,
                           const DMetadataSettingsContainer& settings = DMetadataSettings::instance()->settings())      const;

public: // Template helpers

    bool     setMetadataTemplate(const Template& t)                                                                     const;
    Template getMetadataTemplate()                                                                                      const;
    bool     removeMetadataTemplate()                                                                                   const;

    /**
     * @brief Fills only the copyright values in the template. Use getMetadataTemplate() usually.
     * @return true if valid fields were read.
     */
    bool getCopyrightInformation(Template& t)                                                                           const;

public: // EXIF helpers

    /**
     * @brief Reads an IccProfile that is described or embedded in the metadata.
     * This method does not retrieve profiles embedded in the image but from the Exif metadata,
     * e.g. embedded profiles in JPEG images.
     * Returns a null profile if no profile is found.
     */
    IccProfile getIccProfile()                                                                                          const;

    /**
     * @brief Sets the IccProfile embedded in the Exif metadata.
     */
    bool setIccProfile(const IccProfile& profile);

    /**
     * @brief Remove the Exif color space identification from the image.
     */
    bool removeExifColorSpace()                                                                                         const;

    /**
     * @return millisecond time-stamp from Exif tags or 0 if not found.
     */
    int  getMSecsInfo()                                                                                                 const;

    /**
     * @brief Extract milliseconds time-stamp of photo from an Exif tag and store it to 'ms'.
     * @return true if data are extracted.
     */
    bool mSecTimeStamp(const char* const exifTagName, int& ms)                                                          const;

    bool removeExifTags(const QStringList& tagFilters);

private:

    QString getExifTagStringFromTagsList(const QStringList& tagsList)                                                   const;

public: // IPTC helpers

    IptcCoreContactInfo getCreatorContactInfo()                                                                         const;
    bool setCreatorContactInfo(const IptcCoreContactInfo& info)                                                         const;

    IptcCoreLocationInfo getIptcCoreLocation()                                                                          const;
    bool setIptcCoreLocation(const IptcCoreLocationInfo& location)                                                      const;

    QStringList getIptcCoreSubjects()                                                                                   const;

    bool removeIptcTags(const QStringList& tagFilters);

    /**
     * @return a map of ISO-639-1 2 letters country codes with country names.
     */
    static CountryCodeMap countryCodeMap();

    /**
     * @return a map of ISO-639-2 3 letters country codes with country names.
     */
    static CountryCodeMap countryCodeMap2();

private:

    bool setIptcTag(const QString& text,
                    int maxLength,
                    const char* const debugLabel,
                    const char* const tagKey)                                                                           const;

    QVariant fromIptcEmulateList(const char* const iptcTagName)                                                         const;
    QVariant fromIptcEmulateLangAlt(const char* const iptcTagName)                                                      const;

public: // XMP helpers

    bool removeXmpTags(const QStringList& tagFilters);

private:

    QVariant fromXmpList(const char* const xmpTagName)                                                                  const;
    QVariant fromXmpLangAlt(const char* const xmpTagName)                                                               const;

public: // Video helpers

    /**
     * @brief Try to extract metadata using FFMpeg probe method (libav).
     */
    bool loadUsingFFmpeg(const QString& filePath);

    /**
     * @return video metadata from Xmp tags.
     */
    VideoInfoContainer getVideoInformation()                                                                            const;

    /**
     * @brief Helper method to translate enum values to user presentable strings
     */
    static QString videoColorModelToString(VIDEOCOLORMODEL videoColorModel);

public: // Motion Photo helpers

    bool isMotionPhoto() const;

    QByteArray extractMotionPhotoVideo() const;

public: // Photo helpers

    /**
     * @return a string with Lens mounted on the front of camera.
     * There no standard Exif tag for Lens information.
     * Camera makernotes and Xmp tags are parsed.
     * Take a care : lens information are not standardized and string content is not homogeneous between
     * camera model/maker.
     */
    QString getLensDescription()                                                                                        const;

    /**
     * @return a string with Camera serial number.
     */
    QString getCameraSerialNumber()                                                                                     const;

    PhotoInfoContainer getPhotographInformation()                                                                       const;

    static double apexApertureToFNumber(double aperture);
    static double apexShutterSpeedToExposureTime(double shutterSpeed);

public: // Generic helpers

    /**
     * @return the requested metadata field as a QVariant. See metadatainfo.h for a specification
     * of the format of the QVariant.
     */
    QVariant     getMetadataField(MetadataInfo::Field field)                                                            const;
    QVariantList getMetadataFields(const MetadataFields& fields)                                                        const;

    /**
     * @brief Convert a QVariant value of the specified field to a user-presentable, i18n'ed string.
     * The QVariant must be of the type as specified in metadatainfo.h and as obtained by getMetadataField.
     */
    static QString     valueToString (const QVariant& value, MetadataInfo::Field field);
    static QStringList valuesToString(const QVariantList& list, const MetadataFields& fields);

    /**
     * @return a map of possible enum values and their user-presentable, i18n'ed representation.
     * Valid fields are those which are described as "enum from" or "bit mask from" in metadatainfo.h.
     */
    static QMap<int, QString> possibleValuesForEnumField(MetadataInfo::Field field);

private:

    QVariant fromExifOrXmp(const char* const exifTagName, const char* const xmpTagName)                                 const;
    QVariant fromIptcOrXmp(const char* const iptcTagName, const char* const xmpTagName)                                 const;
    QVariant fromExifOrXmpList(const QStringList& tagList)                                                              const;
    bool hasValidField(const QVariantList& list)                                                                        const;
    QVariant toStringListVariant(const QStringList& list)                                                               const;
    QVariant toStringVariant(const QString& str)                                                                        const;

#ifdef HAVE_HEIF

private:

    /**
     * @brief Libheif helper methods.
     */
    bool loadUsingLibheif(const QString& filePath);

#endif

private:

    /**
     * @brief ImageMagick helper methods.
     */
    bool loadUsingImageMagick(const QString& filePath);

private:

    /**
     * @brief ExifTool helper methods.
     */
    bool loadUsingExifTool(const QString& filePath, bool videoAll = false);
    bool saveUsingExifTool(const QString& filePath) const;
};

} // namespace Digikam
