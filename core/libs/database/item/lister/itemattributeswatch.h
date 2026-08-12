/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-05-04
 * Description : Watch image attributes
 *
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QUrl>

// Local includes

#include "coredbchangesets.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ItemAttributesWatch : public QObject
{
    Q_OBJECT

public:

    static ItemAttributesWatch* instance();
    static void cleanUp();
    static void shutDown();

/*
    void imageTagsChanged(qint64 imageId);
    void imagesChanged(int albumId);

    void imageRatingChanged(qint64 imageId);
    void imageDateChanged(qint64 imageId);
    void imageCaptionChanged(qint64 imageId);
*/

    void fileMetadataChanged(const QUrl& url);

Q_SIGNALS:

    /**
     * Indicates that tags have been assigned or removed
     * for image with given imageId.
     * There is no guarantee that the tags were actually changed.
     * This signal, the signal below, or both may be sent.
     */
    void signalImageTagsChanged(qlonglong imageId);

    /**
     * Indicates that images in the given album id may have changed their tags.
     * This signal, the signal above, or both may be sent.
     */
    void signalImagesChanged(int albumId);

    /**
     * These signals indicated that the rating, data or caption
     * of the image with given imageId was set.
     * There is no guarantee that it actually changed.
     */
    void signalImageRatingChanged(qlonglong imageId);
    void signalImageDateChanged(qlonglong imageId);
    void signalImageCaptionChanged(qlonglong imageId);

    /**
     * Indicates that the metadata if the given file
     * has been changed (a write operation on the file on disk).
     * Usually, the database is updated accordingly, so then this
     * signal is sent in combination with one or more of the above signals.
     */
    void signalFileMetadataChanged(const QUrl& url);

private Q_SLOTS:

    void slotImageChange(const ImageChangeset& changeset);
    void slotImageTagChange(const ImageTagChangeset& changeset);

private:

    /// @note disabled
    ItemAttributesWatch();
    explicit ItemAttributesWatch(QObject*) = delete;
    ~ItemAttributesWatch() override;

    static ItemAttributesWatch* m_instance;
};

} // namespace Digikam
