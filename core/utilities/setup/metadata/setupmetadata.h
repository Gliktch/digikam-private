/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 *
 * SPDX-FileCopyrightText: 2003-2004 by Ralf Holzer <ralf at well.com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupMetadata : public QScrollArea
{
    Q_OBJECT

public:

    enum MetadataTab
    {
        Behavior = 0,
        Sidecars,
        Rotation,
        Display,
        ExifTool,
        Baloo,
        AdvancedConfig
    };

    enum MetadataSubTab
    {
        ExifViewer = 0,
        MakernotesViewer,
        IPTCViewer,
        XMPViewer,
        ExifToolViewer
    };

public:

    explicit SetupMetadata(QWidget* const parent = nullptr);
    ~SetupMetadata() override;

    void applySettings();

    bool exifAutoRotateHasChanged() const;

    void setActiveTab(MetadataTab tab);
    void setActiveSubTab(MetadataSubTab tab);

    MetadataTab activeTab()         const;
    MetadataSubTab activeSubTab()   const;

private:

    void appendBehaviorTab();
    void appendSidecarsTab();
    void appendDisplayTab();
    void appendRotationTab();
    void appendBalooTab();

private Q_SLOTS:

    void slotSidecarReadWriteToggled();
    void slotSidecarFileNameToggled(bool);
    void slotExifAutoRotateToggled(bool);
    void slotWriteWithExifToolToggled(bool);
    void slotExifToolSettingsChanged(bool);
    void slotWriteRawFilesToggled(bool);
    void slotExtensionsEdited(const QString& text);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
