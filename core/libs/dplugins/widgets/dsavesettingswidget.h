/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-13
 * Description : a widget to provide options to save image.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "filesaveconflictbox.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT DSaveSettingsWidget : public QWidget
{
    Q_OBJECT

public:

    enum OutputFormat
    {
        OUTPUT_PNG = 0,
        OUTPUT_TIFF,
        OUTPUT_JPEG,
        OUTPUT_PPM
    };

public:

    explicit DSaveSettingsWidget(QWidget* const parent);
    ~DSaveSettingsWidget() override;

public:

    void setCustomSettingsWidget(QWidget* const custom);

    OutputFormat fileFormat()                           const;
    void setFileFormat(OutputFormat f);

    FileSaveConflictBox::ConflictRule conflictRule()    const;
    void setConflictRule(FileSaveConflictBox::ConflictRule r);

    QString extension()                                 const;
    QString typeMime()                                  const;

    void resetToDefault();

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    static QString extensionForFormat(OutputFormat format);

public Q_SLOTS:

    void slotPopulateImageFormat(bool sixteenBits);

Q_SIGNALS:

    void signalSaveFormatChanged();
    void signalConflictButtonChanged(int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
