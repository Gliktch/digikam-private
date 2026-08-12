/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-13
 * Description : Greycstoration settings widgets
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QFile>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "greycstorationfilter.h"

class QTabWidget;

namespace Digikam
{

class DIGIKAM_EXPORT GreycstorationSettings : public QObject
{
    Q_OBJECT

public:

    explicit GreycstorationSettings(QTabWidget* const parent);
    ~GreycstorationSettings() override;

    void setSettings(const GreycstorationContainer& settings);
    GreycstorationContainer settings() const;

    void setDefaultSettings(const GreycstorationContainer& settings);

    bool loadSettings(QFile& file, const QString& header);
    void saveSettings(QFile& file, const QString& header);

    void setEnabled(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
