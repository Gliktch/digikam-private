/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-18
 * Description : setup Metadata tab.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace ShowFoto
{

class ShowfotoSetupMetadata : public QScrollArea
{
    Q_OBJECT

public:

    enum MetadataTab
    {
        Behavior = 0,
        ExifViewer,
        MakernotesViewer,
        IptcViewer,
        XmpViewer,
        ExifTool
    };

public:

    explicit ShowfotoSetupMetadata(QWidget* const parent = nullptr);
    ~ShowfotoSetupMetadata()      override;

    void applySettings();

    void setActiveTab(MetadataTab tab);
    MetadataTab activeTab() const;

private:

    void readSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
