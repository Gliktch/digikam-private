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
 * SPDX-FileCopyrightText: 2011      by Paolo de Vathaire <paolo dot devathaire at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

namespace MediaWiki
{

/**
 * @brief A image.
 */
class Image
{
public:

    /**
     * @brief Constructs a image.
     */
    Image();

    /**
     * @brief Constructs an image from an other image.
     * @param other an other image
     */
    Image(const Image& other);

    /**
     * @brief Destructs an image.
     */
    ~Image();

    /**
     * @brief Assigning an image from an other image.
     * @param other an other image
     */
    Image& operator=(const Image& other);

    /**
     * @brief Returns true if this instance and other are equal, else false.
     * @param other instance to compare
     * @return true if there are equal, else false
     */
    bool operator==(const Image& other)     const;

    /**
     * @brief Returns the namespace id of the image.
     * @return the namespace id of the image
     */
    qint64 namespaceId()                    const;

    /**
     * @brief Set the namespace id.
     * @param namespaceId the namespace id of the image
     */
    void setNamespaceId(qint64 namespaceId);

    /**
     * @brief Returns the title of the image.
     * @return the title of the image
     */
    QString title()                         const;

    /**
     * @brief Set the title.
     * @param title the title of the image
     */
    void setTitle(const QString& title);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace MediaWiki
