/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * SPDX-FileCopyrightText: 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupAlbumView : public QScrollArea
{
    Q_OBJECT

public:

    enum AlbumTab
    {
        IconView = 0,
        FolderView,
        Preview,
        FullScreen,
        MimeType,
        Category
    };

public:

    explicit SetupAlbumView(QWidget* const parent = nullptr);
    ~SetupAlbumView() override;

    void setActiveTab(AlbumTab tab);
    AlbumTab activeTab() const;

    void applySettings();

    bool useLargeThumbsHasChanged() const;

private:

    void setupIconView();
    void setupTreeView();
    void setupPreView();
    void readSettings();

private Q_SLOTS:

    void slotUseLargeThumbsToggled(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
