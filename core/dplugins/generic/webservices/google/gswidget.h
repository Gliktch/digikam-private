/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2013-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "wssettingswidget.h"
#include "gsitem.h"
#include "dinfointerface.h"

using namespace Digikam;

class QButtonGroup;

namespace DigikamGenericGoogleServicesPlugin
{

enum GPhotoTagsBehaviour
{
    GPTagLeaf = 0,
    GPTagSplit,
    GPTagCombined
};

class GSWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit GSWidget(QWidget* const parent,
                      DInfoInterface* const iface,
                      const GoogleService& service,
                      const QString& serviceName);
    ~GSWidget() override = default;

    void updateLabels(const QString& name = QString(),
                      const QString& url = QString()) override;

private:

    GoogleService m_service;
    QButtonGroup* m_tagsBGrp = nullptr;

    friend class GSWindow;
};

} // namespace DigikamGenericGoogleServicesPlugin
