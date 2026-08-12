/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-17
 * Description : Helper class for Image Description Editor Tab
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QMap>
#include <QObject>
#include <QDateTime>

// Local includes

#include "digikam_export.h"
#include "disjointmetadatadatafields.h"
#include "metaenginesettings.h"

namespace Digikam
{

class ItemInfo;
class CaptionsMap;
class Template;

class DIGIKAM_GUI_EXPORT DisjointMetadata : public QObject
{
    Q_OBJECT

public:

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

public:

    DisjointMetadata();
    ~DisjointMetadata()                                                           override;


    DisjointMetadataDataFields dataFields()                                 const;
    void setDataFields(const DisjointMetadataDataFields& data);

    void reset();

    void load(const ItemInfo& info);

    //@{

    /**
     * Returns the metadata field Status.
     */
    DisjointMetadataDataFields::Status dateTimeStatus()                     const;
    DisjointMetadataDataFields::Status titlesStatus()                       const;
    DisjointMetadataDataFields::Status commentsStatus()                     const;
    DisjointMetadataDataFields::Status pickLabelStatus()                    const;
    DisjointMetadataDataFields::Status colorLabelStatus()                   const;
    DisjointMetadataDataFields::Status ratingStatus()                       const;
    DisjointMetadataDataFields::Status templateStatus()                     const;

    DisjointMetadataDataFields::Status tagStatus(int albumId)               const;
    DisjointMetadataDataFields::Status tagStatus(const QString& tagPath)    const;

    //@}

    //@{

    /**
     * Returns if the metadata field has been changed
     * with the corresponding setter method.
     */
    bool dateTimeChanged()                                                  const;
    bool titlesChanged()                                                    const;
    bool commentsChanged()                                                  const;
    bool pickLabelChanged()                                                 const;
    bool colorLabelChanged()                                                const;
    bool ratingChanged()                                                    const;
    bool templateChanged()                                                  const;
    bool tagsChanged()                                                      const;

    //@}

    //@{

    /**
     * Set metadata field to the given value,
     * and the metadata field status to the corresponding DisjointMetadataDataFields::MetadataAvailable.
     */
    void setDateTime(const QDateTime& dateTime,
                     DisjointMetadataDataFields::Status status = DisjointMetadataDataFields::MetadataAvailable);
    void setTitles(const CaptionsMap& titles,
                   DisjointMetadataDataFields::Status status = DisjointMetadataDataFields::MetadataAvailable);
    void setComments(const CaptionsMap& comments,
                     DisjointMetadataDataFields::Status status = DisjointMetadataDataFields::MetadataAvailable);
    void setPickLabel(int pickId,
                      DisjointMetadataDataFields::Status status = DisjointMetadataDataFields::MetadataAvailable);
    void setColorLabel(int colorId,
                       DisjointMetadataDataFields::Status status = DisjointMetadataDataFields::MetadataAvailable);
    void setRating(int rating,
                   DisjointMetadataDataFields::Status status = DisjointMetadataDataFields::MetadataAvailable);
    void setMetadataTemplate(const Template& t,
                             DisjointMetadataDataFields::Status status = DisjointMetadataDataFields::MetadataAvailable);
    void setTag(int albumID,
                DisjointMetadataDataFields::Status status = DisjointMetadataDataFields::MetadataAvailable);

    /**
     * Special case if the metadata of color, pick or rating has already been changed outside.
     * Replace with current values as if there is no change.
     */
    void replaceColorLabel(int colorId);
    void replacePickLabel(int pickId);
    void replaceRating(int rating);

    //@}

    /**
     * Returns the dateTime.
     * If status is DisjointMetadataDataFields::MetadataDisjoint, the earliest date is returned.
     *                                (see dateTimeInterval())
     * If status is DisjointMetadataDataFields::MetadataInvalid, an invalid date is returned.
     */
    QDateTime   dateTime()                                                  const;

    /**
     * Returns a map all alternate language titles.
     * If status is DisjointMetadataDataFields::MetadataDisjoint, the first loaded map is returned.
     * If status is DisjointMetadataDataFields::MetadataInvalid, CaptionMap() is returned.
     */
    CaptionsMap titles()                                                    const;

    /**
     * Returns a map all alternate language omments .
     * If status is DisjointMetadataDataFields::MetadataDisjoint, the first loaded map is returned.
     * If status is DisjointMetadataDataFields::MetadataInvalid, CaptionMap() is returned.
     */
    CaptionsMap comments()                                                  const;

    /**
     * Returns the Pick Label id (see PickLabel values in globals.h).
     * If status is DisjointMetadataDataFields::MetadataDisjoint, the None Label is returned.
     *                                (see pickLabelInterval())
     * If status is DisjointMetadataDataFields::MetadataInvalid, -1 is returned.
     */
    int         pickLabel()                                                 const;

