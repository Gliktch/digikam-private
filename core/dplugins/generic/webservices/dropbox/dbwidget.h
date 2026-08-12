/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "wssettingswidget.h"
#include "dinfointerface.h"

class QLabel;
class QSpinBox;
class QCheckBox;
class QButtonGroup;
class QComboBox;
class QPushButton;

using namespace Digikam;

namespace DigikamGenericDropBoxPlugin
{

class DBWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit DBWidget(QWidget* const parent,
                      DInfoInterface* const iface,
                      const QString& toolName);
    ~DBWidget() override = default;

    void updateLabels(const QString& name = QString(), const QString& url = QString()) override;

private:

    friend class DBWindow;
};

} // namespace DigikamGenericDropBoxPlugin
