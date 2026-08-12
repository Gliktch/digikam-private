/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : a widget to setup the preview OSD.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QGroupBox>

// Local includes

#include "digikam_export.h"
#include "previewosdsettings.h"

namespace Digikam
{

class DIGIKAM_EXPORT PreviewOsdWidget : public QGroupBox
{
    Q_OBJECT

public:

    explicit PreviewOsdWidget(PreviewOsdSettings* const settings,
                              QWidget* const parent = nullptr);
    ~PreviewOsdWidget() override;

    void readSettings();
    void writeSettings();

public Q_SLOTS:

    void slotSetUnchecked(int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
