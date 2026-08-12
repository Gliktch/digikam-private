/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Album properties dialog.
 *
 * SPDX-FileCopyrightText: 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005      by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QStringList>
#include <QDialog>

namespace Digikam
{

class PAlbum;

class AlbumPropsEdit : public QDialog
{
    Q_OBJECT

public:

    explicit AlbumPropsEdit(PAlbum* const album, bool create = false);
    ~AlbumPropsEdit()                   override;

    QString     title()           const;
    QString     comments()        const;
    QDate       date()            const;
    int         parent()          const;
    QString     category()        const;
    QStringList albumCategories() const;

public:

    static bool editProps(PAlbum* const album,
                          QString&      title,
                          QString&      comments,
                          QDate&        date,
                          QString&      category,
                          QStringList&  albumCategories);

    static bool createNew(PAlbum* const parent,
                          QString&      title,
                          QString&      comments,
                          QDate&        date,
                          QString&      category,
                          QStringList&  albumCategories,
                          int&          parentSelector);

private Q_SLOTS:

    void slotTitleChanged();
    void slotNewAlbumTextChanged(int index);
    void slotDateLowButtonClicked();
    void slotDateAverageButtonClicked();
    void slotDateHighButtonClicked();
    void slotHelp();

private:

    /// @note disabled
    explicit AlbumPropsEdit(QWidget*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
