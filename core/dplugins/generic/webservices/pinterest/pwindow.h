/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Pinterest web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QUrl>
#include <QPair>

// Local includes

#include "wstooldialog.h"
#include "dinfointerface.h"

class QCloseEvent;

using namespace Digikam;

namespace DigikamGenericPinterestPlugin
{

class PWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit PWindow(DInfoInterface* const iface, QWidget* const parent);
    ~PWindow()                      override;

    void reactivate();

    void setItemsList(const QList<QUrl>& urls);

private:

    void readSettings();
    void writeSettings();

    void uploadNextPhoto();

    void buttonStateChange(bool state);
    void closeEvent(QCloseEvent*)   override;

private Q_SLOTS:

    void slotImageListChanged();
    void slotUserChangeRequest();
    void slotNewBoardRequest();
    void slotReloadBoardsRequest();
    void slotStartTransfer();

    void slotBusy(bool);
    void slotSignalLinkingFailed();
    void slotSignalLinkingSucceeded();
    void slotSetUserName(const QString& msg);
    void slotListBoardsFailed(const QString& msg);
    void slotListBoardsDone(const QList<QPair<QString, QString> >& list);
    void slotCreateBoardFailed(const QString& msg);
    void slotCreateBoardSucceeded();
    void slotAddPinFailed(const QString& msg);
    void slotAddPinSucceeded();
    void slotTransferCancel();

    void slotFinished();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPinterestPlugin
