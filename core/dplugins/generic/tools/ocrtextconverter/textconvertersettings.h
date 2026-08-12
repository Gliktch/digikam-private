/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : OCR settings widgets
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QString>
#include <QStringList>

// Local includes

#include "ocroptions.h"

namespace DigikamGenericTextConverterPlugin
{

class TextConverterSettings : public QWidget
{
    Q_OBJECT

public:

    explicit TextConverterSettings(QWidget* const parent = nullptr);
    ~TextConverterSettings() override;

    OcrOptions ocrOptions() const;
    void setOcrOptions(const OcrOptions& opt);

    void readSettings();
    void saveSettings();

    void setDefaultSettings();

    void populateLanguagesMode(const QStringList& langs);

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotLocalizeChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericTextConverterPlugin
