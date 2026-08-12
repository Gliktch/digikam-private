/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-24
 * Description : DNG converter batch dialog
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "dplugindialog.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericDNGConverterPlugin
{

class DNGConverterActionData;

class DNGConverterDialog : public DPluginDialog
{

    Q_OBJECT

public:

    explicit DNGConverterDialog(QWidget* const parent, DInfoInterface* const iface);
    ~DNGConverterDialog() override;

   void addItems(const QList<QUrl>& itemList);

protected:

    void closeEvent(QCloseEvent* e);

private:

    void readSettings();
    void saveSettings();

    void setBusy(bool busy);

    void processAll();
    void processed(const QUrl& url, const QString& tmpFile);
    void processingFailed(const QUrl& url, int result);

private Q_SLOTS:

    void slotDefault();
    void slotClose();
    void slotStartStop();
    void slotAborted();
    void slotThreadFinished();

    /**
     * Set Identity and Target file.
     */
    void slotIdentify();

    void slotDNGConverterAction(const DigikamGenericDNGConverterPlugin::DNGConverterActionData&);

    void slotSetupExifTool();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericDNGConverterPlugin
