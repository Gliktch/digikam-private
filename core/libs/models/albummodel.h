/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-22
 * Description : Qt Model for Albums
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>

// Local includes

#include "abstractalbummodel.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT AlbumModel : public AbstractCheckableAlbumModel
{
    Q_OBJECT

public:

    /**
     * Create a model containing all physical albums
     */
    explicit AlbumModel(RootAlbumBehavior rootBehavior = IncludeRootAlbum, QObject* const parent = nullptr);
    ~AlbumModel() override = default;

    PAlbum* palbumForIndex(const QModelIndex& index) const;

private Q_SLOTS:

    void slotUpdateTrashCounters();

protected:

    QVariant albumData(Album* a, int role)          const override;
    QVariant decorationRoleData(Album* a)           const override;
    Album*   albumForId(int id)                     const override;
};

// ------------------------------------------------------------------

class DIGIKAM_GUI_EXPORT TagModel : public AbstractCheckableAlbumModel
{
    Q_OBJECT

public:

    /**
     * Create a model containing all tags
     */
    explicit TagModel(RootAlbumBehavior rootBehavior = IncludeRootAlbum, QObject* const parent = nullptr);

    TAlbum*      talbumForIndex(const QModelIndex& index) const;
    void         setColumnHeader(const QString& header)        override;
    void         activateFaceTagModel();

protected:

    QVariant     albumData(Album* a, int role)           const override;
    QVariant     decorationRoleData(Album* a)            const override;
    Album*       albumForId(int id)                      const override;
    QVariant     fontRoleData(Album* a)                  const override;
    bool         setData(const QModelIndex& index,
                         const QVariant& value,
                         int role = Qt::EditRole)              override;

private:

    QHash<int, int> m_unconfirmedFaceCount;
};

// ------------------------------------------------------------------

class DIGIKAM_GUI_EXPORT SearchModel : public AbstractCheckableAlbumModel
{
    Q_OBJECT

public:

    /**
     * Create a model containing searches
     */
    explicit SearchModel(QObject* const parent = nullptr);

    SAlbum* salbumForIndex(const QModelIndex& index) const;

    /**
     * Set a hash of internal names (key) that shall be replaced by a user-visible string (value).
     * This affects Qt::DisplayRole and AlbumTitleRole.
     */
    void setReplaceNames(const QHash<QString, QString>& replaceNames);
    void addReplaceName(const QString& technicalName, const QString& userVisibleName);

    /**
     * Set pixmaps for the DecorationRole
     */
    void setPixmapForNormalSearches(const QPixmap& pix);
    void setDefaultPixmap(const QPixmap& pix);
    void setPixmapForTimelineSearches(const QPixmap& pix);
    void setPixmapForHaarSearches(const QPixmap& pix);
    void setPixmapForMapSearches(const QPixmap& pix);
    void setPixmapForDuplicatesSearches(const QPixmap& pix);

protected:

    QVariant albumData(Album* a, int role)          const override;
    Album*   albumForId(int id)                     const override;

private Q_SLOTS:

    void albumSettingsChanged();

protected:

    QHash<int, QPixmap>     m_pixmaps;
    QHash<QString, QString> m_replaceNames;
};

// ------------------------------------------------------------------

/**
 * A model for date based albums.
 */
class DIGIKAM_GUI_EXPORT DateAlbumModel : public AbstractCountingAlbumModel
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent the parent for Qt's parent child mechanism
     */
    explicit DateAlbumModel(QObject* const parent = nullptr);

    DAlbum* dalbumForIndex(const QModelIndex& index)     const;

    /**
     * Finds an album index based on a date. The given date is therefore
     * normalized to year-month-form. The day is ignored. This means the
     * returned index always points to a month DAlbum.
     *
     * @param date the date to search for (year and month)
     * @return model index corresponding to the album with the given date or an
     *         empty index if not found
     */
    QModelIndex monthIndexForDate(const QDate& date)    const;

    /**
     * Set pixmaps for the DecorationRole
     */
    void setPixmaps(const QPixmap& forYearAlbums,
                    const QPixmap& forMonthAlbums);

public Q_SLOTS:

    void setYearMonthMap(const QMap<YearMonth, int>& yearMonthMap);

protected:

    QString  albumName(Album* a)                        const override;
    QVariant decorationRoleData(Album* a)               const override;
    QVariant sortRoleData(Album* a)                     const override;
    Album*   albumForId(int id)                         const override;

protected:

    QPixmap m_yearPixmap;
    QPixmap m_monthPixmap;
};

} // namespace Digikam
