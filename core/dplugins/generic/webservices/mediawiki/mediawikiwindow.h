/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-11
 * Description : a tool to export images to WikiMedia web service
 *
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Parthasarathy Gopavarapu <gparthasarathy93 at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Peter Potrowl <peter dot potrowl at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "wstooldialog.h"
#include "dinfointerface.h"

class QCloseEvent;

class KJob;

using namespace Digikam;

namespace DigikamGenericMediaWikiPlugin
{

class MediaWikiWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit MediaWikiWindow(DInfoInterface* const iface, QWidget* const parent);
    ~MediaWikiWindow()                            override;

public:

    void reactivate();
    bool prepareImageForUpload(const QString& imgPath);

private Q_SLOTS:

    void slotFinished();
    void slotProgressCanceled();
    void slotStartTransfer();
    void slotChangeUserClicked();
    void slotDoLogin(const QString& login,
                     const QString& pass,
                     const QString& wikiName,
                     const QUrl& wikiUrl);
    void slotEndUpload();
    int  slotLoginHandle(KJob* loginJob);

private:

    bool eventFilter(QObject* obj, QEvent* event) override;
    void closeEvent(QCloseEvent*)                 override;
    void readSettings();
    void saveSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMediaWikiPlugin
