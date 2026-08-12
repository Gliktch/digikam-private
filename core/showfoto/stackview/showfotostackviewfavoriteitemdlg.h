/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Showfoto stack view favorite item edit dialog
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDate>
#include <QDialog>
#include <QList>
#include <QUrl>

// Local includes

#include "digikam_config.h"
#include "ditemslist.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoStackViewFavoriteList;
class ShowfotoStackViewFavoriteItem;

class ShowfotoStackViewFavoriteItemDlg : public QDialog
{
    Q_OBJECT

public:

    explicit ShowfotoStackViewFavoriteItemDlg(ShowfotoStackViewFavoriteList* const list,
                                              bool create = false);
    ~ShowfotoStackViewFavoriteItemDlg()   override;

    void setName(const QString& name);
    QString name()                  const;

    void setFavoriteType(int favoriteType);
    int favoriteType()              const;

    void setDescription(const QString& desc);
    QString description()           const;

    void setDate(const QDate& name);
    QDate date()                    const;

    void setIcon(const QString& icon);
    QString icon()                  const;

    void setUrls(const QList<QUrl>& urls);
    QList<QUrl> urls()              const;

    void setCurrentUrl(const QUrl& url);
    QUrl currentUrl()               const;

    void setIconSize(int size);
    void setSortOrder(int order);
    void setSortRole(int role);
    void setParentItem(ShowfotoStackViewFavoriteItem* const pitem);

public:

    static bool favoriteItemDialog(ShowfotoStackViewFavoriteList* const list,
                                   QString& name,
                                   int& favoriteType,
                                   QString& desc,
                                   QDate& date,
                                   QString& icon,
                                   QList<QUrl>& urls,
                                   QUrl& current,
                                   int iconSize,
                                   int sortOrder,
                                   int sortRole,
                                   ShowfotoStackViewFavoriteItem* const pitem,
                                   bool create = false);

private:

    QList<QDate> getItemDates()     const;

    static bool itemIsLessThanHandler(const QTreeWidgetItem* current,
                                      const QTreeWidgetItem& other);

private Q_SLOTS:

    void slotIconChanged();
    void slotIconResetClicked();
    void slotModified();
    void slotUpdateMetadata();
    void slotTypeActivated();
    void slotDateLowButtonClicked();
    void slotDateAverageButtonClicked();
    void slotDateHighButtonClicked();
    void slotHelp();

private:

    bool canAccept()                const;

protected:

    void showEvent(QShowEvent*) override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
