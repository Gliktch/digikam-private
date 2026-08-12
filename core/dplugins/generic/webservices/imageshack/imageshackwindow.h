/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * SPDX-FileCopyrightText: 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2025 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QWidget>

// Local includes

#include "wstooldialog.h"
#include "ditemslist.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericImageShackPlugin
{

class ImageShackWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit ImageShackWindow(DInfoInterface* const iface, QWidget* const parent);
    ~ImageShackWindow() override;

public:

    DItemsList* getImagesList() const;

Q_SIGNALS:

    void signalBusy(bool val);

private Q_SLOTS:

    void slotImageListChanged();
    void slotStartTransfer();
    void slotBusy(bool val);
    void slotFinished();
    void slotCancelClicked();

    void slotJobInProgress(int step, int, const QString& format);
    void slotLoginDone(int errCode, const QString& errMsg);
    void slotGetGalleriesDone(int errCode, const QString& errMsg);
    void slotGetGalleries();

    void slotAddPhotoDone(int errCode, const QString& errMsg);

    void authenticate();

private:

    void readSettings();
    void saveSettings();

    void authenticationDone(int errCode, const QString& errMsg);

    void uploadNextItem();

    void closeEvent(QCloseEvent* e) override;

private Q_SLOTS:

    void slotChangeRegistrantionCode();
    void slotNewAlbumRequest();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericImageShackPlugin
