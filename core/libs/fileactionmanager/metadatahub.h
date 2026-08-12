/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-01-05
 * Description : Metadata handling
 *
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QMap>

// Local includes

#include "metaenginesettings.h"
#include "captionvalues.h"
#include "dmetadata.h"
#include "dimg.h"

namespace Digikam
{

class ApplicationSettings;
class ItemInfo;
class Template;

class MetadataHub
{
public:

    /**
     * The status enum describes the result of joining several metadata sets.
     * If only one set has been added, the status is always MetadataAvailable.
     * If no set has been added, the status is always MetadataInvalid
     */
    enum Status
    {
        MetadataInvalid,   ///< not yet filled with any value
        MetadataAvailable  ///< only one data set has been added, or a common value is available
    };

    enum WriteMode
    {
        /**
         * Write all available information
         */
        FullWrite,

        /**
         * Do a full write if and only if
         *     - metadata fields changed
         *     - the changed fields shall be written according to write settings
         * "Changed" in this context means changed by one of the set... methods,
         * the load() methods are ignored for this attribute.
         * This mode allows to avoid write operations when e.g. the user does not want
         * keywords to be written and only changes keywords.
         */
        FullWriteIfChanged,

        /**
         * Write only the changed parts.
         * Metadata fields which cannot be changed from MetadataHub (photographer ID etc.)
         * will never be written
         */
        PartialWrite
    };

    enum WriteComponents
    {
        WRITE_DATETIME  = 1,
        WRITE_TITLE     = 2,
        WRITE_COMMENTS  = 4,
        WRITE_PICKLABEL = 8,
        WRITE_COLORLABEL= 16,
        WRITE_RATING    = 32,
        WRITE_TEMPLATE  = 64,
        WRITE_TAGS      = 128,
        WRITE_POSITION  = 256,
        WRITE_ALL       = 511
    };
    Q_DECLARE_FLAGS(WriteComponent, WriteComponents)

public:

    /**
     * Constructs a MetadataHub.
     */
    MetadataHub();
    ~MetadataHub();

    void reset();

    // --------------------------------------------------

    /**
     * Add metadata information contained in the ItemInfo object.
     * This method (or in combination with the other load methods)
     * can be called multiple times on the same MetadataHub object.
     * In this case, the metadata will be combined.
     */
    void load(const ItemInfo& info);

//    /**
//     * Add metadata information from the meta engine object
//     */
//    void load(const DMetadata& metadata);

//    /**
//     * Load metadata information from the given file.
//     * (Uses DMetadata, QFileInfo)
//     * @returns True if the metadata could be loaded
//     */
//    bool load(const QString& filePath, const MetaEngineSettingsContainer& settings = MetaEngineSettings::instance()->settings());

    // --------------------------------------------------

    /**
     * @brief writeToMetadata write to metadata using image info to retrieve tags and filepath
     *                        use this method when multiple image infos are loaded in hub
     *
     * @param info           The image info to retrieve current tags
     * @param writeMode      The mode to write metadata
     * @param ignoreLazySync The flag to ignore the lazy sync metadata stage
     * @param settings       The metadata settings to be set
     *
     * @return true if everything is successful
     */
    bool writeToMetadata(const ItemInfo& info,
                         WriteComponent writeMode = WRITE_ALL,
                         bool ignoreLazySync = false,
                         const MetaEngineSettingsContainer& settings = MetaEngineSettings::instance()->settings());


    /**
     * @brief Constructs a meta engine object for given filePath,
     *        calls the above method, writes the changes out to the file,
     *        and notifies the ItemAttributesWatch.
     *
     * @param filePath       The file path to retrieve current tags
     * @param writeMode      The mode to write metadata
     * @param ignoreLazySync The flag to ignore the lazy sync metadata stage
     * @param settings       The metadata settings to be set
     *
     * @warning Do not use this method when multiple image infos are loaded
     *          It will result in disjoint tags not being written
     *          Use writeToMetadata(Image info ...) instead
     *
     * @return Returns if the file has been touched
     */
    bool write(const QString& filePath,
               WriteComponent writeMode = WRITE_ALL,
               bool ignoreLazySync = false,
               const MetaEngineSettingsContainer& settings = MetaEngineSettings::instance()->settings());

