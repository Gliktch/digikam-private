/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-08-05
 * Description : Plasma file indexer interface.
 *
 * SPDX-FileCopyrightText: 2014 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QStringList>
#include <QPointer>

// Local includes

#include "digikam_export.h"

class QUrl;

namespace Digikam
{

class ItemInfo;

class BalooInfo
{
public:

    BalooInfo() = default;

    QStringList tags;
    QString     comment;
    int         rating = -1;
};

/**
 * @brief The BalooWrap class is a singleton class which offer
 *        functionality for reading and writing image
 *        comment, tags and rating from Baloo to digiKam
 *        and from digiKam to Baloo
 *
 *        The singleton functionality is required because
 *        it also watches for changes in Baloo and notify
 *        digiKam, so it could trigger a scan
 */
class DIGIKAM_EXPORT BalooWrap : public QObject
{
    Q_OBJECT

public:

    BalooWrap();
    ~BalooWrap() override;

    /**
     * @brief internalPtr - singleton implementation
     */
    static QPointer<BalooWrap> internalPtr;
    static BalooWrap*          instance();
    static bool                isCreated();

    /**
     * @brief setSemanticInfo - generic method to set all data from digiKam to Baloo
     * @param url             - image url
     * @param bInfo           - container class for tags, comment, rating
     */
    void setSemanticInfo(const QUrl& url, const BalooInfo& bInfo);

    /**
     * @brief getSemanticInfo - used by ItemScanner to retrieve all information
     *                          tags, comment, rating
     * @param url             - image url
     * @return                - container class for tags, comment, rating
     */
    BalooInfo getSemanticInfo(const QUrl& url) const;

    void setSyncToBaloo(bool value);

    bool getSyncToBaloo()                      const;

    void setSyncToDigikam(bool value);

    bool getSyncToDigikam()                    const;

private:

    /// @note disabled
    explicit BalooWrap(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
