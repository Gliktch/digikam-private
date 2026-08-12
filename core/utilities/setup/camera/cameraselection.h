/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : Camera type selection dialog
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
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

// Local includes

#include "searchtextbar.h"

class QTreeWidgetItem;

namespace Digikam
{

class CameraSelection : public QDialog
{
    Q_OBJECT

public:

    explicit CameraSelection(QWidget* const parent = nullptr);
    ~CameraSelection() override;

    void setCamera(const QString& title, const QString& model,
                   const QString& port,  const QString& path);

    QString currentTitle()      const;
    QString currentModel()      const;
    QString currentPortPath()   const;
    QString currentCameraPath() const;

Q_SIGNALS:

    void signalOkClicked(const QString& title, const QString& model,
                         const QString& port,  const QString& path);

private:

    void getCameraList();
    void getSerialPortList();

private Q_SLOTS:

    void slotHelp();
    void slotUMSCameraLinkUsed();
    void slotPTPCameraLinkUsed();
    void slotPTPIPCameraLinkUsed();
    void slotNetworkEditChanged(const QString& text);
    void slotSelectionChanged(QTreeWidgetItem*, int);
    void slotPortChanged();
    void slotOkClicked();
    void slotSearchTextChanged(const SearchTextSettings&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
