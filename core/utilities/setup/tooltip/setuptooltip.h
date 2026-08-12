/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : item tool tip configuration setup tab
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupToolTip : public QScrollArea
{
    Q_OBJECT

public:

    enum ToolTipTab
    {
        IconItems = 0,
        AlbumItems,
        ImportItems
    };

public:

    explicit SetupToolTip(QWidget* const parent = nullptr);
    ~SetupToolTip() override;

    void setActiveTab(ToolTipTab tab);
    ToolTipTab activeTab() const;

    void applySettings();

public Q_SLOTS:

    void slotUseFileMetadataChanged(bool);

private Q_SLOTS:

    void slotImportToolTipsChanged();

private:

    void setupIcon();
    void setupAlbum();
    void setupImport();

    void readSettings();
    void refreshCameraOptions();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
