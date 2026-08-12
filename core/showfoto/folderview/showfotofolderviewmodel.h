/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Showfoto folder view model.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// QT includes

#include <QFileSystemModel>
#include <QStringList>

// Local includes

#include "loadingdescription.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoFolderViewList;

class ShowfotoFolderViewModel : public QFileSystemModel
{
    Q_OBJECT

public:

    explicit ShowfotoFolderViewModel(ShowfotoFolderViewList* const view);
    ~ShowfotoFolderViewModel()                              override;

    static int maxIconSize();

    QVariant data(const QModelIndex& index, int role) const override;

    /**
     * List all file paths from the current model root index selected in the view.
     */
    QStringList currentFilesPath()                    const;

private Q_SLOTS:

    void refreshThumbnails(const LoadingDescription& desc, const QPixmap& pix);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
