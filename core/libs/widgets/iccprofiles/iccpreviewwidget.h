/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-12
 * Description : a widget to display ICC profiles descriptions
 *               in file dialog preview.
 *
 * SPDX-FileCopyrightText: 2006-2007 by Francisco J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

// Local includes

#include "digikam_export.h"

class QUrl;

namespace Digikam
{

class ICCProfileWidget;

class DIGIKAM_EXPORT ICCPreviewWidget : public QScrollArea
{
    Q_OBJECT

public:

    explicit ICCPreviewWidget(QWidget* const parent = nullptr);
    ~ICCPreviewWidget() override = default;

public Q_SLOTS:

    void slotShowPreview(const QUrl& url);
    void slotClearPreview();

private:

    ICCProfileWidget* m_iccProfileWidget = nullptr;
};

} // namespace Digikam
