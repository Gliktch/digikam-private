/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to IPFS web service
 *
 * SPDX-FileCopyrightText: 2018      by Amar Lakshya <amar dot lakshya at xaviers dot edu dot in>
 * SPDX-FileCopyrightText: 2018-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QLabel>

// Local includes

#include "ipfsimageslist.h"
#include "ipfstalker.h"
#include "wstooldialog.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericIpfsPlugin
{

class IpfsWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit IpfsWindow(DInfoInterface* const iface, QWidget* const parent = nullptr);
    ~IpfsWindow()                       override;

    void reactivate();

public Q_SLOTS:

    // UI callbacks

    void slotUpload();
    void slotFinished();
    void slotCancel();

    // IpfsTalker callbacks

/*
     void apiAuthorized(bool success, const QString& username);
     void apiAuthError(const QString& msg);
*/
    void apiProgress(unsigned int percent, const IpfsTalkerAction& action);
    void apiRequestPin(const QUrl& url);
    void apiSuccess(const IpfsTalkerResult& result);
    void apiError(const QString& msg, const IpfsTalkerAction& action);
    void apiBusy(bool busy);

private:

    void closeEvent(QCloseEvent* e)     override;
    void setContinueUpload(bool state);
    void readSettings();
    void saveSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericIpfsPlugin
