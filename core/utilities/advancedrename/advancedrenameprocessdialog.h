/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-11-14
 * Description : process dialog for renaming files
 *
 * SPDX-FileCopyrightText: 2010-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "advancedrenamedialog.h"
#include "dprogressdlg.h"
#include "digikam_export.h"

class QCloseEvent;
class QPixmap;
class QUrl;

namespace Digikam
{

class LoadingDescription;

class DIGIKAM_GUI_EXPORT AdvancedRenameProcessDialog : public DProgressDlg
{
    Q_OBJECT

public:

    explicit AdvancedRenameProcessDialog(const NewNamesList& list,
                                         QWidget* const parent = nullptr);
    ~AdvancedRenameProcessDialog() override;

    QList<QUrl> failedUrls() const;

private:

    void abort();
    void complete();
    void processOne();
    void getNextThumbnail();

protected:

    void closeEvent(QCloseEvent* e) override;

protected Q_SLOTS:

    void slotCancel()               override;
    void slotRenameFinished();
    void slotRenameFailed(const QUrl& url);

private Q_SLOTS:

    void slotRenameImages();
    void slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix);

private:

    /// @note disabled
    AdvancedRenameProcessDialog(const AdvancedRenameProcessDialog&)            = delete;
    AdvancedRenameProcessDialog& operator=(const AdvancedRenameProcessDialog&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
