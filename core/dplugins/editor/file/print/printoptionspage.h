/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : image editor printing interface.
 *
 * SPDX-FileCopyrightText: 2009      by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "iccprofile.h"

using namespace Digikam;

namespace DigikamEditorPrintToolPlugin
{

class PrintOptionsPage : public QWidget
{
    Q_OBJECT

public:

    enum ScaleMode
    {
        NoScale,
        ScaleToPage,
        ScaleToCustomSize
    };

    /**
     * Order should match the content of the unit combbox in the ui file
     */
    enum Unit
    {
        Millimeters,
        Centimeters,
        Inches
    };

public:

    explicit PrintOptionsPage(QWidget* const parent, const QSize& imageSize);
    ~PrintOptionsPage()                        override;

    void loadConfig();
    void saveConfig();

    bool          enlargeSmallerImages() const;
    bool          colorManaged()         const;
    bool          autoRotation()         const;
    Qt::Alignment alignment()            const;
    IccProfile    outputProfile()        const;

    ScaleMode     scaleMode()            const;
    Unit          scaleUnit()            const;
    double        scaleWidth()           const;
    double        scaleHeight()          const;

private Q_SLOTS:

    void adjustWidthToRatio();
    void adjustHeightToRatio();
    void slotAlertSettings(bool);
    void slotSetupDlg();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorPrintToolPlugin
