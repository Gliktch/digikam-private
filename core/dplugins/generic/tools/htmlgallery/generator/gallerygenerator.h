/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "dprogresswdg.h"
#include "dhistoryview.h"

using namespace Digikam;

namespace DigikamGenericHtmlGalleryPlugin
{

class GalleryInfo;
class GalleryElementFunctor;

/**
 * This class is responsible for generating the HTML and scaling the images
 * according to the settings specified by the user.
 */
class GalleryGenerator : public QObject
{
    Q_OBJECT

public:

    explicit GalleryGenerator(GalleryInfo* const);
    ~GalleryGenerator() override;

    void setProgressWidgets(DHistoryView* const, DProgressWdg* const);

    bool run();
    bool warnings() const;

    /**
     * Produce a web-friendly file name
     */
    static QString webifyFileName(const QString&);

Q_SIGNALS:

    /**
     * This signal is emitted from GalleryElementFunctor. It uses a
     * QueuedConnection to switch between the GalleryElementFunctor thread and
     * the gui thread.
     */
    void logWarningRequested(const QString&);

public Q_SLOTS:

    void slotCancel();

private Q_SLOTS:

    void logWarning(const QString&);

private:

    class Private;
    Private* const d = nullptr;

    friend class Private;
    friend class GalleryElementFunctor;
};

} // namespace DigikamGenericHtmlGalleryPlugin