    /**
     * Returns the Color Label id (see ColorLabel values in globals.h).
     * If status is DisjointMetadataDataFields::MetadataDisjoint, the None Label is returned.
     *                                (see colorLabelInterval())
     * If status is DisjointMetadataDataFields::MetadataInvalid, -1 is returned.
     */
    int         colorLabel()                                                const;

    /**
     * Returns the rating.
     * If status is DisjointMetadataDataFields::MetadataDisjoint, the lowest rating is returned.
     *                                (see ratingInterval())
     * If status is DisjointMetadataDataFields::MetadataInvalid, -1 is returned.
     */
    int         rating()                                                    const;

    /**
     * Returns the metadata template.
     * If status is DisjointMetadataDataFields::MetadataDisjoint, the first loaded template is returned.
     * If status is DisjointMetadataDataFields::MetadataInvalid, 0 is returned.
     */
    Template    metadataTemplate()                                          const;

    /**
     * Returns the earliest and latest date.
     * If status is DisjointMetadataDataFields::MetadataAvailable, the values are the same.
     * If status is DisjointMetadataDataFields::MetadataInvalid, invalid dates are returned.
     */
    void        dateTimeInterval(QDateTime& lowest,
                                 QDateTime& highest)                        const;

    /**
     * Returns the lowest and highest Pick Label id (see PickLabel values from globals.h).
     * If status is DisjointMetadataDataFields::MetadataAvailable, the values are the same.
     * If status is DisjointMetadataDataFields::MetadataInvalid, -1 is returned.
     */
    void        pickLabelInterval(int& lowest,
                                  int& highest)                             const;

    /**
     * Returns the lowest and highest Color Label id (see ColorLabel values from globals.h).
     * If status is DisjointMetadataDataFields::MetadataAvailable, the values are the same.
     * If status is DisjointMetadataDataFields::MetadataInvalid, -1 is returned.
     */
    void        colorLabelInterval(int& lowest,
                                   int& highest)                            const;

    /**
     * Returns the lowest and highest rating.
     * If status is DisjointMetadataDataFields::MetadataAvailable, the values are the same.
     * If status is DisjointMetadataDataFields::MetadataInvalid, -1 is returned.
     */
    void        ratingInterval(int& lowest,
                               int& highest)                                const;

    /**
     * Returns a QStringList with all tags with status DisjointMetadataDataFields::MetadataAvailable.
     * (i.e., the intersection of tags from all loaded metadata sets)
     */
    QStringList keywords()                                                  const;

    /**
     * Returns a map with the status for each tag.
     * Any tag that was set on one of the loaded images is contained in the map.
     * (If a tag is not contained in the map, it was not set on any of the loaded images)
     * If the tag was set on all loaded images, the status is DisjointMetadataDataFields::MetadataAvailable.
     * If the tag was set on at least one, but not all of the loaded images, the status is DisjointMetadataDataFields::MetadataDisjoint.
     */
    QMap<int, DisjointMetadataDataFields::Status> tags()                    const;

    void resetChanged();

    /**
     * Applies the set of metadata contained in this MetadataHub
     * to the given ItemInfo object.
     * @return Returns true if the info object has been changed
     */
    bool write(ItemInfo info, WriteMode writeMode = FullWrite);

    /**
     * With the currently applied changes, the given writeMode and settings,
     * returns if write(DMetadata), write(QString) or write(DImg) will actually
     * apply any changes.
     */
    bool willWriteMetadata(WriteMode writeMode,
                           const MetaEngineSettingsContainer& settings =
                               MetaEngineSettings::instance()->settings()) const;

    /**
     * @brief changedFlags - used for selective metadata write. The result will be passed to metadatahub and it will
     *                     - write it to disk
     * @return - metadatahub flags encoded as int
     */
    int changedFlags();

private Q_SLOTS:

    void slotTagDeleted(int tagId);
    void slotInvalidate();

private:

    void load(const QDateTime& dateTime,
              const CaptionsMap& titles,
              const CaptionsMap& comment,
              int colorLabel, int pickLabel,
              int rating, const Template& t);

    void loadTags(const QList<int>& tagIds);
    void notifyTagDeleted(int id);
    void applyChangeNotifications();

private:

    /// @note disabled
    explicit DisjointMetadata(QObject*) = delete;

    /// @note disabled copy constructor and operator to prevent potential slicing with this class, reported by Clazy static analyzer.
    // https://github.com/KDE/clazy/blob/master/docs/checks/README-copyable-polymorphic.md
    // Use DisjointMetadataDataFields container setter and getter instead.
    // TODO: remove legacy implementations for these methods later if no side effect.
    DisjointMetadata(const DisjointMetadata& other);
    DisjointMetadata& operator=(const DisjointMetadata& other);

private:

    class Private;
    Private* d = nullptr;
};

} // namespace Digikam