    /**
     * @brief Constructs a meta engine object from the metadata stored in the given DImg object,
     *        calls the above method, and changes the stored metadata in the DImg object.
     *
     * @param image          The DImg container to retrieve current tags
     * @param writeMode      The mode to write metadata
     * @param ignoreLazySync The flag to ignore the lazy sync metadata stage
     * @param settings       The metadata settings to be set
     *
     * @return Returns if the DImg object has been touched
     */
    bool write(const DImg& image,
               WriteComponent writeMode = WRITE_ALL,
               bool ignoreLazySync = false,
               const MetaEngineSettingsContainer& settings = MetaEngineSettings::instance()->settings());

    /**
     * @brief Will write only Tags to image. Used by TagsManager to write tags to image
     *        Other metadata are not updated.

     * @param filePath       The file path to update current tags
     * @param writeMode      The mode to write metadata
     * @param settings       The metadata settings to be set
     *
     * @return if tags were successfully written.
     */
    bool writeTags(const QString& filePath,
                   WriteComponent writeMode = WRITE_ALL,
                   const MetaEngineSettingsContainer& settings = MetaEngineSettings::instance()->settings());

    /**
     * @brief Used to deduplicate code from writeTags and usual write, all write to tags
     *        operations must be done here
     *
     * @param metadata meta engine object that apply changes
     * @param saveTags save switch
     *
     * @return if tags were successfully set
     */
    bool writeTags(const DMetadata& metadata, bool saveTags);

    /**
     * @brief cleanupTags remove duplicates and obsolete tags before setting metadata
     *
     * @param toClean tag list to be cleared and de-duplicated
     *
     * @return clean tag list
     */
    QStringList cleanupTags(const QStringList& toClean);

    /**
     * @brief With the currently applied changes, the given writeMode and settings,
     *
     * @param writeMode      The mode to write metadata
     * @param settings       The metadata settings to be set
     *
     * @return if write(DMetadata), write(QString) or write(DImg) will actually
     * apply any changes.
     */
    bool willWriteMetadata(Digikam::MetadataHub::WriteComponent writeMode = WRITE_ALL,
                           const MetaEngineSettingsContainer& settings = MetaEngineSettings::instance()->settings()) const;

    /**
     * @brief write tags, comments and rating to KDE Nepomuk replacement: Baloo
     *
     * @param filePath path to file to add comments, tags and rating
     * @param settings metadata settings to be set
     */
    void writeToBaloo(const QString& filePath,
                      const MetaEngineSettingsContainer& settings = MetaEngineSettings::instance()->settings());

protected:

    /**
     * @brief Applies the set of metadata contained in this MetadataHub
     *        to the given meta engine object.
     *
     * @param metadata  The metadata backend instance.
     * @param writeMode The mode to write metadata.
     * @param settings  The MetaEngineSettingsContainer determine whether data is actually set or not.
     *                  The following metadata fields may be set (depending on settings):
     *                   - Comment
     *                   - Date
     *                   - Rating
     *                   - Tags
     *                   - Photographer ID (data from settings)
     *                   - Credits (data from settings)
     *
     * @note The data fields taken from this MetadataHub object are only set if their status is MetadataAvailable.
     *       If the status is MetadataInvalid or MetadataDisjoint, the respective metadata field is not touched.
     *
     * @return Returns true if the metadata object has been touched.
     */
    bool write(DMetadata& metadata,
               WriteComponent writeMode = WRITE_ALL,
               const MetaEngineSettingsContainer& settings = MetaEngineSettings::instance()->settings());

    void load(const QDateTime& dateTime,
              const CaptionsMap& titles,
              const CaptionsMap& comment,
              int colorLabel, int pickLabel,
              int rating, const Template& t);

    void loadTags(const QList<int>& loadedTagIds);
    void loadFaceTags(const ItemInfo& info);

    void notifyTagDeleted(int id);

    void applyChangeNotifications();

private:

    bool writeFaceTagsMap(const DMetadata& metadata, bool saveFaces);

private:

    class Private;
    Private* const d = nullptr;

private:

    /// @note disabled
    MetadataHub(const MetadataHub&)            = delete;
    MetadataHub& operator=(const MetadataHub&) = delete;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::MetadataHub::WriteComponent)
