/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-15
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2010      by Jens Mueller <tschenser at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>
#include <QString>
#include <QUrl>
#include <QNetworkReply>

// Local includes

#include "dinfointerface.h"
#include "thumbnailloadthread.h"

using namespace Digikam;

namespace DigikamGenericGoogleServicesPlugin
{

enum ReplaceDialog_Result
{
    PWR_CANCEL      = 0,
    PWR_ADD         = 1,
    PWR_ADD_ALL     = 2,
    PWR_REPLACE     = 3,
    PWR_REPLACE_ALL = 4
};

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:

    ReplaceDialog(QWidget* const parent,
                  const QString& caption,
                  DInfoInterface* const iface,
                  const QUrl& src,
                  const QUrl& dest);
    ~ReplaceDialog() override;

    int getResult();

public Q_SLOTS:

    void cancelPressed();
    void addPressed();
    void addAllPressed();
    void replacePressed();
    void replaceAllPressed();

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);
    void slotThumbnail(const LoadingDescription&, const QPixmap&);
    void slotProgressTimerDone();

private:

    QPixmap setProgressAnimation(const QPixmap& thumb, const QPixmap& pix);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericGoogleServicesPlugin
