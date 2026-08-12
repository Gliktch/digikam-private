/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Showfoto stack view favorites item
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QDate>
#include <QList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace ShowFoto
{

class ShowfotoStackViewFavoriteItem : public QTreeWidgetItem
{

public:

    enum FavoriteType
    {
        FavoriteRoot = -1,      ///< Favorite is root item from hierarchy.
        FavoriteFolder,         ///< Favorite is a simple folder in hierarchy.
        FavoriteItem            ///< Favorite is a hierarchy item including all properties.
    };

public:

    explicit ShowfotoStackViewFavoriteItem(QTreeWidget* const parent);
    ShowfotoStackViewFavoriteItem(QTreeWidgetItem* const parent, int favType);
    ~ShowfotoStackViewFavoriteItem() override;

    void setName(const QString& name);
    QString name()            const;

    void setHierarchy(const QString& desc);
    QString hierarchy()       const;

    void setFavoriteType(int favoriteType);
    int favoriteType()        const;

    void setDescription(const QString& desc);
    QString description()     const;

    void setDate(const QDate& date);
    QDate date()              const;

    void setUrls(const QList<QUrl>&);
    QList<QUrl> urls()        const;

    void setCurrentUrl(const QUrl& url);
    QUrl currentUrl()         const;

    /**
     * @brief helper method to get a list local paths from image urls included in favorite item.
     */
    QStringList urlsToPaths() const;

public:

    /**
     * @brief helper static method to get hierarchy path from item. @param name is the title and @param pitem the parent instance.
     */
    static QString hierarchyFromParent(const QString& name, ShowfotoStackViewFavoriteItem* const pitem);

private:

    void updateToolTip();
    Q_DISABLE_COPY(ShowfotoStackViewFavoriteItem)

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
