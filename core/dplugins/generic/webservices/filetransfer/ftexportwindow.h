/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-28
 * Description : a tool to export image to a KIO accessible location
 *
 * SPDX-FileCopyrightText: 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "wstooldialog.h"
#include "dinfointerface.h"

class KJob;

namespace KIO
{
    class Job;
}

using namespace Digikam;

namespace DigikamGenericFileTransferPlugin
{

class FTExportWidget;

/**
 * Main window of the KIOExport tool.
 */

class FTExportWindow: public WSToolDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent the parent QWidget
     */
    explicit FTExportWindow(DInfoInterface* const iface, QWidget* const parent);

    /**
     * Destructor.
     */
    ~FTExportWindow() override;

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

private Q_SLOTS:

    /**
     * Processes changes on the image list.
     */
    void slotImageListChanged();

    /**
     * Starts uploading the selected images.
     */
    void slotUpload();

    /**
     * Processes changes in the target url.
     */
    void slotTargetUrlChanged(const QUrl& target);

    /**
     * Removes the copied image from the image list.
     */
    void slotCopyingDone(KIO::Job* job, const QUrl& from, const QUrl& to,
                         const QDateTime& mtime, bool directory, bool renamed);

    /**
     * Re-enables the dialog after the job finished and displays a warning if
     * something didn't work.
     */
    void slotCopyingFinished(KJob* job);

    void slotFinished();

private:

    /**
     * Handle Close event from dialog title bar.
     */
    void closeEvent(QCloseEvent* e) override;

    /**
     * Refresh status (enabled / disabled) of the upload button according to
     * contents of the image list and the specified target.
     */
    void updateUploadButton();

    /**
     * Restores settings.
     */
    void restoreSettings();

    /**
     * Saves settings.
     */
    void saveSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericFileTransferPlugin
