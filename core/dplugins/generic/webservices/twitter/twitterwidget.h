/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter (x.com) social network
 *
 * SPDX-FileCopyrightText: 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
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

namespace DigikamGenericTwitterPlugin
{

class TwWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit TwWidget(QWidget* const parent,
                      DInfoInterface* const iface,
                      const QString& toolName);
    ~TwWidget() override = default;

    void updateLabels(const QString& name = QString(),
                      const QString& url = QString()) override;

Q_SIGNALS:

    void reloadAlbums(long long userID);

private Q_SLOTS:

/*
    void slotReloadAlbumsRequest();
*/

private:

    friend class TwWindow;
};

} // namespace DigikamGenericTwitterPlugin
