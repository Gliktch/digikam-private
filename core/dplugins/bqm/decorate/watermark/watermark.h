/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-28
 * Description : batch tool to add visible watermark.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Mikkel Baekhoej Christensen <mbc at baekhoej dot dk>
 * SPDX-FileCopyrightText: 2017      by Ahmed Fathi <ahmed dot fathi dot abdelmageed at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFont>

// Local includes

#include "batchtool.h"

using namespace Digikam;

namespace DigikamBqmWatermarkPlugin
{

class WaterMark : public BatchTool
{
    Q_OBJECT

public:

    explicit WaterMark(QObject* const parent = nullptr);
    ~WaterMark()                                            override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private Q_SLOTS:

    void slotSettingsChanged()                              override;
    void slotAssignSettings2Widget()                        override;

private:

    bool toolOperations()                                   override;
    int  queryFontSize(const QString& text,
                       const QFont& font,
                       int length)                    const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmWatermarkPlugin
