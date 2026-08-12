/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : configuration view for external BQM plugin
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "dpluginconfview.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT DPluginConfViewBqm : public DPluginConfView
{
    Q_OBJECT

public:

    explicit DPluginConfViewBqm(QWidget* const parent = nullptr);
    ~DPluginConfViewBqm() override = default;

    void loadPlugins()    override;
};

} // namespace Digikam
