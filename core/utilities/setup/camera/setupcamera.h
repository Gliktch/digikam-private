/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : camera setup tab.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>
#include <QString>

// Local includes

#include "dbusydlg.h"

namespace Digikam
{

class SetupCamera : public QScrollArea
{
    Q_OBJECT

public:

    enum CameraTab
    {
        Devices = 0,
        Behavior,
        ImportFilters,
        ImportWindow
    };

    enum ConflictRule
    {
        OVERWRITE = 0,
        DIFFNAME,
        SKIPFILE
    };

public:

    explicit SetupCamera(QWidget* const parent = nullptr);
    ~SetupCamera() override;

    void applySettings();
    bool checkSettings();

    bool useFileMetadata();

    void setActiveTab(CameraTab tab);
    CameraTab activeTab() const;

Q_SIGNALS:

    void signalUseFileMetadataChanged(bool);

private Q_SLOTS:

    void slotSelectionChanged();

    void slotAddCamera();
    void slotRemoveCamera();
    void slotEditCamera();
    void slotAutoDetectCamera();

    void slotAddedCamera(const QString& title, const QString& model,
                         const QString& port,  const QString& path);
    void slotEditedCamera(const QString& title, const QString& model,
                          const QString& port,  const QString& path);

    void slotImportSelectionChanged();
    void slotAddFilter();
    void slotRemoveFilter();
    void slotEditFilter();
    void slotResetFilter();
    void slotPreviewItemsClicked();
    void slotPreviewFullImageSizeClicked();

private:

    void readSettings();

private:

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------

class CameraAutoDetectThread : public DBusyThread
{
    Q_OBJECT

public:

    explicit CameraAutoDetectThread(QObject* const parent);
    ~CameraAutoDetectThread() override;

    int     result()    const;
    QString model()     const;
    QString port()      const;

private:

    void run()                override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
