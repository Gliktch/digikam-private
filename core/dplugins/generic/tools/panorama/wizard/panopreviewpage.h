/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dwizardpage.h"
#include "panoactions.h"
#include <QMutexLocker>
#include <QMutex>

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class PanoManager;

class PanoPreviewPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit PanoPreviewPage(PanoManager* const mngr, QWizard* const dlg);
    ~PanoPreviewPage()      override;

private:

    void computePreview();
    void startStitching();

    void preInitializePage();
    void initializePage()   override;
    bool validatePage()     override;
    void cleanupPage()      override;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    void cleanupPage(QMutexLocker<QMutex>& lock);

#else

    void cleanupPage(QMutexLocker& lock);

#endif

Q_SIGNALS:

    void signalPreviewFinished();
    void signalStitchingFinished();

private Q_SLOTS:

    void slotCancel();
    void slotStartStitching();
    void slotPanoAction(const DigikamGenericPanoramaPlugin::PanoActionData&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPanoramaPlugin
