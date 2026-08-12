/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : a tool to export items to a local storage
 *
 * SPDX-FileCopyrightText: 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2025 by Maik Qualmann <metzpinguin at gmail dot com>
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

using namespace Digikam;

namespace DigikamGenericFileCopyPlugin
{

class FCExportWidget;

/**
 * Main window of the FileCopyExport tool.
 */

class FCExportWindow: public WSToolDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent the parent QWidget
     */
    explicit FCExportWindow(DInfoInterface* const iface, QWidget* const parent);

    /**
     * Destructor.
     */
    ~FCExportWindow() override;

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

private Q_SLOTS:

    /**
     * Starts copy the selected images.
     */
    void slotCopy();

    /**
     * Processes changes on the image list.
     */
    void slotImageListChanged();

    /**
     * Processes changes in the target url.
     */
    void slotTargetUrlChanged(const QUrl& target);

    /**
     * Removes the copied image from the image list.
     */
    void slotCopyingDone(const QUrl& from, const QUrl& to);

    /**
     * Re-enables the dialog after the job finished and displays a warning if
     * something didn't work.
     */
    void slotCopyingFinished();

    /**
     * Re-enables the dialog after cancel clicked.
     */
    void slotCancelCopy();

    void slotFinished();

protected:

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

} // namespace DigikamGenericFileCopyPlugin
