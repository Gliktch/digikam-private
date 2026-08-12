/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>

// Local includes

#include "wscomboboxintermediate.h"
#include "ditemslist.h"
#include "wssettingswidget.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericINatPlugin
{

class INatWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit INatWidget(QWidget* const parent,
                        DInfoInterface* const iface,
                        const QString& serviceName);
    ~INatWidget()                                       override;

    void updateLabels(const QString& name,
                      const QString& url = QString())   override;

private:

    class Private;
    Private* const d = nullptr;

    friend class INatWindow;
};

} // namespace DigikamGenericINatPlugin
