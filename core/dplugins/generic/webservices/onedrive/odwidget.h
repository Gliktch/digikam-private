/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "wssettingswidget.h"
#include "oditem.h"
#include "dinfointerface.h"

using namespace Digikam;

class QButtonGroup;

namespace DigikamGenericOneDrivePlugin
{

class ODWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit ODWidget(QWidget* const parent,
                      DInfoInterface* const iface,
                      const QString& toolName);
    ~ODWidget() override = default;

    void updateLabels(const QString& name = QString(),
                      const QString& url = QString()) override;

private:

    friend class ODWindow;
};

} // namespace DigikamGenericOneDrivePlugin
