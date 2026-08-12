/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to Imgur web service
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marius Orcsik <marius at habarnam dot ro>
 * SPDX-FileCopyrightText: 2013-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QLabel>
#include <QList>
#include <QUrl>

// Local includes

#include "imgurimageslist.h"
#include "wstooldialog.h"
#include "imgurtalker.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericImgUrPlugin
{

class ImgurWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit ImgurWindow(DInfoInterface* const iface, QWidget* const parent = nullptr);
    ~ImgurWindow()                      override;

public:

    void reactivate();

    void setItemsList(const QList<QUrl>& urls);

public Q_SLOTS:

    // UI callbacks

    void slotForgetButtonClicked();
    void slotUpload();
    void slotAnonUpload();
    void slotFinished();
    void slotCancel();

    // ImgurTalker callbacks

    void slotApiAuthorized(bool success, const QString& userName);
    void slotApiAuthError(const QString& msg);
    void slotApiProgress(unsigned int percent, const ImgurTalkerAction& action);
    void slotApiRequestPin(const QUrl& url);
    void slotApiSuccess(const ImgurTalkerResult& result);
    void slotApiError(const QString& msg, const ImgurTalkerAction& action);
    void slotApiBusy(bool busy);

private:

    void closeEvent(QCloseEvent* e)     override;
    void setContinueUpload(bool state);
    void readSettings();
    void saveSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericImgUrPlugin
