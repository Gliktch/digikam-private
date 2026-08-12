/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-05
 * Description : Placeholder widget for when backends are activated
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFrame>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT PlaceholderWidget : public QFrame
{
    Q_OBJECT

public:

    explicit PlaceholderWidget(QWidget* const parent = nullptr);
    ~PlaceholderWidget() override;

    void setMessage(const QString& message);

private:

    Q_DISABLE_COPY(PlaceholderWidget)

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam
