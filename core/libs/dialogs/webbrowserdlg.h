/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-21
 * Description : a simple web browser dialog based on Qt WebEngine.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt include

#include <QDialog>
#include <QWidget>
#include <QUrl>
#include <QString>
#include <QCloseEvent>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class SearchTextSettings;

class DIGIKAM_EXPORT WebBrowserDlg : public QDialog
{
    Q_OBJECT

public:

    explicit WebBrowserDlg(const QUrl& url,
                           QWidget* const parent,
                           bool hideDeskBrowser = false);
    ~WebBrowserDlg()                override;

Q_SIGNALS:

    void urlChanged(const QUrl& url);
    void closeView(bool val);

private Q_SLOTS:

    void slotUrlChanged(const QUrl&);
    void slotLoadingStarted();
    void slotLoadingFinished(bool);
    void slotTitleChanged(const QString&);
    void slotSearchTextChanged(const SearchTextSettings&);
    void slotGoHome();
    void slotDesktopWebBrowser();

protected:

    void showEvent(QShowEvent*)     override;
    void closeEvent(QCloseEvent*)   override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
