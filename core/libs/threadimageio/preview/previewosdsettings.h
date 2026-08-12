/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : Preview OSD settings container.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QColor>
#include <QString>
#include <QStringList>
#include <QPainter>
#include <QFont>
#include <QFontDatabase>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * This class contain host all the preview OSD settings.
 */
class DIGIKAM_EXPORT PreviewOsdSettings
{

public:

    PreviewOsdSettings()  = default;
    ~PreviewOsdSettings() = default;

    void readFromConfig(const QString& configGroupName);
    void writeToConfig(const QString& configGroupName);

public:

    /**
     * Helper methods to render text on the OSD.
     */
    static void printInfoText(QPainter& p, int& offset, const QString& str, const QColor& pcol = Qt::white);
    static void printComments(QPainter& p, int& offset, const QString& comments);
    static void printTags(QPainter& p, int& offset, QStringList& tags);

public:

    /**
     * Print picture file name while slide
     */
    bool                         printName                  = true;

    /**
     * Print picture creation date while slide
     */
    bool                         printDate                  = false;

    /**
     * Print camera Aperture and Focal while slide
     */
    bool                         printApertureFocal         = false;

    /**
     * Print camera Make and Model while slide
     */
    bool                         printMakeModel             = false;

    /**
     * Print camera Lens model while slide
     */
    bool                         printLensModel             = false;

    /**
     * Print camera Exposure and Sensitivity while slide
     */
    bool                         printExpoSensitivity       = false;

    /**
     * Print picture comment while slide
     */
    bool                         printCaption               = false;

    /**
     * Print image title while slide
     */
    bool                         printTitle                 = false;

    /**
     * Print image captions if no title available while slide
     */
    bool                         printCapIfNoTitle          = false;

    /**
     * Print tag names while slide
     */
    bool                         printKeywords              = false;

    /**
     * Font for the display of caption text
     */
    QFont                        captionFont                = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

private:

    const QString configPreviewOsdPrintApertureFocalEntry    = QLatin1String("PreviewOsdPrintApertureFocal");
    const QString configPreviewOsdPrintCommentEntry          = QLatin1String("PreviewOsdPrintComment");
    const QString configPreviewOsdPrintTitleEntry            = QLatin1String("PreviewOsdPrintTitle");
    const QString configPreviewOsdPrintCapIfNoTitleEntry     = QLatin1String("PreviewOsdPrintCapIfNoTitle");
    const QString configPreviewOsdPrintDateEntry             = QLatin1String("PreviewOsdPrintDate");
    const QString configPreviewOsdPrintExpoSensitivityEntry  = QLatin1String("PreviewOsdPrintExpoSensitivity");
    const QString configPreviewOsdPrintMakeModelEntry        = QLatin1String("PreviewOsdPrintMakeModel");
    const QString configPreviewOsdPrintLensModelEntry        = QLatin1String("PreviewOsdPrintLensModel");
    const QString configPreviewOsdPrintNameEntry             = QLatin1String("PreviewOsdPrintName");
    const QString configPreviewOsdPrintTagsEntry             = QLatin1String("PreviewOsdPrintTags");
    const QString configPreviewOsdCaptionFontEntry           = QLatin1String("PreviewOsdCaptionFont");
};

} // namespace Digikam
