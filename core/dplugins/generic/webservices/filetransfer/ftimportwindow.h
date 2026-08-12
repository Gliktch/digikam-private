/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 04.10.2009
 * Description : a tool to import items from a KIO accessible location
 *
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
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

/**
 * Main dialog used for the import tool.
 */
class FTImportWindow : public WSToolDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent the parent widget
     */
    explicit FTImportWindow(DInfoInterface* const iface, QWidget* const parent);

    /**
     * Destructor.
     */
    ~FTImportWindow() override;

private Q_SLOTS:

    /**
     * Starts importing the selected images.
     */
    void slotImport();

    /**
     * Reacts on changes in the image list or the target to update button
     * activity etc.
     */
    void slotSourceAndTargetUpdated();

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

private:

    /**
     * Handle Close event from dialog title bar.
     */
    void closeEvent(QCloseEvent* e) override;

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
