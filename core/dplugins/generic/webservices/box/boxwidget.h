/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Box web service
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
#include "boxitem.h"
#include "dinfointerface.h"

class QButtonGroup;

using namespace Digikam;

namespace DigikamGenericBoxPlugin
{

class BOXWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit BOXWidget(QWidget* const parent,
                       DInfoInterface* const iface,
                       const QString& toolName);
    ~BOXWidget() override = default;

    void updateLabels(const QString& name = QString(),
                      const QString& url = QString()) override;

private:

    friend class BOXWindow;
};

} // namespace DigikamGenericBoxPlugin
