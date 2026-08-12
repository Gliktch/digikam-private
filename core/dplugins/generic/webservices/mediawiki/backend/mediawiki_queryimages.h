/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Ludovic Delfau <ludovicdelfau at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// KDE includes

#include <kjob.h>

// Local includes

#include "mediawiki_job.h"
#include "mediawiki_image.h"

namespace MediaWiki
{

class Iface;
class QueryImagesPrivate;

/**
 * @brief Query images job.
 *
 * Gets a list of all images used on pages.
 */
class QueryImages : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QueryImages)

public:

    /**
     * @brief Indicates all possible error conditions found during the processing of the job.
     */
    enum
    {
        /**
         * @brief A network error has occurred.
         */
        NetworkError = KJob::UserDefinedError + 1,

        /**
         * @brief A XML error has occurred.
         */
        XmlError
    };

public:

    /**
     * @brief Constructs a query images job.
     * @param MediaWiki the MediaWiki concerned by the job
     * @param parent the QObject parent
     */
    explicit QueryImages(Iface& MediaWiki, QObject* const parent = nullptr);

    /**
     * @brief Destroys a query images job.
     */
    ~QueryImages() override = default;

    /**
     * @brief Set the title.
     * @param title the title of the page
     */
    void setTitle(const QString& title);

    /**
     * @brief Set the limit.
     * @param limit number of images returned by one signal #pages()
     */
    void setLimit(unsigned int limit);

    /**
     * @brief Starts the job asynchronously.
     */
    void start() override;

Q_SIGNALS:

    /**
     * @brief Provides a list of all images used on pages
     *
     * This signal can be emitted several times.
     *
     * @param pages list of all images used on pages
     */
    void images(const QList<Image>& images);

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

} // namespace MediaWiki
