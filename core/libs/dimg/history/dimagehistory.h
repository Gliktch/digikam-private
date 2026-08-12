/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class for manipulating modifications changeset for non-destruct. editing
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QHash>
#include <QList>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>
#include <QVariant>

// Local includes

#include "digikam_export.h"
#include "filteraction.h"
#include "historyimageid.h"

namespace Digikam
{

class DIGIKAM_EXPORT DImageHistory
{
public:

    class Entry
    {
    public:

        /**
         * @brief A DImageHistory is a list of entries.
         *
         * Each entry has one action. The action can be null,
         * but it shall be null only if it is the action of the first
         * entry, with the "Original" as referred image,
         * representing the action of digitization.
         *
         * There can be zero, one or any number
         * of referred images per entry.
         * A referred image is a file in the state after the action is applied.
         */
        FilterAction          action;
        QList<HistoryImageId> referredImages;
    };

public:

    DImageHistory();
    DImageHistory(const DImageHistory& other);
    ~DImageHistory();

    DImageHistory& operator=(const DImageHistory& other);

    /**
     * @brief A history is null if it is constructed with the default constructor
     */
    bool isNull()                                                           const;

    /**
     * @brief A history is considered empty if there are no entries.
     */
    bool isEmpty()                                                          const;

    /**
     * @brief A history is a valid history (telling something about the past),
     * if the history is not empty, and there is at least one
     * referred image other than the "Current" entry,
     * or there is a valid action.
     */
    bool isValid()                                                          const;

    /// @return The number of entries
    int size()                                                              const;

    bool operator==(const DImageHistory& other)                             const;

    bool operator!=(const DImageHistory& other)                             const
    {
        return !operator==(other);
    }

    bool operator<(const DImageHistory& other)                              const;
    bool operator>(const DImageHistory& other)                              const;

    /**
     * @brief Appends a new filter action to the history.
     */
    DImageHistory& operator<<(const FilterAction& action);

    /**
     * @brief Appends a new referred image, representing the current state
     * of the history.
     * If you add an id of type Current, adjustReferredImages() will be called.
     */
    DImageHistory& operator<<(const HistoryImageId& imageId);

    void appendReferredImage(const HistoryImageId& id);
    void insertReferredImage(int entryIndex, const HistoryImageId& id);

    /// @brief Removes the last entry from the history
    void removeLast();

    /**
     * @brief Access entries.
     * There are size() entries.
     */
    QList<DImageHistory::Entry>&       entries();
    const QList<DImageHistory::Entry>& entries()                            const;
    Entry&                             operator[](int i);
    const Entry&                       operator[](int i)                    const;

    /**
     * @brief Access actions.
     *
     * There is one action per entry,
     * but the action may be null.
     */

    /// @return True if there is any non-null action
    bool hasActions()                                                       const;
    bool hasFilters()                                                       const
    {
        return hasActions();
    }

    /// @return The number of non-null actions
    int actionCount()                                                       const;

    /// @brief Gets all actions which are not null
    QList<FilterAction> allActions()                                        const;
    const FilterAction& action(int i)                                       const;

    /**
     * @brief Access referred images
     */
    QList<HistoryImageId>& referredImages(int i);
    const QList<HistoryImageId>& referredImages(int i)                      const;
    QList<HistoryImageId> allReferredImages()                               const;
    HistoryImageId currentReferredImage()                                   const;
    HistoryImageId originalReferredImage()                                  const;
    QList<HistoryImageId> referredImagesOfType(HistoryImageId::Type type)   const;
    bool hasReferredImages()                                                const;
    bool hasReferredImageOfType(HistoryImageId::Type type)                  const;
    bool hasCurrentReferredImage()                                          const;
    bool hasOriginalReferredImage()                                         const;

    /**
     * @brief Edit referred images
     */

    /// @brief Remove all referredImages, leaving the entries list untouched
    void clearReferredImages();

    /**
     * @brief Adjusts the type of a Current HistoryImageId:
     * If it is the first entry, it becomes Original,
     * if it is in an intermediate entry, it becomes Intermediate,
     * if in the last entry, it stays current.
     */
    void adjustReferredImages();

    /// @brief Changes the UUID of the current (last added current) referred image
    void adjustCurrentUuid(const QString& uuid);

    /**
     * @brief Remove file path entries pointing to the given absolute path
     * from any referred images. This is useful when said file
     * is about to be overwritten.
     * All other HistoryImageId fields remain unchanged, no HistoryImageId is removed.
     * path: directory path, without filename.
     */
    void purgePathFromReferredImages(const QString& path, const QString& fileName);

    /**
     * @brief Change file path entries of the current referred image
     */
    void moveCurrentReferredImage(const QString& newPath, const QString& newFileName);

    /**
     * @brief Serialize toand from XML.
     *
     * @note The "Current" entry is skipped when writing to XML,
     * so make sure the file into the metadata of which you write the XML,
     * is the file marked as "Current" in this history.
     */
    QString toXml()                                                         const;
    static DImageHistory fromXml(const QString& xml);

public:

    /// @note Set as public there because of PrivateSharedNull
    class Private;

private:

    QSharedDataPointer<Private> d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::DImageHistory)
