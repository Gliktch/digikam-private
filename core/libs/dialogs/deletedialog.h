/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-07
 * Description : a dialog to delete item.
 *
 * SPDX-FileCopyrightText: 2004      by Michael Pyne <michael dot pyne at kdemail dot net>
 * SPDX-FileCopyrightText: 2006      by Ian Monroe <ian at monroe dot nu>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QTreeWidget>
#include <QUrl>
#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

namespace DeleteDialogMode
{

enum ListMode
{
    Files,
    Albums,
    Subalbums
};

enum DeleteMode
{
    NoChoiceTrash,             ///< "Do not show again" checkbox, does not show if config entry is set
    NoChoiceDeletePermanently, ///< Same as above
    UserPreference,            ///< Checkbox to toggle trash/permanent, preset to user's last preference
    UseTrash,                  ///< Same checkbox as above, preset to trash
    DeletePermanently          ///< Same checkbox as above, preset to permanent
};

} // namespace DeleteDialogMode

// -----------------------------------------------------------

class DeleteItem : public QTreeWidgetItem
{

public:

    explicit DeleteItem(QTreeWidget* const parent, const QUrl& url);
    ~DeleteItem()                     override;

    bool    hasValidThumbnail() const;
    QUrl    url()               const;
    QString fileUrl()           const;

    void setThumb(const QPixmap& pix, bool hasThumb = true);

private:

    Q_DISABLE_COPY(DeleteItem)

private:

    class Private;
    Private* const d = nullptr;
};

// -----------------------------------------------------------

class DeleteItemList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit DeleteItemList(QWidget* const parent = nullptr);
    ~DeleteItemList()                            override;

private :

    void drawRow(QPainter* p,
                 const QStyleOptionViewItem& opt,
                 const QModelIndex& index) const override;

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    class Private;
    Private* const d = nullptr;
};

// -----------------------------------------------------------

class DeleteWidget : public QWidget
{
    Q_OBJECT

public:

    explicit DeleteWidget(QWidget* const parent = nullptr);
    ~DeleteWidget() override;

    void setUrls(const QList<QUrl>& urls);
    void setListMode(DeleteDialogMode::ListMode mode);
    void setDeleteMode(DeleteDialogMode::DeleteMode deleteMode);

private Q_SLOTS:

    void slotShouldDelete(bool);

private:

    void updateText();

private:

    class Private;
    Private* const d = nullptr;

    friend class DeleteDialog;
};

// -----------------------------------------------------------

class DeleteDialog : public QDialog
{
    Q_OBJECT

public:

    enum Mode
    {
        ModeFiles,
        ModeAlbums,
        ModeSubalbums
    };

public:

    explicit DeleteDialog(QWidget* const parent);
    ~DeleteDialog()                 override;

    bool confirmDeleteList(const QList<QUrl>& condemnedURLs,
                           DeleteDialogMode::ListMode listMode,
                           DeleteDialogMode::DeleteMode deleteMode);
    bool shouldDelete()        const;

    void setUrls(const QList<QUrl>& urls);
    void presetDeleteMode(DeleteDialogMode::DeleteMode mode);
    void setListMode(DeleteDialogMode::ListMode mode);

protected Q_SLOTS:

    void slotUser1Clicked();
    void slotShouldDelete(bool);

private Q_SLOTS:

    void slotHelp();

protected:

    void keyPressEvent(QKeyEvent*)  override;
    void showEvent(QShowEvent*)     override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
