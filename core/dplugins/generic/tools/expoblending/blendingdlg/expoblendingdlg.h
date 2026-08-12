/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QPixmap>
#include <QDialog>
#include <QDialogButtonBox>

// Local includes

#include "expoblendingactions.h"

class QCloseEvent;

namespace DigikamGenericExpoBlendingPlugin
{

class ExpoBlendingManager;
class ExpoBlendingActionData;

class ExpoBlendingDlg : public QDialog
{
    Q_OBJECT

public:

    explicit ExpoBlendingDlg(ExpoBlendingManager* const mngr,
                             QWidget* const parent = nullptr);
    ~ExpoBlendingDlg()              override;

    void loadItems(const QList<QUrl>& urls);

Q_SIGNALS:

    void cancelClicked();

private:

    void closeEvent(QCloseEvent*)   override;

    void setRejectButtonMode(QDialogButtonBox::StandardButton button);

    void readSettings();
    void saveSettings();

    void busy(bool busy);
    void saveItem(const QUrl& temp, const EnfuseSettings& settings);

    void setIdentity(const QUrl& url, const QString& identity);

private Q_SLOTS:

    void slotCloseClicked();
    void slotDefault();
    void slotPreview();
    void slotProcess();
    void slotCancelClicked();
    void slotFinished();

    void slotLoadProcessed(const QUrl&);
    void slotExpoBlendingAction(const DigikamGenericExpoBlendingPlugin::ExpoBlendingActionData&);
    void slotAddItems(const QList<QUrl>& urls);
    void slotItemClicked(const QUrl& url);

    void slotPreviewButtonClicked();
    void slotFileFormatChanged();

protected:

    void showEvent(QShowEvent*)     override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericExpoBlendingPlugin
