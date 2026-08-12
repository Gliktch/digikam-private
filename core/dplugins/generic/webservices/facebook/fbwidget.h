/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

//local includes

#include "wssettingswidget.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericFaceBookPlugin
{

class FbWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit FbWidget(QWidget* const parent,
                      DInfoInterface* const iface,
                      const QString& toolName);
    ~FbWidget();

    void updateLabels(const QString& name = QString(),
                      const QString& url = QString()) override;

Q_SIGNALS:

    void reloadAlbums(long long userID);

private Q_SLOTS:

    void slotReloadAlbumsRequest();

private:

    friend class FbWindow;
};

} // namespace DigikamGenericFaceBookPlugin
